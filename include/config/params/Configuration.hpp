#ifndef LIRS_RTSP_VIDEO_SERVER_CONFIGURATION_HPP
#define LIRS_RTSP_VIDEO_SERVER_CONFIGURATION_HPP

#include <set>
#include <vector>
#include <unordered_map>

namespace lirs {

    namespace config {

        namespace params {

            typedef std::unordered_map<std::string, std::string> topic_mapping_t;

            class GenericCameraParameters {

            public:

                // setters

                GenericCameraParameters &setFrameRate(uint16_t num, uint16_t den = 1) {
                    m_frameRate = std::make_pair(num, den);
                    return *this;
                }

                GenericCameraParameters &setResolution(uint16_t width, uint16_t height) {
                    m_resolution = std::make_pair(width, height);
                    return *this;
                }

                GenericCameraParameters &setPixelFormat(std::string pixelFormat) {
                    m_pixelFormat = std::move(pixelFormat);
                    return *this;
                }

                // getters

                std::pair<uint16_t, uint16_t> const &getFrameRate() const {
                    return m_frameRate;
                }

                std::pair<uint16_t, uint16_t> const &getResolution() const {
                    return m_resolution;
                }

                uint16_t getWidth() const {
                    return m_resolution.first;
                }

                uint16_t getHeight() const {
                    return m_resolution.second;
                }

                std::string const &getPixelFormat() const {
                    return m_pixelFormat;
                }

            private:

                std::pair<uint16_t, uint16_t> m_frameRate;

                std::pair<uint16_t, uint16_t> m_resolution;

                std::string m_pixelFormat;
            };

            class EncoderParameters {

            public:

                // default constructor

                EncoderParameters() : m_slices(0),
                                      m_bitrate(0),
                                      m_vbvBufSize(0),
                                      m_intraRefreshEnabled(false) {}

                // setters

                EncoderParameters &setTune(std::string tune) {
                    m_tune = std::move(tune);
                    return *this;
                }

                EncoderParameters &setPreset(std::string preset) {
                    m_preset = std::move(preset);
                    return *this;
                }

                EncoderParameters &setSlices(uint16_t slices) {
                    m_slices = slices;
                    return *this;
                }

                EncoderParameters &setBitrate(uint16_t bitrate) {
                    m_bitrate = bitrate;
                    return *this;
                }

                EncoderParameters &setVbvBufSize(uint16_t vbvBufSize) {
                    m_vbvBufSize = vbvBufSize;
                    return *this;
                }

                EncoderParameters &setIntraRefreshEnabled(bool intraRefreshEnabled) {
                    m_intraRefreshEnabled = intraRefreshEnabled;
                    return *this;
                }

                // getters

                std::string const &getTune() const {
                    return m_tune;
                }

                std::string const &getPreset() const {
                    return m_preset;
                }

                uint16_t getSlices() const {
                    return m_slices;
                }

                uint16_t getBitrate() const {
                    return m_bitrate;
                }

                uint16_t getVbvBufSize() const {
                    return m_vbvBufSize;
                }

                bool isIntraRefreshEnabled() const {
                    return m_intraRefreshEnabled;
                }

            private:

                std::string m_tune;

                std::string m_preset;

                uint16_t m_slices;

                uint16_t m_bitrate;

                uint16_t m_vbvBufSize;

                bool m_intraRefreshEnabled;
            };

            class CameraParameters {

            public:

                // setters

                CameraParameters &setName(std::string name) {
                    m_name = std::move(name);
                    return *this;
                }

                CameraParameters &setResource(std::string resource) {
                    m_resource = std::move(resource);
                    return *this;
                }

                CameraParameters &setInputParams(GenericCameraParameters const &inputParams) {
                    m_inputParams = inputParams;
                    return *this;
                }

                CameraParameters &setOutputParams(GenericCameraParameters const &outputParams) {
                    m_outputParams = outputParams;
                    return *this;
                }

                CameraParameters &setEncoderParams(EncoderParameters const &encoderParams) {
                    m_encoderParams = encoderParams;
                    return *this;
                }

                // getters

                std::string const &getName() const {
                    return m_name;
                }

                std::string const &getResource() const {
                    return m_resource;
                }

                GenericCameraParameters const &getInputParams() const {
                    return m_inputParams;
                }

                GenericCameraParameters const &getOutputParams() const {
                    return m_outputParams;
                }

                EncoderParameters const &getEncoderParams() const {
                    return m_encoderParams;
                }

            private:

                std::string m_name;

                std::string m_resource;

                GenericCameraParameters m_inputParams;

                GenericCameraParameters m_outputParams;

                EncoderParameters m_encoderParams;
            };

            class ServerParameters {

            public:

                // default constructor
                ServerParameters() : m_maxPacketSize(0),
                                     m_rtspPortNum(0),
                                     m_maxBufSize(0),
                                     m_httpEnabled(false),
                                     m_httpPortNum(0),
                                     m_cameraTopicMappings({}) {}

                ~ServerParameters() {
                    m_cameraTopicMappings.clear();
                }

                // setters

                ServerParameters &setMaxPacketSize(uint16_t maxPacketSize) {
                    m_maxPacketSize = maxPacketSize;
                    return *this;
                }

                ServerParameters &setRtspPortNum(uint16_t rtspPortNum) {
                    m_rtspPortNum = rtspPortNum;
                    return *this;
                }

                ServerParameters &setTopicPrefix(std::string topicPrefix) {
                    m_topicPrefix = std::move(topicPrefix);
                    return *this;
                }

                ServerParameters &setMaxBufSize(uint32_t maxBufSize) {
                    m_maxBufSize = maxBufSize;
                    return *this;
                }

                ServerParameters &setHttpEnabled(bool httpEnabled) {
                    m_httpEnabled = httpEnabled;
                    return *this;
                }

                ServerParameters &setHttpPortNum(uint16_t httpPortNum) {
                    m_httpPortNum = httpPortNum;
                    return *this;
                }

                bool addCameraTopic(std::string cameraName, std::string topic) {

                    auto search = m_cameraTopicMappings.find(cameraName);

                    if (search == m_cameraTopicMappings.end()) {
                        m_cameraTopicMappings.emplace(std::move(cameraName), std::move(topic));
                        return true;
                    }

                    return false;
                }

                // getters

                uint16_t getMaxPacketSize() const {
                    return m_maxPacketSize;
                }

                uint16_t getRtspPortNum() const {
                    return m_rtspPortNum;
                }

                std::string const &getTopicPrefix() const {
                    return m_topicPrefix;
                }

                uint32_t getMaxBufSize() const {
                    return m_maxBufSize;
                }

                bool isHttpEnabled() const {
                    return m_httpEnabled;
                }

                uint16_t getHttpPortNum() const {
                    return m_httpPortNum;
                }

                topic_mapping_t const &getCameraTopicMappings() const {
                    return m_cameraTopicMappings;
                }

                // throws out_of_range exception
                std::string const &getCameraTopic(std::string const &cameraName) const {
                    return m_cameraTopicMappings.at(cameraName);
                }

            private:

                uint16_t m_maxPacketSize;

                uint16_t m_rtspPortNum;

                std::string m_topicPrefix;

                uint32_t m_maxBufSize;

                bool m_httpEnabled;

                uint16_t m_httpPortNum;

                topic_mapping_t m_cameraTopicMappings;
            };

            struct Configuration {

            public:

                // default constructor

                Configuration() : m_activeCameras({}), m_serverParams({}), m_cameraParams({}) {}

                // setters

                Configuration &setActiveCameras(std::set<std::string> activeCameras) {
                    m_activeCameras = std::move(activeCameras);
                    return *this;
                }

                Configuration &setServerParams(params::ServerParameters const &serverParams) {
                    m_serverParams = serverParams;
                    return *this;
                }

                bool addCameraParams(params::CameraParameters const &cameraParams) {

                    auto searchParams = m_cameraParams.find(cameraParams.getName());

                    if (searchParams == m_cameraParams.end()) {

                        m_cameraParams.emplace(cameraParams.getName(), cameraParams);

                        return true;
                    }

                    return false;
                }

                bool addActiveCamera(std::string const &cameraName) {

                    auto const searchCamera = m_activeCameras.find(cameraName);

                    if (searchCamera == m_activeCameras.end()) {
                        m_activeCameras.emplace(cameraName);
                        return true;
                    }

                    return false;
                }

                // getters

                std::set<std::string> const &getActiveCameras() const {
                    return m_activeCameras;
                }

                params::ServerParameters const &getServerParams() const {
                    return m_serverParams;
                }

                std::unordered_map<std::string, params::CameraParameters> const &getCameraParams() const {
                    return m_cameraParams;
                }

            private:

                std::set<std::string> m_activeCameras;

                params::ServerParameters m_serverParams;

                std::unordered_map<std::string, params::CameraParameters> m_cameraParams;
            };
        }
    }
}

#endif //LIRS_RTSP_VIDEO_SERVER_CONFIGURATION_HPP
