#include "LiveCameraRTSPServer.hpp"

namespace LIRS {

    LiveCameraRTSPServer::LiveCameraRTSPServer(unsigned int port, int httpPort) :
            rtspPort(port), httpTunnelingPort(httpPort), watcher(0),
            scheduler(nullptr), env(nullptr), server(nullptr) {

        OutPacketBuffer::maxSize = OUT_PACKET_BUFFER_MAX_SIZE_BYTES;

        LOG(DEBUG) << "Setting OutPacketBuffer max size to " << OutPacketBuffer::maxSize << " (bytes)";

        // create scheduler and environment
        scheduler = BasicTaskScheduler::createNew();
        env = BasicUsageEnvironment::createNew(*scheduler);
    }

    LiveCameraRTSPServer::~LiveCameraRTSPServer() {

        Medium::close(server); // deletes all server media sessions

        // delete all framed sources
        for (auto &src : allocatedVideoSources) {
            if (src) Medium::close(src);
        }

        env->reclaim();

        delete scheduler;

        transcoders.clear();
        allocatedVideoSources.clear();
        watcher = 0;

        LOG(DEBUG) << "RTSP server has been destructed!";
    }

    void LiveCameraRTSPServer::stopServer() {

        LOG(DEBUG) << "Stop server is invoked!";

        watcher = 's';
    }

    void LiveCameraRTSPServer::addTranscoder(Transcoder &transcoder) {
        transcoders.push_back(&transcoder);
    }

    void LiveCameraRTSPServer::run() {

        if (server) {

            LOG(WARN) << "Server is already running!";

            return; // already running
        }

        // create server listening on the specified RTSP port
        server = RTSPServer::createNew(*env, rtspPort);

        if (!server) {
            LOG(ERROR) << "Failed to create RTSP server: " << env->getResultMsg();
            exit(1);
        }

        LOG(DEBUG) << "Server has been created on port " << rtspPort;

        if (httpTunnelingPort != -1) { // set up HTTP tunneling (see Live555 docs)
            auto res = server->setUpTunnelingOverHTTP(httpTunnelingPort);
            if (res) {
                LOG(INFO) << "Enabled HTTP tunneling over: " << httpTunnelingPort;
            }
        }

        LOG(DEBUG) << "Creating media session for each transcoder";

        // create media session for each video source (transcoder)
        for (auto &transcoder : transcoders) {
            addMediaSession(transcoder, transcoder->getCameraName(), "stream description");
        }

        env->taskScheduler().doEventLoop(&watcher); // do not return
    }

    void LiveCameraRTSPServer::announceStream(ServerMediaSession *sms, const std::string &deviceName) {
        auto url = server->rtspURL(sms);
        LOG(INFO) << "Play the stream of the '" << deviceName.data() << "' camera using the following URL: " << url;
        delete[] url;
    }

    void LiveCameraRTSPServer::addMediaSession(Transcoder *transcoder, const std::string &streamName,
                                               const std::string &streamDesc) {

        LOG(DEBUG) << "Adding media session for camera: " << transcoder->getCameraName();

        // create framed source for camera
        auto framedSource = LiveCamFramedSource::createNew(*env, *transcoder);

        // store it in order to cleanup after
        allocatedVideoSources.push_back(framedSource);

        // create stream replicator for the framed source
        auto replicator = StreamReplicator::createNew(*env, framedSource, False);

        // create media session with the specified topic and description
        auto sms = ServerMediaSession::createNew(*env, streamName.c_str(), "stream information", streamDesc.c_str(),
                                                 False, "a=fmtp:96\n");

        // add unicast subsession
        sms->addSubsession(CameraUnicastServerMediaSubsession::createNew(*env, replicator, estimatedBitrate, udpDatagramSize));

        server->addServerMediaSession(sms);

        // announce stream
        announceStream(sms, transcoder->getCameraName());
    }

}
