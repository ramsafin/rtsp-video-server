#include "Config.hpp"
#include "Transcoder.hpp"

namespace LIRS {

    Transcoder::~Transcoder() {

        stop();

        cleanup();

        LOG(WARN) << "Transcoder destructed: " << cameraName;
    }

    Transcoder::Transcoder(const Configuration &config, std::string const &cameraName, std::string const &cameraUrl)
            : config(config), cameraName(cameraName), cameraUrl(cameraUrl), needToStopFlag(false),
              isRunningFlag(false) {

        LOG(DEBUG) << "Constructing transcoder for " << cameraUrl;

        // get the pixel format enum
        this->rawPixFormat = av_get_pix_fmt(config.get_pixel_format().c_str());
        this->encoderPixFormat = av_get_pix_fmt(config.get_codec_pixel_format().c_str());
        assert(rawPixFormat != AV_PIX_FMT_NONE && encoderPixFormat != AV_PIX_FMT_NONE);

        LOG(DEBUG) << "Set pixel formats of the camera original/codec: " << config.get_pixel_format() << "/"
                   << config.get_codec_pixel_format();

        //set framerate
        frameRate = (AVRational) {static_cast<int>(config.get_framerate()), 1};

        registerAll();

        initializeDecoder();

        initializeEncoder();

        initializeConverter();

        initFilters();
    }

    void Transcoder::run() {

        // set the flag
        isRunningFlag.store(true);

        // read raw data from the device into the packet
        while (!needToStopFlag.load() && av_read_frame(decoderContext.formatContext, decodingPacket) == 0) {

            // check whether it is a video stream's data
            if (decodingPacket->stream_index == decoderContext.videoStream->index) {

                // fill raw frame with data from decoded packet
                if (decode(decoderContext.codecContext, rawFrame, decodingPacket)) {

                    // push frames to the buffer
                    int statusCode = av_buffersrc_add_frame_flags(bufferSrcCtx, rawFrame, AV_BUFFERSRC_FLAG_KEEP_REF);

                    if (statusCode < 0) continue; // workaround for buggy cameras

                    // pull frames from the filter graph
                    while (true) {

                        statusCode = av_buffersink_get_frame(bufferSinkCtx, filterFrame);

                        if (statusCode == AVERROR(EAGAIN) || statusCode == AVERROR_EOF) {
                            break;
                        }

                        assert(statusCode >= 0);

                        av_frame_make_writable(convertedFrame);

                        // convert raw frame into another pixel format
                        sws_scale(converterContext, reinterpret_cast<const uint8_t *const *>(filterFrame->data),
                                  filterFrame->linesize, 0, static_cast<int>(frameHeight),
                                  convertedFrame->data, convertedFrame->linesize);

                        // copy pts/dts, etc.
                        av_frame_copy_props(convertedFrame, filterFrame);

                        if (encode(encoderContext.codecContext, convertedFrame, encodingPacket) >= 0) {

                            // new encoded data is available (one NALU)
                            if (onEncodedDataCallback) {
                                onEncodedDataCallback(
                                        std::vector<uint8_t>(encodingPacket->data + NALU_START_CODE_BYTES_NUMBER,
                                                             encodingPacket->data + encodingPacket->size)
                                );
                            }
                        }

                        av_packet_unref(encodingPacket);
                    }

                    av_frame_unref(filterFrame);
                }
            }

            av_packet_unref(decodingPacket);
        }

        isRunningFlag.store(false);
    }

    void Transcoder::stop() {

        if (!isRunningFlag.load())
            return;

        needToStopFlag.store(true);

        // wait
        while (isRunningFlag.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    void Transcoder::registerAll() {

        LOG(DEBUG) << "Registering ffmpeg stuff";

        av_register_all();

        avdevice_register_all();

        avcodec_register_all();

        avfilter_register_all();
    }

    void Transcoder::initializeDecoder() {

        LOG(DEBUG) << "Initialize decoder of the camera " << cameraUrl;

        // holds the general information about the format (container)
        decoderContext.formatContext = avformat_alloc_context();

        LOG(DEBUG) << "Using Video4Linux2 API for decoding raw data";

        AVInputFormat *inputFormat = av_find_input_format("v4l2"); // using Video4Linux API for capturing

        auto frameResolutionStr = utils::concatParams({config.get_frame_width(), config.get_frame_height()}, "x");
        auto framerateStr = utils::concatParams({(size_t) frameRate.num, (size_t) frameRate.den}, "/");

        AVDictionary *options = nullptr;

        av_dict_set(&options, "video_size", frameResolutionStr.data(), 0);
        av_dict_set(&options, "pixel_format", av_get_pix_fmt_name(rawPixFormat), 0);
        av_dict_set(&options, "framerate", framerateStr.data(), 0);

        int statCode = avformat_open_input(&decoderContext.formatContext, cameraUrl.data(),
                                           inputFormat, &options);
        av_dict_free(&options);
        assert(statCode == 0);

        // get the info on all available streams
        statCode = avformat_find_stream_info(decoderContext.formatContext, nullptr);
        assert(statCode >= 0);

        av_dump_format(decoderContext.formatContext, 0, cameraName.data(), 0);

        // find video stream (if multiple video streams are available then you should choose one manually)
        int videoStreamIndex = av_find_best_stream(decoderContext.formatContext, AVMEDIA_TYPE_VIDEO, -1, -1,
                                                   &decoderContext.codec, 0);
        assert(videoStreamIndex >= 0);
        assert(decoderContext.codec);
        decoderContext.videoStream = decoderContext.formatContext->streams[videoStreamIndex];

        // create codec context (for each codec its own codec context)
        decoderContext.codecContext = avcodec_alloc_context3(decoderContext.codec);
        assert(decoderContext.codecContext);

        // copy video stream parameters to the codec context
        statCode = avcodec_parameters_to_context(decoderContext.codecContext, decoderContext.videoStream->codecpar);
        assert(statCode >= 0);

        // initialize the codec context to use the created codec context
        statCode = avcodec_open2(decoderContext.codecContext, decoderContext.codec, &options);
        assert(statCode == 0);

        // update parameters
        frameRate = decoderContext.videoStream->r_frame_rate;
        frameWidth = static_cast<size_t>(decoderContext.codecContext->width);
        frameHeight = static_cast<size_t>(decoderContext.codecContext->height);
        rawPixFormat = decoderContext.codecContext->pix_fmt;
        sourceBitRate = static_cast<size_t>(decoderContext.codecContext->bit_rate);

        LOG(DEBUG) << "Decoder params: width: " << frameWidth << ", height: " << frameHeight << ", pixel_fmt: "
                   << av_get_pix_fmt_name(rawPixFormat) << ", framerate: " << frameRate.num;

        // allocate decoding packet
        decodingPacket = av_packet_alloc();
        av_init_packet(decodingPacket);

        // allocate frame
        rawFrame = av_frame_alloc();
    }

    void Transcoder::initializeEncoder() {

        LOG(DEBUG) << "Initialize HEVC encoder";

        // allocate format context for an output format (null - no output file)
        int statCode = avformat_alloc_output_context2(&encoderContext.formatContext, nullptr, "null", nullptr);
        assert(statCode >= 0);

        encoderContext.codec = avcodec_find_encoder_by_name("libx265");
        assert(encoderContext.codec);

        // create new video output stream (dummy)
        encoderContext.videoStream = avformat_new_stream(encoderContext.formatContext, encoderContext.codec);
        assert(encoderContext.videoStream);
        encoderContext.videoStream->id = encoderContext.formatContext->nb_streams - 1;

        // create codec context (for each codec new codec context)
        encoderContext.codecContext = avcodec_alloc_context3(encoderContext.codec);
        assert(encoderContext.codecContext);

        // set up parameters
        encoderContext.codecContext->width = static_cast<int>(config.get_streaming_frame_width());
        encoderContext.codecContext->height = static_cast<int>(config.get_streaming_frame_height());

        encoderContext.codecContext->profile = FF_PROFILE_HEVC_MAIN;

        encoderContext.codecContext->time_base = (AVRational) {1, static_cast<int>(config.get_streaming_framerate())};
        encoderContext.codecContext->framerate = (AVRational) {static_cast<int>(config.get_streaming_framerate()), 1};

        // set encoder's pixel format (it is advised to use yuv420p)
        encoderContext.codecContext->pix_fmt = encoderPixFormat;

        if (encoderContext.formatContext->flags & AVFMT_GLOBALHEADER) {
            encoderContext.codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        }

        // copy encoder parameters to the video stream parameters
        avcodec_parameters_from_context(encoderContext.videoStream->codecpar, encoderContext.codecContext);

        AVDictionary *options = nullptr;

        // the faster you get, the less compression is achieved
        av_dict_set(&options, "preset", "veryfast", 0);

        // optimization for fast encoding and low latency streaming
        av_dict_set(&options, "tune", "zerolatency", 0);

        av_dict_set(&options, "b", utils::to_string_with_prefix(config.get_bitrate(), "K").data(), 0);

        // set additional codec options
        av_opt_set(encoderContext.codecContext->priv_data, "x265-params",
                   config.get_codec_params_str().data(), 0);

        // open the output format to use given codec
        statCode = avcodec_open2(encoderContext.codecContext, encoderContext.codec, &options);
        av_dict_free(&options);
        assert(statCode == 0);

        // initializes time base automatically
        statCode = avformat_write_header(encoderContext.formatContext, nullptr);
        assert(statCode >= 0);

        // report info to the console
        av_dump_format(encoderContext.formatContext, encoderContext.videoStream->index, "null", 1);

        // allocate encoding packet
        encodingPacket = av_packet_alloc();
        av_init_packet(encodingPacket);

        LOG(DEBUG) << "Encoder params: width: " << config.get_streaming_frame_width() << ", height: "
                   << config.get_streaming_frame_height() << ", pixel_fmt: "
                   << av_get_pix_fmt_name(encoderPixFormat) << ", framerate: " << config.get_streaming_framerate();

    }

    void Transcoder::initializeConverter() {

        LOG(DEBUG) << "Initialize converter " << av_get_pix_fmt_name(rawPixFormat) << " -> "
                   << av_get_pix_fmt_name(encoderPixFormat) << ", " << frameWidth << "x" << frameHeight
                   << " -> " << config.get_streaming_frame_width() << "x" << config.get_streaming_frame_height();

        // allocate frame to be used in converter
        convertedFrame = av_frame_alloc();
        convertedFrame->width = static_cast<int>(config.get_streaming_frame_width());
        convertedFrame->height = static_cast<int>(config.get_streaming_frame_height());
        convertedFrame->format = encoderPixFormat;
        int statCode = av_frame_get_buffer(convertedFrame, 0); // ref counted frame
        assert(statCode == 0);

        // create converter from raw pixel format to encoder supported pixel format
        converterContext = sws_getCachedContext(nullptr, static_cast<int>(frameWidth), static_cast<int>(frameHeight),
                                                rawPixFormat,
                                                convertedFrame->width, convertedFrame->height,
                                                encoderPixFormat,
                                                SWS_LANCZOS, nullptr, nullptr, nullptr);
    }

    void Transcoder::initFilters() {

        // allocate filter frame (where the filtered frame will be stored)
        filterFrame = av_frame_alloc();

        // create buffer source and sink
        AVFilter *bufferSrc = avfilter_get_by_name("buffer");
        AVFilter *bufferSink = avfilter_get_by_name("buffersink");

        AVFilterInOut *outputs = avfilter_inout_alloc();
        AVFilterInOut *inputs = avfilter_inout_alloc();

        // allocate filter graph
        filterGraph = avfilter_graph_alloc();

        char args[128];
        snprintf(args, sizeof(args), "width=%d:height=%d:pix_fmt=%d:time_base=%d/%d:sar=%d/%d:frame_rate=%d/%d",
                 (int) frameWidth, (int) frameHeight, rawPixFormat, decoderContext.videoStream->time_base.num,
                 decoderContext.videoStream->time_base.den, decoderContext.videoStream->sample_aspect_ratio.num,
                 decoderContext.videoStream->sample_aspect_ratio.den, frameRate.num, frameRate.den);

        // create buffer source with the specified params
        auto status = avfilter_graph_create_filter(&bufferSrcCtx, bufferSrc, "in", args, nullptr, filterGraph);
        assert(status >= 0);

        // create buffer sink
        status = avfilter_graph_create_filter(&bufferSinkCtx, bufferSink, "out", nullptr, nullptr, filterGraph);
        assert(status >= 0);

        outputs->name = av_strdup("in");
        outputs->filter_ctx = bufferSrcCtx;
        outputs->pad_idx = 0;
        outputs->next = nullptr;

        inputs->name = av_strdup("out");
        inputs->filter_ctx = bufferSinkCtx;
        inputs->pad_idx = 0;
        inputs->next = nullptr;

        // create filter query
        char frameStepFilterQuery[16];

        if (this->filterQuery.empty()) {

            snprintf(frameStepFilterQuery, sizeof(frameStepFilterQuery), "fps=fps=%d/%d",
                     static_cast<int>(config.get_streaming_framerate()), 1);

            // add graph represented by the filter query
            status = avfilter_graph_parse(filterGraph, frameStepFilterQuery, inputs, outputs, nullptr);
            assert(status >= 0);

        } else {

            status = avfilter_graph_parse(filterGraph, filterQuery.c_str(), inputs, outputs, nullptr);
            assert(status >= 0);
        }

        status = avfilter_graph_config(filterGraph, nullptr);
        assert(status >= 0);
    }

    int Transcoder::decode(AVCodecContext *codecContext, AVFrame *frame, AVPacket *packet) {

        // send a packet to be filled with raw data
        int statCode = avcodec_send_packet(codecContext, packet);

        if (statCode < 0) {
//            LOG(ERROR) << "Error sending a packet for decoding: " << config.get_streaming_source();
            return statCode;
        }

        // receive decoded raw frame
        statCode = avcodec_receive_frame(codecContext, frame);

        if (statCode == AVERROR(EAGAIN) || statCode == AVERROR_EOF) {
//            LOG(WARN) << "No frames available or end of file has been reached";
            return statCode;
        }

        if (statCode < 0) {
//            LOG(ERROR) << "Error during decoding";
            return statCode;
        }

        return true;
    }

    int Transcoder::encode(AVCodecContext *codecContext, AVFrame *frame, AVPacket *packet) {

        // send a raw frame to be encoded
        int statCode = avcodec_send_frame(codecContext, frame);

        if (statCode < 0) {
//            LOG(ERROR) << "Error during sending frame for encoding";
            return statCode;
        }

        // receive a packet containing encoded data
        statCode = avcodec_receive_packet(codecContext, packet);

        if (statCode == AVERROR(EAGAIN) || statCode == AVERROR_EOF) {
//            LOG(WARN) << "EAGAIN or EOF while encoding";
            return statCode;

        }

        if (statCode < 0) {
//            LOG(ERROR) << "Error during receiving packet for encoding";
            return statCode;
        }

        return statCode;
    }

    void Transcoder::cleanup() {

        avfilter_graph_free(&filterGraph);

        // close dummy file
        avio_close(encoderContext.formatContext->pb);

        // cleanup converter
        sws_freeContext(converterContext);

        // cleanup packets used for decoding and encoding
        av_packet_free(&decodingPacket);
        av_packet_free(&encodingPacket);

        // cleanup frames for decoding and encoding
        av_frame_free(&rawFrame);
        av_frame_free(&convertedFrame);
        av_frame_free(&filterFrame);

        // cleanup decoder and encoder codec contexts
        avcodec_free_context(&decoderContext.codecContext);
        avcodec_free_context(&encoderContext.codecContext);

        // close input format for the video device
        avformat_close_input(&decoderContext.formatContext);

        // cleanup decoder and encoder format contexts
        avformat_free_context(decoderContext.formatContext);
        avformat_free_context(encoderContext.formatContext);

        // reset all class members
        decoderContext = {};
        encoderContext = {};

        LOG(DEBUG) << "Cleanup transcoder!";
    }

    void Transcoder::setOnEncodedDataCallback(std::function<void(std::vector<uint8_t> &&)> callback) {
        onEncodedDataCallback = std::move(callback);
    }

    const bool Transcoder::isRunning() const {
        return isRunningFlag.load();
    }

    Configuration const &Transcoder::getConfig() const {
        return config;
    }

    std::string const &Transcoder::getCameraName() const {
        return cameraName;
    }

    std::string const &Transcoder::getCameraUrl() const {
        return cameraUrl;
    }

    // TranscoderContext

    TranscoderContext::TranscoderContext() : formatContext(nullptr), codecContext(nullptr),
                                             codec(nullptr), videoStream(nullptr) {}
}
