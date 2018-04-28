#ifndef LIVE_USB_CAM_FRAMED_SOURCE_HPP
#define LIVE_USB_CAM_FRAMED_SOURCE_HPP

#include <FramedSource.hh>
#include <UsageEnvironment.hh>

#include <mutex>
#include <thread>

#include "Transcoder.hpp"

namespace LIRS {

    /**
     * Implementation of the framed source for live camera.
     */
    class LiveCamFramedSource : public FramedSource {
    public:

        static LiveCamFramedSource *createNew(UsageEnvironment &env, Transcoder &transcoder);

    protected:

        /**
         * Constructs a new instance of framed source using video device as a source.
         *
         * @param env - environment (see Live555 docs).
         * @param transcoder - provides with an encoded data.
         */
        LiveCamFramedSource(UsageEnvironment &env, Transcoder &transcoder);

        ~LiveCamFramedSource() override;

        void doGetNextFrame() override;

        void doStopGettingFrames() override;

    private:

        /**
         * Provides encoded data from the video device it represents.
         */
        Transcoder &transcoder;

        /**
         * Indicating an event invoking deliver frame method.
         */
        EventTriggerId eventTriggerId;

        /**
         * Mutex to access encoded data.
         * @see encodedData
         * @see deliverData()
         */
        std::mutex encodedDataMutex;

        /**
         * Encoded data buffer.
         */
        std::vector<std::vector<uint8_t>> encodedDataBuffer;

        /**
         * Encoded data.
         */
        std::vector<uint8_t> encodedData;

        size_t max_nalu_size_bytes;

        /**
         * Function to be called when the video source has a new available encoded data.
         */
        void onEncodedData(std::vector<uint8_t> &&data);

        /**
         * Delivers encoded data.
         */
        void deliverData();

        /**
         * Trigger function.
         */
        static void deliverFrame0(void *);
    };

}

#endif //LIVE_USB_CAM_FRAMED_SOURCE_HPP
