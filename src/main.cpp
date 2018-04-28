#include "Config.hpp"
#include "LiveCameraRTSPServer.hpp"
#include <csignal>

namespace {

    std::function<void(int)> shutdown_handler;

    void signal_handler(int signal) {
        shutdown_handler(signal);
    }
}

int main(int argc, char **argv) {

    // set SIG* handlers
    std::signal(SIGTERM, signal_handler);
    std::signal(SIGKILL, signal_handler);
    std::signal(SIGINT, signal_handler);

    // init loggers
    initLogger(log4cpp::Priority::INFO);
    av_log_set_level(AV_LOG_INFO);

    // load configuration
    LIRS::Configuration configuration;

    configuration.loadConfig(argc, argv);

    LIRS::Transcoder camera(configuration);

    LIRS::LiveCameraRTSPServer server;

    server.udpDatagramSize = static_cast<unsigned int>(configuration.get_udp_datagram_size());
    server.estimatedBitrate = static_cast<unsigned int>(configuration.get_bitrate());

    shutdown_handler = [&server](int signal) {
        LOG(INFO) << "Terminating server...";
        server.stopServer();
    };

    server.addTranscoder(camera);

    server.run();

    return 0;
}
