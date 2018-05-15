#include "LiveCamFramedSource.hpp"

namespace LIRS {

    LiveCamFramedSource *LiveCamFramedSource::createNew(UsageEnvironment &env, Transcoder &transcoder) {
        return new LiveCamFramedSource(env, transcoder);
    }

    LiveCamFramedSource::~LiveCamFramedSource() {

        transcoder.stop();

        // delete trigger
        envir().taskScheduler().deleteEventTrigger(eventTriggerId);
        eventTriggerId = 0;

        // cleanup encoded data buffer
        encodedDataBuffer.clear();
        encodedDataBuffer.shrink_to_fit();

        LOG(INFO) << transcoder.getCameraName() << ":max NALU size: " << max_nalu_size_bytes;
    }

    LiveCamFramedSource::LiveCamFramedSource(UsageEnvironment &env, Transcoder &transcoder) :
            FramedSource(env), transcoder(transcoder), eventTriggerId(0), max_nalu_size_bytes(0) {

        // create trigger invoking method which will deliver frame
        eventTriggerId = envir().taskScheduler().createEventTrigger(LiveCamFramedSource::deliverFrame0);

        encodedDataBuffer.reserve(5); // reserve enough space for handling incoming encoded data

        // set transcoder's callback indicating new encoded data availability
        transcoder.setOnEncodedDataCallback(std::bind(&LiveCamFramedSource::onEncodedData, this,
                                                      std::placeholders::_1));

        // start video data encoding/decoding in a new thread

        LOG(DEBUG) << "Starting to capture and encode video from the camera: "
                   << transcoder.getCameraUrl();

        std::thread([&transcoder]() {
            transcoder.run();
        }).detach();
    }

    void LiveCamFramedSource::onEncodedData(std::vector<uint8_t> &&newData) {

        if (!isCurrentlyAwaitingData()) {
            return;
        }

        encodedDataMutex.lock();

        // store encoded data to be processed later
        encodedDataBuffer.emplace_back(std::move(newData));

        encodedDataMutex.unlock();

        // publish an event to be handled by the event loop
        envir().taskScheduler().triggerEvent(eventTriggerId, this);
    }

    void LiveCamFramedSource::deliverFrame0(void *clientData) {
        ((LiveCamFramedSource *) clientData)->deliverData();
    }

    void LiveCamFramedSource::doStopGettingFrames() {

        LOG(DEBUG) << "Stop getting frames from the camera: " << transcoder.getCameraName();

        FramedSource::doStopGettingFrames();
    }

    void LiveCamFramedSource::deliverData() {

        if (!isCurrentlyAwaitingData()) {
            return;
        }

        encodedDataMutex.lock();

        encodedData = std::move(encodedDataBuffer.back());

        encodedDataBuffer.pop_back();

        encodedDataMutex.unlock();

        if (encodedData.size() > max_nalu_size_bytes) {
            max_nalu_size_bytes = encodedData.size();
        }

        if (encodedData.size() > fMaxSize) { // truncate data

            LOG(WARN) << "Exceeded max size, truncated: " << fNumTruncatedBytes << ", size: " << encodedData.size();

            fFrameSize = fMaxSize;

            fNumTruncatedBytes = static_cast<unsigned int>(encodedData.size() - fMaxSize);

        } else {
            fFrameSize = static_cast<unsigned int>(encodedData.size());
        }

        // can be changed to the actual frame's captured time
        gettimeofday(&fPresentationTime, nullptr);

        // DO NOT CHANGE ADDRESS, ONLY COPY (see Live555 docs)
        memcpy(fTo, encodedData.data(), fFrameSize);

        // should be invoked after successfully getting data
        FramedSource::afterGetting(this);
    }

    void LiveCamFramedSource::doGetNextFrame() {

        if (!encodedDataBuffer.empty()) {
            deliverData();
        } else {
            fFrameSize = 0;
            return;
        }
    }
}
