#ifndef LIVE_VIDEO_STREAM_CONFIG_HPP
#define LIVE_VIDEO_STREAM_CONFIG_HPP

#include <string>
#include "Logger.hpp"
#include "Version.hpp"

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/option.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

namespace po = boost::program_options;

#define RARE_CAM "/dev/v4l/by-path/pci-0000:00:14.0-usb-0:3:1.0-video-index0"
#define LEFT_CAM "/dev/v4l/by-path/pci-0000:00:14.0-usb-0:4:1.0-video-index0"
#define RIGHT_CAM "/dev/v4l/by-path/pci-0000:00:1a.0-usb-0:1.3:1.0-video-index0"
#define ZOOM_CAM "/dev/v4l/by-path/pci-0000:00:14.0-usb-0:2:1.0-video-index0"

namespace LIRS {

    class Configuration {

    public:

        constexpr static unsigned int DEFAULT_STREAMING_FRAMERATE = 5;

        constexpr static unsigned int DEFAULT_FRAMERATE = 15;

        constexpr static unsigned int DEFAULT_FRAME_WIDTH = 744;

        constexpr static unsigned int DEFAULT_FRAME_HEIGHT = 480;

        constexpr static unsigned int DEFAULT_UDP_DATAGRAM_SIZE = 1500;

        constexpr static unsigned int DEFAULT_BITRATE_KBPS = 100;

        constexpr static char const *DEFAULT_PIX_FORMAT = "bayer_grbg8";

        // =========== Codec settings ================= //

        constexpr static unsigned int DEFAULT_VBV_BUFSIZE = 512;

        constexpr static unsigned int DEFAULT_INTRA_REFRESH_ENABLED = 0;

        constexpr static unsigned int DEFAULT_NUM_SLICES = 1;

        constexpr static char const *CODEC_PARAMS_PATTERN = "slices=%d:intra-refresh=%d:vbv-maxrate=%d:vbv-bufsize=%d";

        std::string get_codec_params_str() const {
            char formattedStr[512];
            sprintf(formattedStr, CODEC_PARAMS_PATTERN, (int) number_of_slices, intra_refresh_enabled,
                    (int) estimated_bitrate_kbps, (int) vbv_bufsize_bytes);

            LOG(INFO) << "Set codec params to: " << formattedStr;

            return std::string(formattedStr);
        }

        bool loadConfig(int cliArgc, char **cliArgs) {

            po::options_description cli_options;

            po::options_description generic("Generic options");
            po::options_description streaming("Streaming configuration");
            po::options_description codec("Codec configuration");

            generic.add_options()
                    ("version,v", "print version")
                    ("help", "produce help message");

            streaming.add_options()
                    ("source,s", po::value<std::string>(&streaming_source_url)->default_value(RARE_CAM),
                     "set video streaming camera url")
                    ("topic,t", po::value<std::string>(&streaming_topic_name)->default_value("rare"),
                     "set streaming topic name")
                    ("width,w", po::value<size_t>(&origin_frame_width)->default_value(DEFAULT_FRAME_WIDTH),
                     "set original frame width")
                    ("height,h", po::value<size_t>(&origin_frame_height)->default_value(DEFAULT_FRAME_HEIGHT),
                     "set original frame height")
                    ("fps,f", po::value<size_t>(&origin_framerate)->default_value(DEFAULT_FRAMERATE),
                     "set original framerate")
                    ("out-width", po::value<size_t>(&streaming_frame_width)->default_value(DEFAULT_FRAME_WIDTH),
                     "set streaming frame width")
                    ("out-height", po::value<size_t>(&streaming_frame_height)->default_value(DEFAULT_FRAME_HEIGHT),
                     "set streaming frame height")
                    ("pix-fmt", po::value<std::string>(&origin_pixel_format)->default_value(DEFAULT_PIX_FORMAT),
                     "set original pixel format")
                    ("out-fps",
                     po::value<size_t>(&streaming_framerate)->default_value(DEFAULT_STREAMING_FRAMERATE),
                     "set streaming framerate")
                    ("trial", po::value<size_t>(&trial_index)->default_value(1));

            codec.add_options()
                    ("codec-pix-fmt", po::value<std::string>(&codec_input_pixel_format)->default_value("yuv420p"),
                     "set codec pixel format")
                    ("num-slices", po::value<size_t>(&number_of_slices)->default_value(DEFAULT_NUM_SLICES),
                     "set number of slices")
                    ("vbv-bufsize", po::value<size_t>(&vbv_bufsize_bytes)->default_value(DEFAULT_VBV_BUFSIZE),
                     "set vbv buffer size in bytes")
                    ("intra-refresh",
                     po::value<bool>(&intra_refresh_enabled)->default_value(DEFAULT_INTRA_REFRESH_ENABLED),
                     "enable intra-refresh")
                    ("udp", po::value<size_t>(&udp_datagram_size_bytes)->default_value(DEFAULT_UDP_DATAGRAM_SIZE),
                     "set UDP datagram size in bytes")
                    ("bitrate", po::value<size_t>(&estimated_bitrate_kbps)->default_value(DEFAULT_BITRATE_KBPS),
                     "set estimated bitrate in kbps");


            cli_options.add(generic).add(streaming).add(codec);

            po::variables_map vm;

            po::store(po::parse_command_line(cliArgc, reinterpret_cast<const char *const *>(cliArgs),
                                             cli_options), vm, true);

            po::notify(vm);

            if (vm.count("help")) {
                std::cout << cli_options << std::endl;
                exit(0);
            }

            if (vm.count("version")) {
                std::cout << "Live Video Stream Server\nCurrent version: " << LIVE_VIDEO_STREAM_VERSION << std::endl;
                exit(0);
            }

            if (vm.count("source")) {

                if (streaming_source_url == "rare") {
                    streaming_source_url = RARE_CAM;
                } else if (streaming_source_url == "right") {
                    streaming_source_url = RIGHT_CAM;
                } else if (streaming_source_url == "left") {
                    streaming_source_url = LEFT_CAM;
                } else if (streaming_source_url == "center") {
                    streaming_source_url = ZOOM_CAM;
                }
            }

            LOG(DEBUG) << "Configuration has been loaded!";

            LOG(INFO) << "trial: " << trial_index << "\n\t"
                      << "source: " << streaming_source_url << "\n\t"
                      << "topic: " << streaming_topic_name << "\n\t"
                      << "origin_frame_width: " << origin_frame_width << "\n\t"
                      << "origin_frame_height: " << origin_frame_height << "\n\t"
                      << "out_frame_width: " << streaming_frame_width << "\n\t"
                      << "out_frame_height: " << streaming_frame_height << "\n\t"
                      << "origin_framerate: " << origin_framerate << "\n\t"
                      << "out_framerate: " << streaming_framerate << "\n\t"
                      << "slices: " << number_of_slices << "\n\t"
                      << "vbv_bufsize: " << vbv_bufsize_bytes << "\n\t"
                      << "intra_refresh: " << (intra_refresh_enabled ? "yes" : "no") << "\n\t"
                      << "bitrate: " << estimated_bitrate_kbps << "\n\t"
                      << "UDP: " << udp_datagram_size_bytes << "\n";

            return true;
        }

    private:

        std::string streaming_source_url;

        std::string streaming_topic_name;

        size_t origin_frame_width;

        size_t streaming_frame_width;

        size_t origin_frame_height;

        size_t streaming_frame_height;

        std::string origin_pixel_format;

        std::string codec_input_pixel_format;

        size_t origin_framerate;

        size_t streaming_framerate;

        // Codec settings

        size_t number_of_slices;

        size_t vbv_bufsize_bytes;

        bool intra_refresh_enabled;

        size_t estimated_bitrate_kbps;

        size_t udp_datagram_size_bytes;

        // special
        size_t trial_index;

    public:

        const std::string &get_streaming_source() const {
            return streaming_source_url;
        }

        const std::string &get_streaming_topic() const {
            return streaming_topic_name;
        }

        size_t get_frame_width() const {
            return origin_frame_width;
        }

        size_t get_streaming_frame_width() const {
            return streaming_frame_width;
        }

        size_t get_frame_height() const {
            return origin_frame_height;
        }

        size_t get_streaming_frame_height() const {
            return streaming_frame_height;
        }

        const std::string &get_pixel_format() const {
            return origin_pixel_format;
        }

        const std::string &get_codec_pixel_format() const {
            return codec_input_pixel_format;
        }

        size_t get_framerate() const {
            return origin_framerate;
        }

        size_t get_streaming_framerate() const {
            return streaming_framerate;
        }

        size_t get_number_of_slices() const {
            return number_of_slices;
        }

        size_t get_vbv_bufsize() const {
            return vbv_bufsize_bytes;
        }

        bool is_intra_refresh() const {
            return intra_refresh_enabled;
        }

        size_t get_bitrate() const {
            return estimated_bitrate_kbps;
        }

        size_t get_udp_datagram_size() const {
            return udp_datagram_size_bytes;
        }

    };
}

#endif //LIVE_VIDEO_STREAM_CONFIG_H
