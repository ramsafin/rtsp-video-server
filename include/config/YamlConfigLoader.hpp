#ifndef LIRS_RTSP_VIDEO_SERVER_YAML_CONFIG_LOADER_HPP
#define LIRS_RTSP_VIDEO_SERVER_YAML_CONFIG_LOADER_HPP

#include "ConfigLoader.hpp"

namespace lirs {

    namespace config {

        class YamlConfigLoader final : public ConfigLoader {

        public:

            explicit YamlConfigLoader(std::string const &configResource);

            bool load(params::Configuration &configuration) override;
        };
    }
}

#endif //LIRS_RTSP_VIDEO_SERVER_YAML_CONFIG_LOADER_HPP
