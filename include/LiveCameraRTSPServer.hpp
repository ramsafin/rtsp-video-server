#ifndef LIVE_VIDEO_STREAM_LIVE_RTSP_SERVER_HPP
#define LIVE_VIDEO_STREAM_LIVE_RTSP_SERVER_HPP

#include <UsageEnvironment.hh>
#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>
#include <liveMedia.hh>

#include "LiveCamFramedSource.hpp"
#include "CameraUnicastServerMediaSubsession.hpp"
#include "config/params/Configuration.hpp"

namespace LIRS {

    class LiveCameraRTSPServer {

    public:

        /** Constants **/

        explicit LiveCameraRTSPServer(lirs::config::params::ServerParameters const &config);

        ~LiveCameraRTSPServer();

        /**
         * Changes the watch variable in order to stop the event loop.
         */
        void stopServer();


        void addTranscoder(std::shared_ptr<Transcoder> transcoder);

        /*
         * Creates a new RTSP server adding subsessions to each video source.
         */
        void run();

    private:

        /*
         * Watch variable to control the event loop.
         */
        char watcher;

        TaskScheduler *scheduler;

        UsageEnvironment *env;

        RTSPServer *server;

        lirs::config::params::ServerParameters config;

        /**
         * Video sources (transcoders).
         */
        std::vector<std::shared_ptr<Transcoder>> transcoders;

        /**
         * Framed sources.
         */
        std::vector<FramedSource *> allocatedVideoSources;

        /**
         * Announce new create media session.
         *
         * @param sms - created server media session.
         * @param deviceName - the name of the video source device.
         */
        void announceStream(ServerMediaSession *sms, const std::string &deviceName);

        /**
         * Adds new server media session using transcoder as a source to the server.
         *
         * @param transcoder - video source.
         * @param streamName - the name of the stream (part of the URL), e.g. rtsp://.../camera/1.
         * @param streamDesc -description of the stream.
         */
        void addMediaSession(std::shared_ptr<Transcoder> transcoder, const std::string &streamName, const std::string &streamDesc);

    };
}

#endif //LIVE_VIDEO_STREAM_LIVE_RTSP_SERVER_HPP
