#include <CameraUnicastServerMediaSubsession.hpp>

namespace LIRS {

    CameraUnicastServerMediaSubsession *
    CameraUnicastServerMediaSubsession::createNew(UsageEnvironment &env, StreamReplicator *replicator,
                                                  size_t estBitrate, size_t udpDatagramSize) {
        return new CameraUnicastServerMediaSubsession(env, replicator, estBitrate, udpDatagramSize);
    }

    CameraUnicastServerMediaSubsession::CameraUnicastServerMediaSubsession(UsageEnvironment &env,
                                                                           StreamReplicator *replicator,
                                                                           size_t estBitrate,
                                                                           size_t udpDatagramSize)
            : OnDemandServerMediaSubsession(env, False), replicator(replicator), estBitrate(estBitrate),
              udpDatagramSize(udpDatagramSize) {

        LOG(DEBUG) << "Unicast media subsession with UDP datagram size of " << udpDatagramSize
                   << " and estimated bitrate of " << estBitrate << " (kbps) is created";
    }

    FramedSource *
    CameraUnicastServerMediaSubsession::createNewStreamSource(unsigned clientSessionId, unsigned &estBitrate) {

        estBitrate = static_cast<unsigned int>(this->estBitrate);

        auto source = replicator->createStreamReplica();

        // only discrete frames are being sent (w/o start code bytes)
        return H265VideoStreamDiscreteFramer::createNew(envir(), source);
    }

    RTPSink *
    CameraUnicastServerMediaSubsession::createNewRTPSink(Groupsock *rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic,
                                                         FramedSource *inputSource) {

        auto sink = H265VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);

        // set the UDP datagram size
        sink->setPacketSizes(static_cast<unsigned int>(udpDatagramSize), static_cast<unsigned int>(udpDatagramSize));

        return sink;
    }

}


