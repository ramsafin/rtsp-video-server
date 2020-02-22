#include "config/ExtensionConfigLoaderFactory.hpp"
#include "config/YamlConfigLoader.hpp"
#include "utils/Logger.hpp"

namespace lirs {

    namespace config {

        // todo: add other formats support, e.g. JSON, XML, HTTP

        std::shared_ptr<ConfigLoader>
        ExtensionConfigLoaderFactory::createConfigLoader(std::string const &configResource) {

            switch (determineFileType(configResource)) {
                case ConfigFileType::YAML:
                    return std::make_shared<YamlConfigLoader>(configResource);
                default:
                    throw std::invalid_argument("Given file format is not supported.");
            }
        }


        ConfigFileType ExtensionConfigLoaderFactory::determineFileType(std::string const &configResource) {

            auto extension = configResource.substr(configResource.rfind('.') + 1);

            if (extension == "yaml" || extension == "yml") return ConfigFileType::YAML;

            return ConfigFileType::UNDEFINED;
        }
    }
}