#include <yaml-cpp/yaml.h>

#include "config/YamlConfigLoader.hpp"
#include "utils/Logger.hpp"

namespace lirs {

    namespace config {

        YamlConfigLoader::YamlConfigLoader(std::string const &configResource)
                : ConfigLoader(configResource) {}

        bool YamlConfigLoader::load(params::Configuration &configuration) {

            auto configRoot = YAML::LoadFile(m_configResource);

            if (configRoot.IsNull()) {
                LOG(ERROR) << "Cannot parse YAML configuration file: is it empty?";
                return false;
            }

            auto configNode = configRoot["config"];

            if (!configNode) {
                LOG(ERROR) << "Cannot parse YAML configuration file: no 'config' key.";
                return false;
            }

            auto serverConfigNode = configNode["server"];

            if (!serverConfigNode) {
                LOG(ERROR) << "Cannot parse YAML configuration file: no 'server' key.";
                return false;
            }

            auto camerasConfigNode = configNode["cameras"];

            if (!camerasConfigNode) {
                LOG(ERROR) << "Cannot parse YAML configuration file: no 'cameras' key.";
                return false;
            }

            // parse 'active_cameras'
            auto activeCamerasNode = configRoot["active_cameras"];

            if (!activeCamerasNode) {

                LOG(ERROR) << "Cannot parse YAML configuration file: no active cameras are specified.";
                return false;
            }

            if (activeCamerasNode.IsSequence()) {

                for (YAML::detail::iterator_value const &activeCamera : activeCamerasNode) {
                    configuration.addActiveCamera(activeCamera.as<std::string>());
                }

            } else if (activeCamerasNode.IsScalar()) {

                configuration.addActiveCamera(activeCamerasNode.as<std::string>());

            } else {
                LOG(ERROR) << "Cannot parse YAML configuration file: "
                           << "'active_cameras' is neither scalar nor sequence.";
            }

            // parse 'cameras'
            if (activeCamerasNode.size() > camerasConfigNode.size()) {

                LOG(ERROR) << "Cannot parse YAML configuration file: there are more 'active_cameras' specified "
                           << "than actual number of defined camera configurations.";

                return false;
            }

            for (auto const &activeCamera : configuration.getActiveCameras()) {

                auto activeCameraNode = camerasConfigNode[activeCamera];

                if (!activeCameraNode.IsDefined()) {

                    LOG(ERROR) << "Cannot parse YAML configuration file: no '" << activeCamera
                               << "' is defined in 'cameras'.";

                    return false;
                }

                // parse camera config
                params::CameraParameters cameraParameters;

                cameraParameters.setName(activeCamera);
                cameraParameters.setResource(activeCameraNode["resource"].as<std::string>());


                auto inputParamsNode = activeCameraNode["input"];

                if (!inputParamsNode) {

                    LOG(ERROR) << "Cannot parse YAML configuration file: no 'input' key in '" << activeCamera
                               << "' configuration.";

                    return false;
                }

                auto outputParamsNode = activeCameraNode["output"];

                if (!outputParamsNode) {

                    LOG(ERROR) << "Cannot parse YAML configuration file: no 'output' key in '" << activeCamera
                               << "' configuration.";

                    return false;
                }

                auto encoderParamsNode = activeCameraNode["encoder"];

                if (!encoderParamsNode) {

                    LOG(ERROR) << "Cannot parse YAML configuration file: no 'encoder' key in '" << activeCamera
                               << "' configuration.";

                    return false;
                }

                // input, output and encoder check

                if (!inputParamsNode["frame_rate"]
                    || !inputParamsNode["resolution"]
                    || !inputParamsNode["pixel_format"]) {

                    LOG(ERROR) << "Cannot parse YAML configuration file: some parameters in 'input' are not presented.";

                    return false;
                }

                if (!outputParamsNode["frame_rate"]
                    || !outputParamsNode["resolution"]
                    || !outputParamsNode["pixel_format"]) {

                    LOG(ERROR)
                            << "Cannot parse YAML configuration file: some parameters in 'output' are not presented.";

                    return false;
                }

                if (!encoderParamsNode["bitrate"]
                    || !encoderParamsNode["vbv_buf_size"]
                    || !encoderParamsNode["preset"]
                    || !encoderParamsNode["tune"]
                    || !encoderParamsNode["slices"]
                    || !encoderParamsNode["intra_refresh_enabled"]) {

                    LOG(ERROR)
                            << "Cannot parse YAML configuration file: some parameters in 'encoder' are not presented.";

                    return false;
                }

                // input

                params::GenericCameraParameters inputParams;
                params::GenericCameraParameters outputParams;
                params::EncoderParameters encoderParams;

                inputParams.setFrameRate(inputParamsNode["frame_rate"]["num"].as<std::uint16_t>(),
                        inputParamsNode["frame_rate"]["den"].as<std::uint16_t>());

                inputParams.setResolution(inputParamsNode["resolution"]["width"].as<std::uint16_t>(),
                        inputParamsNode["resolution"]["height"].as<std::uint16_t>());

                inputParams.setPixelFormat(inputParamsNode["pixel_format"].as<std::string>());

                // output

                outputParams.setFrameRate(outputParamsNode["frame_rate"]["num"].as<std::uint16_t>(),
                        outputParamsNode["frame_rate"]["den"].as<std::uint16_t>());

                outputParams.setResolution(outputParamsNode["resolution"]["width"].as<std::uint16_t>(),
                        outputParamsNode["resolution"]["height"].as<std::uint16_t>());

                outputParams.setPixelFormat(outputParamsNode["pixel_format"].as<std::string>());

                // encoder

                encoderParams.setBitrate(encoderParamsNode["bitrate"].as<std::uint16_t>());

                encoderParams.setVbvBufSize(encoderParamsNode["vbv_buf_size"].as<std::uint16_t>());

                encoderParams.setPreset(encoderParamsNode["preset"].as<std::string>());

                encoderParams.setTune(encoderParamsNode["tune"].as<std::string>());

                encoderParams.setSlices(encoderParamsNode["slices"].as<std::uint16_t>());

                encoderParams.setIntraRefreshEnabled(encoderParamsNode["intra_refresh_enabled"].as<bool>());

                // set refs
                cameraParameters.setInputParams(inputParams);
                cameraParameters.setOutputParams(outputParams);
                cameraParameters.setEncoderParams(encoderParams);

                configuration.addCameraParams(cameraParameters);
            }

            // server config

            params::ServerParameters serverParams;

            serverParams.setMaxPacketSize(serverConfigNode["max_packet_size"].as<std::uint16_t>());

            serverParams.setRtspPortNum(serverConfigNode["rtsp_port_num"].as<uint16_t>());

            serverParams.setTopicPrefix(serverConfigNode["topic_prefix"].as<std::string>(""));

            serverParams.setMaxBufSize(serverConfigNode["max_buf_size"].as<uint32_t>());

            serverParams.setHttpEnabled(serverConfigNode["http_enabled"].as<bool>());

            if (serverParams.isHttpEnabled())
                serverParams.setHttpPortNum(serverConfigNode["http_port_num"].as<std::uint16_t>());

            auto mappingsNode = serverConfigNode["mappings"];

            if (!mappingsNode || mappingsNode.size() == 0 || !mappingsNode.IsMap()) {

                LOG(ERROR) << "Cannot parse YAML configuration file: no 'mappings' defined.";

                return false;
            }

            auto &activeCamerasSet = configuration.getActiveCameras();

            if (mappingsNode.size() < activeCamerasSet.size()) {
                LOG(ERROR) << "Cannot parse YAML configuration file: number of mappings is less than number of active cameras.";
                return false;
            }

            params::topic_mapping_t topicMappings{};

            for (YAML::const_iterator mappingIter = mappingsNode.begin(); mappingIter != mappingsNode.end(); ++mappingIter) {

                auto cameraName = mappingIter->first.as<std::string>();

                // check if camera is in active cameras list
                // and all active cameras are presented here

                auto topic = mappingIter->second.as<std::string>();

                topicMappings.emplace(cameraName, topic);
            }

            for (auto &activeCamera : activeCamerasSet) {

                auto searchTopic = topicMappings.find(activeCamera);

                if (searchTopic == topicMappings.end()) {
                    LOG(ERROR) << "Cannot parse YAML configuration file: '" <<  activeCamera << "' has no mapping.";
                    return false;
                };

                serverParams.addCameraTopic(activeCamera, searchTopic->second);
            }

            configuration.setServerParams(serverParams);

            return true;
        }
    }
}