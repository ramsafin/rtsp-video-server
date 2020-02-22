#ifndef LIVE_VIDEO_STREAM_CUSTOM_SERVER_MEDIA_SUBSESSION_HPP
#define LIVE_VIDEO_STREAM_CUSTOM_SERVER_MEDIA_SUBSESSION_HPP

#include <OnDemandServerMediaSubsession.hh>
#include <StreamReplicator.hh>
#include <H265VideoRTPSink.hh>
#include <H265VideoStreamDiscreteFramer.hh>

#include "utils/Logger.hpp"
#include "Config.hpp"

namespace LIRS {

    /**
     * Live555 unicast server media subsession class implementation.
     */
    class CameraUnicastServerMediaSubsession : public OnDemandServerMediaSubsession {

    public:

        static CameraUnicastServerMediaSubsession *
        createNew(UsageEnvironment &env, StreamReplicator *replicator, size_t estBitrate, size_t udpDatagramSize);

    protected:

        /**
         * Replicates the streaming source for each new client.
         */
        StreamReplicator *replicator;

        /**
         * Estimated bitrate of the video stream.
         */
        size_t estBitrate;

        /**
         * UDP datagram size in bytes.
         */
        size_t udpDatagramSize;


        CameraUnicastServerMediaSubsession(UsageEnvironment &env,
                                           StreamReplicator *replicator,
                                           size_t estBitrate,
                                           size_t udpDatagramSize);


        FramedSource *createNewStreamSource(unsigned clientSessionId, unsigned &estBitrate) override;


        RTPSink *createNewRTPSink(Groupsock *rtpGroupsock,
                                  unsigned char rtpPayloadTypeIfDynamic,
                                  FramedSource *inputSource) override;

    };
}

#endif //LIVE_VIDEO_STREAM_CUSTOM_SERVER_MEDIA_SUBSESSION_HPP
