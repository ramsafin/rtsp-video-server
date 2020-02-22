#ifndef LIRS_RTSP_VIDEO_SERVER_CONFIG_LOADER_HPP
#define LIRS_RTSP_VIDEO_SERVER_CONFIG_LOADER_HPP

#include "config/params/Configuration.hpp"

namespace lirs {

    namespace config {

        // Configuration Loader Service.

        class ConfigLoader {

        public:

            virtual ~ConfigLoader();

            explicit ConfigLoader(std::string configResource);

            virtual bool load(params::Configuration &configuration) = 0;

        protected:

            std::string m_configResource;
        };
    }
}

#endif //LIRS_RTSP_VIDEO_SERVER_CONFIG_LOADER_HPP
