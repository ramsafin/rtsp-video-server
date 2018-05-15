#ifndef LIVE_VIDEO_STREAM_TRANSCODER_HPP
#define LIVE_VIDEO_STREAM_TRANSCODER_HPP

#include <atomic>
#include <functional>
#include <string>
#include <thread>

#include "Logger.hpp"
#include "Utils.hpp"
#include "Config.hpp"

#ifdef __cplusplus
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixdesc.h>
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
}
#endif

namespace LIRS {

    /**
     * Structure representing context for decoding or encoding process.
     * @note: user must manually destruct this object (no destructor).
     */
    typedef struct TranscoderContext {

        /**
         * Input or output context format.
         */
        AVFormatContext *formatContext;

        /**
         * Decoding/encoding context for the decoder or encoder.
         */
        AVCodecContext *codecContext;

        /**
         * Encoder or decoder.
         */
        AVCodec *codec;

        /**
         * Video data stream.
         */
        AVStream *videoStream;

        /**
         * Default constructor.
         */
        TranscoderContext();

    } TranscoderContext;


    /**
     * Transcoder decodes video resource (captures video frames) and encodes it.
     */
    class Transcoder {

    public:

        /**
         * Constructs transcoder using configuration.
         *
         * @param config - configuration to create transcoder.
         */
        explicit Transcoder(const Configuration &config, std::string const &, std::string const &);

        /**
         * Don't allow to copy this object.
         */
        Transcoder(const Transcoder &) = delete;

        /**
         * Don't allow copy assignment operator to be used on this object.
         */
        Transcoder &operator=(const Transcoder &) = delete;

        /**
         * Destructor of the transcoder.
         * @note the actual destruction occurs in the cleanup().
         */
        ~Transcoder();

        /**
         * Starts the process of decoding / encoding data from the video source.
         * Sets the isPlayingFlag to true.
         */
        void run();

        /**
         * Stops capturing, encoding.
         * Also sets the isPlayingFlag to false.
         */
        void stop();

        /**
         * Sets callback function which indicates that a new encoded video data is available.
         *
         * @param callback - callback function.
         */
        void setOnEncodedDataCallback(std::function<void(std::vector<uint8_t> &&)> callback);

        /**
         * Returns this object's configuration.
         *
         * @return configuration of this transcoder.
         */
        Configuration const &getConfig() const;

        /**
         * Whether the resource is running: captures frames and produces encoded data.
         *
         * @return true - if the resource is running, otherwise - false.
         */
        const bool isRunning() const;

        std::string const &getCameraName() const;

        std::string const &getCameraUrl() const;


    private:

        /* parameters */

        Configuration const &config;

        std::string cameraName;

        std::string cameraUrl;

        /**
         * Frame width.
         */
        size_t frameWidth;

        /**
         * Frame height.
         */
        size_t frameHeight;

        /**
         * Raw video data's pixel format.
         */
        AVPixelFormat rawPixFormat;

        /**
         * Encoded video data pixel format.
         */
        AVPixelFormat encoderPixFormat;

        /**
         * Device's supported framerate.
         */
        AVRational frameRate;

        /**
         * Video source's bitrate (raw video data).
         */
        size_t sourceBitRate;

        /**
         * Decoder video context.
         * Used for decoding.
         */
        TranscoderContext decoderContext;

        /**
         * Encoder video context.
         * Used for encoding.
         */
        TranscoderContext encoderContext;

        /**
         * Holds raw video data and additional information (resolution, etc.)
         */
        AVFrame *rawFrame;

        /**
         * Holds converted frame data (from one pixel format to another one).
         */
        AVFrame *convertedFrame;

        /**
         * Frame retrieved from the filter.
         */
        AVFrame *filterFrame;

        /**
         * Decoding packet (is sent to the decoder).
         */
        AVPacket *decodingPacket;

        /**
         * Encoding packet (holds encoded data).
         */
        AVPacket *encodingPacket;

        /**
         * Conversion context from one pixel format to another one.
         */
        SwsContext *converterContext;

        /**
         * Filter query.
         * Filter graph is constructed from it.
         */
        std::string filterQuery;

        /**
         * Filter graph used to create complex filter chains.
         */
        AVFilterGraph *filterGraph;

        /**
         * Filter context for buffer source.
         * Frames to be filtered are passed to the buffer source.
         */
        AVFilterContext *bufferSrcCtx;

        /**
         * Filter context for buffer sink.
         * Frames to be filtered are retrieved from the buffer sink.
         */
        AVFilterContext *bufferSinkCtx;

        std::atomic_bool needToStopFlag;

        std::atomic_bool isRunningFlag;

        /**
         * Callback function called when new encoded video data is available.
         */
        std::function<void(std::vector<uint8_t> &&)> onEncodedDataCallback;

        /** constants **/

        /**
         * NALU start code bytes number (first 4 bytes, {0x0, 0x0, 0x0, 0x1}).
         * Used to truncate start codes from the encoded data.
         */
        constexpr static unsigned int NALU_START_CODE_BYTES_NUMBER = 4U;

        /* Methods */

        /**
         * Registers ffmpeg codecs, etc.
         */
        void registerAll();

        /**
         * Initializes decoder in order to capture raw frames from the video source.
         */
        void initializeDecoder();

        /**
         * Initializes encoder in order to encode raw frames.
         * Tune encoder here using different profiles, tune options.
         */
        void initializeEncoder();

        /**
         * Initializes converter from raw pixel format to the encoder supported pixel format.
         */
        void initializeConverter();

        /**
         * Initializes filters, e.g. 'framestep', 'fps'.
         * See filter docs.
         */
        void initFilters();

        /**
         * Captures raw video frame from the device.
         *
         * @param codecContext - codec context (for decoding).
         * @param frame - captured raw video frame.
         * @param packet - packet to be sent to the decoder in order to get frame.
         * @return >= 0 if success, otherwise error occurred.
         */
        int decode(AVCodecContext *codecContext, AVFrame *frame, AVPacket *packet);

        /**
         * Encodes raw frame and stores encode data in packet.
         *
         * @param codecContext - codec context (for encoding).
         * @param frame - raw frame to be sent to the encoder.
         * @param packet - packet with encoded data set by the encoder.
         * @return >= 0 - success, othwerwise error occurred.
         */
        int encode(AVCodecContext *codecContext, AVFrame *frame, AVPacket *packet);

        /**
         * Close all resources, free allocated memory, etc.
         */
        void cleanup();

    };
}

#endif //LIVE_VIDEO_STREAM_TRANSCODER_HPP
