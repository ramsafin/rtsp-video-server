#include "config/ConfigLoader.hpp"

namespace lirs {

    namespace config {

        ConfigLoader::~ConfigLoader() {
            m_configResource.clear();
        }

        ConfigLoader::ConfigLoader(std::string configResource)
                : m_configResource(std::move(configResource)) {}
    }

}
