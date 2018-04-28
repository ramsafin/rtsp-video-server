#ifndef LIVE_VIDEO_STREAM_LIVE_RTSP_SERVER_HPP
#define LIVE_VIDEO_STREAM_LIVE_RTSP_SERVER_HPP

#include <UsageEnvironment.hh>
#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>
#include <liveMedia.hh>

#include "LiveCamFramedSource.hpp"
#include "CameraUnicastServerMediaSubsession.hpp"

namespace LIRS {

    class LiveCameraRTSPServer {

    public:

        unsigned int udpDatagramSize = Configuration::DEFAULT_UDP_DATAGRAM_SIZE;

        unsigned int estimatedBitrate = Configuration::DEFAULT_BITRATE_KBPS;

        /** Constants **/

        constexpr static unsigned int DEFAULT_RTSP_PORT_NUMBER = 8554;

        constexpr static unsigned int OUT_PACKET_BUFFER_MAX_SIZE_BYTES = 2 * 1000 * 1000;

        explicit LiveCameraRTSPServer(unsigned int port = DEFAULT_RTSP_PORT_NUMBER, int httpPort = -1);

        ~LiveCameraRTSPServer();

        /**
         * Changes the watch variable in order to stop the event loop.
         */
        void stopServer();

        /**
         * Adds transcoder as a source in order to create server media session with it.
         *
         * @param transcoder - a reference to the transcoder.
         */
        void addTranscoder(Transcoder &transcoder);

        /*
         * Creates a new RTSP server adding subsessions to each video source.
         */
        void run();

    private:

        /**
         * RTSP port number.
         */
        unsigned int rtspPort;

        /**
         * RTSP server's HTTP tunneling port number.
         */
        int httpTunnelingPort;

        /*
         * Watch variable to control the event loop.
         */
        char watcher;

        TaskScheduler *scheduler;

        UsageEnvironment *env;

        RTSPServer *server;

        /**
         * Video sources (transcoders).
         */
        std::vector<Transcoder *> transcoders;

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
        void addMediaSession(Transcoder *transcoder, const std::string &streamName, const std::string &streamDesc);

    };
}

#endif //LIVE_VIDEO_STREAM_LIVE_RTSP_SERVER_HPP
