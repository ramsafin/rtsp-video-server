#ifndef LIRS_RTSP_VIDEO_SERVER_CONFIG_FILE_TYPE_HPP
#define LIRS_RTSP_VIDEO_SERVER_CONFIG_FILE_TYPE_HPP

#include <cstdint>

namespace lirs {

    namespace config {

        enum class ConfigFileType : uint8_t {
            YAML = 0,
            JSON,
            XML,
            INI,
            UNDEFINED
        };
    }
}

#endif //LIRS_RTSP_VIDEO_SERVER_CONFIG_FILE_TYPE_HPP
