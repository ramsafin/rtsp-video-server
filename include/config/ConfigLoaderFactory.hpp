#ifndef LIRS_RTSP_VIDEO_SERVER_CONFIG_LOADER_FACTORY_HPP
#define LIRS_RTSP_VIDEO_SERVER_CONFIG_LOADER_FACTORY_HPP

#include "ConfigLoader.hpp"

namespace lirs {

    namespace config {

        class ConfigLoaderFactory {

        public:

            virtual std::shared_ptr<ConfigLoader> createConfigLoader(std::string const &configResource) = 0;
        };
    }
}

#endif //LIRS_RTSP_VIDEO_SERVER_CONFIG_LOADER_FACTORY_HPP
