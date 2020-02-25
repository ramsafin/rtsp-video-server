#ifndef LIRS_RTSP_VIDEO_SERVER_EXTENSION_CONFIG_LOADER_FACTORY_HPP
#define LIRS_RTSP_VIDEO_SERVER_EXTENSION_CONFIG_LOADER_FACTORY_HPP

#include <memory>

#include "ConfigLoaderFactory.hpp"
#include "ConfigFileType.hpp"

namespace lirs {

    namespace config {

        class ExtensionConfigLoaderFactory : public ConfigLoaderFactory {

        public:

            std::shared_ptr<ConfigLoader> createConfigLoader(std::string const &configResource) override;

        private:

             static ConfigFileType determineFileType(std::string const &configResource);
        };
    }
}
#endif //LIRS_RTSP_VIDEO_SERVER_EXTENSION_CONFIG_LOADER_FACTORY_HPP
