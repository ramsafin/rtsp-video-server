#include "LiveCameraRTSPServer.hpp"
#include "config/ExtensionConfigLoaderFactory.hpp"
#include <csignal>

namespace {
    std::function<void(int)> shutdown_handler;

    void signal_handler(int signal) {
        shutdown_handler(signal);
    }
}

int main(int argc, char **argv) {

    assert(argc >= 2);

    std::signal(SIGTERM, signal_handler);
    std::signal(SIGKILL, signal_handler);
    std::signal(SIGINT, signal_handler);

    // init loggers
    initLogger(log4cpp::Priority::INFO);

    av_log_set_level(AV_LOG_INFO);

    using namespace lirs::config;

    params::Configuration configuration;

    ExtensionConfigLoaderFactory extensionConfigLoaderFactory;

    try {

        auto configLoader = extensionConfigLoaderFactory.createConfigLoader(argv[1]);

        bool status = configLoader->load(configuration);

        assert(status);

        LIRS::LiveCameraRTSPServer server(configuration.getServerParams());

        shutdown_handler = [&server](int signal){
            LOG(INFO) << "Terminating server: " << signal;
            server.stopServer();
        };

        for (auto &conf : configuration.getCameraParams()) {

            server.addTranscoder(std::make_shared<LIRS::Transcoder>(conf.second));
        }

        server.run();


    } catch (const std::invalid_argument& err) {

        LOG(ERROR) << err.what();
    }

    return 0;

}