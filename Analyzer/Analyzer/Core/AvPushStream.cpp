#include "AvPushStream.h"
#include "Config.h"
#include "Utils/Log.h"
#include "Utils/Common.h"
#include "Control.h"
#include "ControlExecutor.h"
#include "Analyzer.h"
extern "C" {
#include "libswscale/swscale.h"
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
}
#pragma warning(disable: 4996)

namespace AVSAnalyzer {
    AvPushStream::AvPushStream(Config* config, Control* control) :
        mConfig(config),
        mControl(control)
    {
        LOGI("");
        initVideoFrameQueue();
        initAudioFrameQueue();
    }

    AvPushStream::~AvPushStream()
    {
        LOGI("");
        closeConnect();

    }


    bool AvPushStream::connect() {

        if (avformat_alloc_output_context2(&mFmtCtx, NULL, "flv", mControl->pushStreamUrl.data()) < 0) {
            LOGE("avformat_alloc_output_context2 error");
            return false;
        }

        // 初始化视频编码器 start
        AVCodec* videoCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
        if (!videoCodec) {
            LOGE("avcodec_find_decoder error");
            return false;
        }
        mVideoCodecCtx = avcodec_alloc_context3(videoCodec);
        if (!mVideoCodecCtx) {
            LOGE("avcodec_alloc_context3 error");
            return false;
        }
        int bit_rate = 300 * 1024 * 8;  //压缩后每秒视频的bit位大小 300kB

        // CBR：Constant BitRate - 固定比特率
    //    mVideoCodecCtx->flags |= AV_CODEC_FLAG_QSCALE;
    //    mVideoCodecCtx->bit_rate = bit_rate;
    //    mVideoCodecCtx->rc_min_rate = bit_rate;
    //    mVideoCodecCtx->rc_max_rate = bit_rate;
    //    mVideoCodecCtx->bit_rate_tolerance = bit_rate;

        //VBR
        mVideoCodecCtx->flags |= AV_CODEC_FLAG_QSCALE;
        mVideoCodecCtx->rc_min_rate = bit_rate / 2;
        mVideoCodecCtx->rc_max_rate = bit_rate / 2 + bit_rate;
        mVideoCodecCtx->bit_rate = bit_rate;

        //ABR：Average Bitrate - 平均码率
    //    mVideoCodecCtx->bit_rate = bit_rate;

        mVideoCodecCtx->codec_id = videoCodec->id;
        mVideoCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;// 不支持AV_PIX_FMT_BGR24直接进行编码
        mVideoCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
        mVideoCodecCtx->width = mControl->videoWidth;
        mVideoCodecCtx->height = mControl->videoHeight;
        mVideoCodecCtx->time_base = { 1,mControl->videoFps };
        mVideoCodecCtx->framerate = { mControl->videoFps, 1 };
        mVideoCodecCtx->gop_size = 12;
        mVideoCodecCtx->max_b_frames = 0;
        mVideoCodecCtx->thread_count = 1;

        //mVideoCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;  //全局参数

        unsigned char sps_pps[] = { 0x00 ,0x00 ,0x01,0x67,0x42,0x00 ,0x2a ,0x96 ,0x35 ,0x40 ,0xf0 ,0x04 ,
                            0x4f ,0xcb ,0x37 ,0x01 ,0x01 ,0x01 ,0x40 ,0x00 ,0x01 ,0xc2 ,0x00 ,0x00 ,0x57 ,
                            0xe4 ,0x01 ,0x00 ,0x00 ,0x00 ,0x01 ,0x68 ,0xce ,0x3c ,0x80, 0x00 };

        mVideoCodecCtx->extradata_size = sizeof(sps_pps);
        mVideoCodecCtx->extradata = (uint8_t*)av_mallocz(mVideoCodecCtx->extradata_size);
        memcpy(mVideoCodecCtx->extradata, sps_pps, mVideoCodecCtx->extradata_size);



        //unsigned char sps_pps[] = { 0x00 ,0x00 ,0x01,0x67,0x42,0x00 ,0x2a ,0x96 ,0x35 ,0x40 ,0xf0 ,0x04 ,
        //                        0x4f ,0xcb ,0x37 ,0x01 ,0x01 ,0x01 ,0x40 ,0x00 ,0x01 ,0xc2 ,0x00 ,0x00 ,0x57 ,
        //                        0xe4 ,0x01 ,0x00 ,0x00 ,0x00 ,0x01 ,0x68 ,0xce ,0x3c ,0x80, 0x00 };
        //mVideoCodecCtx->extradata_size = sizeof(sps_pps);
        //mVideoCodecCtx->extradata = (uint8_t*)av_mallocz(mVideoCodecCtx->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
        //memset(mVideoCodecCtx->extradata, 0, sizeof(sps_pps) + AV_INPUT_BUFFER_PADDING_SIZE);
        //memcpy(mVideoCodecCtx->extradata, sps_pps, sizeof(sps_pps));


        //unsigned char sps_pps[23] = { 0x00, 0x00, 0x00, 0x01, 0x67, 0x42, 0x00, 0x0a, 0xf8, 0x0f, 0x00, 0x44, 0xbe, 0x8,
        //                              0x00, 0x00, 0x00, 0x01, 0x68, 0xce, 0x38, 0x80 };
        //mVideoCodecCtx->extradata_size = 23;
        //mVideoCodecCtx->extradata = (uint8_t*)av_malloc(23 + AV_INPUT_BUFFER_PADDING_SIZE);
        //memset(mVideoCodecCtx->extradata, 0, 23 + AV_INPUT_BUFFER_PADDING_SIZE);
        //memcpy(mVideoCodecCtx->extradata, sps_pps, 23);


        AVDictionary* video_codec_options = NULL;
        av_dict_set(&video_codec_options, "profile", "main", 0);
        av_dict_set(&video_codec_options, "preset", "superfast", 0);
        av_dict_set(&video_codec_options, "tune", "fastdecode", 0);

        if (avcodec_open2(mVideoCodecCtx, videoCodec, &video_codec_options) < 0) {
            LOGE("avcodec_open2 error");
            return false;
        }

        mVideoStream = avformat_new_stream(mFmtCtx, videoCodec);
        if (!mVideoStream) {
            LOGE("avformat_new_stream error");
            return false;
        }
        mVideoStream->id = mFmtCtx->nb_streams - 1;
        // stream的time_base参数非常重要，它表示将现实中的一秒钟分为多少个时间基, 在下面调用avformat_write_header时自动完成
        avcodec_parameters_from_context(mVideoStream->codecpar, mVideoCodecCtx);
        mVideoIndex = mVideoStream->id;
        // 初始化视频编码器 end



        if (mControl->audioIndex > -1) {
            // 初始化音频编码器 start
            AVCodec* audioCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
            if (!audioCodec) {
                LOGE("avcodec_find_decoder error");
                return false;
            }
            mAudioCodecCtx = avcodec_alloc_context3(audioCodec);
            if (!mAudioCodecCtx) {
                LOGE("avcodec_alloc_context3 error");
                return false;
            }


            mAudioCodecCtx->codec_id = audioCodec->id;
            mAudioCodecCtx->codec_type = AVMEDIA_TYPE_AUDIO;
            mAudioCodecCtx->bit_rate = 128000;//音频码率
            mAudioCodecCtx->channel_layout = AV_CH_LAYOUT_STEREO;// 声道层
            mAudioCodecCtx->channels = av_get_channel_layout_nb_channels(mAudioCodecCtx->channel_layout);// 声道数
            mAudioCodecCtx->sample_rate = 44100;//采样率
            mAudioCodecCtx->frame_size = 1024;//每帧单个通道的采样点数
            mAudioCodecCtx->profile = FF_PROFILE_AAC_LOW;
            mAudioCodecCtx->sample_fmt = AV_SAMPLE_FMT_FLTP;//ffmpeg对于AAC编码的采样点格式默认只支持AV_SAMPLE_FMT_FLTP，通常PCM文件或者播放器播放的音频采样点格式是 AV_SAMPLE_FMT_S16
            mAudioCodecCtx->time_base = { 1024, 44100 };
            mAudioCodecCtx->framerate = { 44100, 1024 };

            // 将编码器上下文和编码器进行关联
            if (avcodec_open2(mAudioCodecCtx, audioCodec, NULL) < 0) {
                LOGE("avcodec_open2 error");
                return false;
            }
            mAudioStream = avformat_new_stream(mFmtCtx, audioCodec);
            if (!mAudioStream) {
                LOGE("avformat_new_stream error");
                return false;
            }
            mAudioStream->id = mFmtCtx->nb_streams - 1;
            avcodec_parameters_from_context(mAudioStream->codecpar, mAudioCodecCtx);
            mAudioIndex = mAudioStream->id;

            // 初始化音频编码器 end

        }

        av_dump_format(mFmtCtx, 0, mControl->pushStreamUrl.data(), 1);

        // open output url
        if (!(mFmtCtx->oformat->flags & AVFMT_NOFILE)) {
            if (avio_open(&mFmtCtx->pb, mControl->pushStreamUrl.data(), AVIO_FLAG_WRITE) < 0) {
                LOGE("avio_open error url:%s", mControl->pushStreamUrl.data());
                return false;
            }
        }


        AVDictionary* fmt_options = NULL;
        //av_dict_set(&fmt_options, "bufsize", "1024", 0);
        av_dict_set(&fmt_options, "rw_timeout", "30000000", 0); //设置rtmp/http-flv连接超时（单位 us）
        av_dict_set(&fmt_options, "stimeout", "30000000", 0);   //设置rtsp连接超时（单位 us）
        av_dict_set(&fmt_options, "rtsp_transport", "tcp", 0);
        //av_dict_set(&fmt_options, "muxdelay", "0.1", 0);
        //av_dict_set(&fmt_options, "tune", "zerolatency", 0);

        mFmtCtx->video_codec_id = mFmtCtx->oformat->video_codec;
        mFmtCtx->audio_codec_id = mFmtCtx->oformat->audio_codec;

        if (avformat_write_header(mFmtCtx, &fmt_options) < 0) { // 调用该函数会将所有stream的time_base，自动设置一个值，通常是1/90000或1/1000，这表示一秒钟表示的时间基长度
            LOGE("avformat_write_header error");
            return false;
        }

        mConnectCount++;


        return true;
    }
    bool AvPushStream::reConnect() {
        if (mConnectCount <= 100) {
            closeConnect();

            if (connect()) {
                return true;
            }
            else {
                return false;
            }

        }
        return false;

    }
    void AvPushStream::closeConnect() {
        LOGI("");

        clearVideoFrameQueue();
        clearAudioFrameQueue();

        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (mFmtCtx) {
            // 推流需要释放start
            if (mFmtCtx && !(mFmtCtx->oformat->flags & AVFMT_NOFILE)) {
                avio_close(mFmtCtx->pb);
            }
            // 推流需要释放end



            avformat_free_context(mFmtCtx);
            mFmtCtx = NULL;
        }

        if (mVideoCodecCtx) {
            if (mVideoCodecCtx->extradata) {
                av_free(mVideoCodecCtx->extradata);
                mVideoCodecCtx->extradata = NULL;
            }

            avcodec_close(mVideoCodecCtx);
            avcodec_free_context(&mVideoCodecCtx);
            mVideoCodecCtx = NULL;
            mVideoIndex = -1;
        }

        if (mAudioCodecCtx) {
            avcodec_close(mAudioCodecCtx);
            avcodec_free_context(&mAudioCodecCtx);
            mAudioCodecCtx = NULL;
            mAudioIndex = -1;
        }

    }

    void AvPushStream::initVideoFrameQueue() {
        mReusedVideoFrameQ_mtx.lock();
        VideoFrame* frame = NULL;
        int size = mControl->videoWidth * mControl->videoHeight * mControl->videoChannel;

        for (size_t i = 0; i < 5; i++)
        {
            frame = new VideoFrame(VideoFrame::BGR, size, mControl->videoWidth, mControl->videoHeight);
            mReusedVideoFrameQ.push(frame);
        }
        mReusedVideoFrameQ_mtx.unlock();

    }
    void AvPushStream::pushReusedVideoFrame(VideoFrame* frame) {
        mReusedVideoFrameQ_mtx.lock();
        mReusedVideoFrameQ.push(frame);
        mReusedVideoFrameQ_mtx.unlock();
    }

    void AvPushStream::pushVideoFrame(unsigned char* data, int size) {

        VideoFrame* frame = NULL;

        for (int i = 0; i < 3; i++)
        {
            mReusedVideoFrameQ_mtx.lock();
            if (!mReusedVideoFrameQ.empty()) {
                frame = mReusedVideoFrameQ.front();
                mReusedVideoFrameQ.pop();
                mReusedVideoFrameQ_mtx.unlock();

                break;
            }
            else {
                mReusedVideoFrameQ_mtx.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        if (frame) {
            frame->size = size;
            memcpy(frame->data, data, size);

            mVideoFrameQ_mtx.lock();
            mVideoFrameQ.push(frame);
            mVideoFrameQ_mtx.unlock();
        }
        else {
            LOGE("AvPushStream.mReusedVideoFrameQ is empty");

        }
    }
    bool AvPushStream::getVideoFrame(VideoFrame*& frame, int& frameQSize) {

        mVideoFrameQ_mtx.lock();

        if (!mVideoFrameQ.empty()) {
            frame = mVideoFrameQ.front();
            mVideoFrameQ.pop();
            frameQSize = mVideoFrameQ.size();
            mVideoFrameQ_mtx.unlock();
            return true;

        }
        else {
            frameQSize = 0;
            mVideoFrameQ_mtx.unlock();
            return false;
        }

    }
    void AvPushStream::clearVideoFrameQueue() {

        mVideoFrameQ_mtx.lock();
        while (!mVideoFrameQ.empty())
        {
            VideoFrame* frame = mVideoFrameQ.front();
            mVideoFrameQ.pop();
            delete frame;
            frame = NULL;
        }
        mVideoFrameQ_mtx.unlock();


        mReusedVideoFrameQ_mtx.lock();
        while (!mReusedVideoFrameQ.empty())
        {
            VideoFrame* frame = mReusedVideoFrameQ.front();
            mReusedVideoFrameQ.pop();
            delete frame;
            frame = NULL;
        }
        mReusedVideoFrameQ_mtx.unlock();

    }


    void AvPushStream::initAudioFrameQueue() {

        mReusedAudioFrameQ_mtx.lock();
        AudioFrame* frame = NULL;
        int size = 4096;

        for (size_t i = 0; i < 10; i++)
        {
            frame = new AudioFrame(size);
            mReusedAudioFrameQ.push(frame);
        }
        mReusedAudioFrameQ_mtx.unlock();


    }
    void AvPushStream::pushReusedAudioFrame(AudioFrame* frame) {
        mReusedAudioFrameQ_mtx.lock();
        mReusedAudioFrameQ.push(frame);
        mReusedAudioFrameQ_mtx.unlock();

    }

    void AvPushStream::pushAudioFrame(unsigned char* data, int size) {
        AudioFrame* frame = NULL;
        for (int i = 0; i < 6; i++)
        {
            mReusedAudioFrameQ_mtx.lock();
            if (!mReusedAudioFrameQ.empty()) {
                frame = mReusedAudioFrameQ.front();
                mReusedAudioFrameQ.pop();
                mReusedAudioFrameQ_mtx.unlock();
                break;
            }
            else {
                mReusedAudioFrameQ_mtx.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        if (frame) {
            frame->size = size;
            memcpy(frame->data, data, size);

            mAudioFrameQ_mtx.lock();
            mAudioFrameQ.push(frame);
            mAudioFrameQ_mtx.unlock();
        }
        else {
            LOGE("AvPushStream.mReusedAudioFrameQ is empty");
        }
    }
    bool AvPushStream::getAudioFrame(AudioFrame*& frame, int& frameQSize) {

        mAudioFrameQ_mtx.lock();

        if (!mAudioFrameQ.empty()) {
            frame = mAudioFrameQ.front();
            mAudioFrameQ.pop();
            frameQSize = mAudioFrameQ.size();
            mAudioFrameQ_mtx.unlock();
            return true;

        }
        else {
            mAudioFrameQ_mtx.unlock();
            return false;
        }

    }
    void AvPushStream::clearAudioFrameQueue() {
        mAudioFrameQ_mtx.lock();
        while (!mAudioFrameQ.empty())
        {
            AudioFrame* frame = mAudioFrameQ.front();
            mAudioFrameQ.pop();

            delete frame;
            frame = NULL;

        }
        mAudioFrameQ_mtx.unlock();

        mReusedAudioFrameQ_mtx.lock();
        while (!mReusedAudioFrameQ.empty())
        {
            AudioFrame* frame = mReusedAudioFrameQ.front();
            mReusedAudioFrameQ.pop();
            delete frame;
            frame = NULL;
        }
        mReusedAudioFrameQ_mtx.unlock();
    }


    int AvPushStream::writePkt(AVPacket* pkt) {
        mWritePkt_mtx.lock();
        int ret = av_write_frame(mFmtCtx, pkt);
        mWritePkt_mtx.unlock();

        return ret;

    }

    void AvPushStream::encodeVideoAndWriteStreamThread(void* arg) {
        ControlExecutor* executor = (ControlExecutor*)arg;
        int width = executor->mControl->videoWidth;
        int height = executor->mControl->videoHeight;

        VideoFrame* videoFrame = NULL; // 未编码的视频帧（bgr格式）
        int         videoFrameQSize = 0; // 未编码视频帧队列当前长度

        AVFrame* frame_yuv420p = av_frame_alloc();
        frame_yuv420p->format = executor->mPushStream->mVideoCodecCtx->pix_fmt;
        frame_yuv420p->width = width;
        frame_yuv420p->height = height;

        int frame_yuv420p_buff_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, width, height, 1);
        uint8_t* frame_yuv420p_buff = (uint8_t*)av_malloc(frame_yuv420p_buff_size);
        av_image_fill_arrays(frame_yuv420p->data, frame_yuv420p->linesize,
            frame_yuv420p_buff,
            AV_PIX_FMT_YUV420P,
            width, height, 1);



        AVPacket* pkt = av_packet_alloc();// 编码后的视频帧
        int64_t  encodeSuccessCount = 0;
        int64_t  frameCount = 0;

        int64_t t1 = 0;
        int64_t t2 = 0;
        int ret = -1;
        while (executor->getState())
        {
            if (executor->mPushStream->getVideoFrame(videoFrame, videoFrameQSize)) {
                //executor->mAnalyzer->show(frame_bgr);
                //executor->mAnalyzer->show(frame_yuv420p->linesize, frame_yuv420p->data);

                // frame_bgr 转  frame_yuv420p
                executor->mPushStream->bgr24ToYuv420p(videoFrame->data, width, height, frame_yuv420p_buff);
                executor->mPushStream->pushReusedVideoFrame(videoFrame);


                frame_yuv420p->pts = frame_yuv420p->pkt_dts = av_rescale_q_rnd(frameCount,
                    executor->mPushStream->mVideoCodecCtx->time_base,
                    executor->mPushStream->mVideoStream->time_base,
                    (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));

                frame_yuv420p->pkt_duration = av_rescale_q_rnd(1,
                    executor->mPushStream->mVideoCodecCtx->time_base,
                    executor->mPushStream->mVideoStream->time_base,
                    (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));

                frame_yuv420p->pkt_pos = -1;

                t1 = Analyzer_getCurTime();
                ret = avcodec_send_frame(executor->mPushStream->mVideoCodecCtx, frame_yuv420p);
                if (ret >= 0) {
                    ret = avcodec_receive_packet(executor->mPushStream->mVideoCodecCtx, pkt);
                    if (ret >= 0) {
                        t2 = Analyzer_getCurTime();
                        encodeSuccessCount++;

                        //LOGI("encode 1 frame spend：%lld(ms),frameCount=%lld, encodeSuccessCount = %lld, frameQSize=%d,ret=%d", 
                        //    (t2 - t1), frameCount, encodeSuccessCount, frameQSize, ret);


                        // 如果实际推流的是flv文件，不会执行里面的fix_packet_pts
                        if (pkt->pts == AV_NOPTS_VALUE) {
                            LOGE("pkt->pts == AV_NOPTS_VALUE");

                        }
                        pkt->stream_index = executor->mPushStream->mVideoIndex;

                        pkt->pos = -1;
                        pkt->duration = frame_yuv420p->pkt_duration;

                        ret = executor->mPushStream->writePkt(pkt);

                        if (ret < 0) {
                            LOGE("writePkt : ret=%d", ret);
                        }

                    }
                    else {
                        LOGE("avcodec_receive_packet error : ret=%d", ret);
                    }

                }
                else {
                    LOGE("avcodec_send_frame error : ret=%d", ret);
                }

                frameCount++;
            }
            else {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        //av_write_trailer(executor->mPushStream->mFmtCtx);//写文件尾

        av_packet_unref(pkt);
        pkt = NULL;


        av_free(frame_yuv420p_buff);
        frame_yuv420p_buff = NULL;

        av_frame_free(&frame_yuv420p);
        //av_frame_unref(frame_yuv420p);
        frame_yuv420p = NULL;


    }

    void AvPushStream::encodeAudioAndWriteStreamThread(void* arg) {

        ControlExecutor* executor = (ControlExecutor*)arg;

        AudioFrame* audioFrame = NULL; // 未编码的音频帧（pcm格式）
        int      audioFrameQSize = 0; // 未编码音频帧队列当前长度

         // 音频输入参数start
        uint64_t in_channel_layout = AV_CH_LAYOUT_STEREO;// 输入声道层
        int in_channels = av_get_channel_layout_nb_channels(in_channel_layout);// 输入声道数
        //in_channel_layout = av_get_default_channel_layout(in_channels);// 输入声道层
        AVSampleFormat in_sample_fmt = AV_SAMPLE_FMT_S16;
        int in_sample_rate = 44100;
        int in_nb_samples = 1024;
        // 音频输入参数end


       // 音频重采样输出参数start
        uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO;
        int out_channels = av_get_channel_layout_nb_channels(out_channel_layout);
        AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_FLTP;
        int out_sample_rate = 44100;
        int out_nb_samples = 1024;
        // 音频重采样输出参数end


        struct SwrContext* swr_ctx_audioConvert = swr_alloc();
        swr_ctx_audioConvert = swr_alloc_set_opts(swr_ctx_audioConvert,
            out_channel_layout,
            out_sample_fmt,
            out_sample_rate,
            in_channel_layout,
            in_sample_fmt,
            in_sample_rate,
            0, NULL);
        swr_init(swr_ctx_audioConvert);


        AVFrame* frame = av_frame_alloc();
        frame->nb_samples = out_nb_samples;
        frame->sample_rate = out_sample_rate;
        frame->format = out_sample_fmt;
        frame->channel_layout = out_channel_layout;
        frame->channels = out_channels;

        int frame_buff_size = av_samples_get_buffer_size(NULL, frame->channels, frame->nb_samples, out_sample_fmt, 1);
        uint8_t* frame_buff = (uint8_t*)av_malloc(frame_buff_size);
        avcodec_fill_audio_frame(frame, frame->channels, out_sample_fmt, (const uint8_t*)frame_buff, frame_buff_size, 1);

        uint8_t** convert_data = (uint8_t**)calloc(out_channels, sizeof(*convert_data));
        av_samples_alloc(convert_data, NULL, out_channels, out_nb_samples, out_sample_fmt, 0);


        AVPacket* pkt = av_packet_alloc();// 编码后的音频帧
        int64_t  encodeSuccessCount = 0;
        int64_t  frameCount = 0;

        int64_t t1 = 0;
        int64_t t2 = 0;
        int ret = -1;
        while (executor->getState())
        {
            if (executor->mPushStream->getAudioFrame(audioFrame, audioFrameQSize)) {

                memcpy(frame_buff, audioFrame->data, audioFrame->size);

                // 重采样
                swr_convert(swr_ctx_audioConvert, convert_data, executor->mPushStream->mAudioCodecCtx->frame_size,
                    (const uint8_t**)frame->data, frame->nb_samples);

                memcpy(frame->data[0], convert_data[0], audioFrame->size);
                memcpy(frame->data[1], convert_data[1], audioFrame->size);

                //https://www.dandelioncloud.cn/article/details/1441257596882898946
                executor->mPushStream->pushReusedAudioFrame(audioFrame);

                frame->pts = frame->pkt_dts = av_rescale_q_rnd(frameCount,
                    executor->mPushStream->mAudioCodecCtx->time_base,
                    executor->mPushStream->mAudioStream->time_base,
                    (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));

                frame->pkt_duration = av_rescale_q_rnd(1,
                    executor->mPushStream->mAudioCodecCtx->time_base,
                    executor->mPushStream->mAudioStream->time_base,
                    (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));

                frame->pkt_pos = -1;
                t1 = Analyzer_getCurTime();
                ret = avcodec_send_frame(executor->mPushStream->mAudioCodecCtx, frame);
                if (ret >= 0) {
                    while (avcodec_receive_packet(executor->mPushStream->mAudioCodecCtx, pkt) >= 0)
                    {
                        t2 = Analyzer_getCurTime();
                        encodeSuccessCount++;

                        //LOGI("encode 1 frame spend：%lld(ms),frameCount=%lld, encodeSuccessCount = %lld, frameQSize=%d,ret=%d",
                        //    (t2 - t1), frameCount, encodeSuccessCount, frameQSize, ret);

                        // 如果实际推流的是flv文件，不会执行里面的fix_packet_pts
                        if (pkt->pts == AV_NOPTS_VALUE) {
                            LOGE("pkt->pts == AV_NOPTS_VALUE");

                        }
                        pkt->stream_index = executor->mPushStream->mAudioIndex;
                        pkt->pos = -1;
                        pkt->duration = frame->pkt_duration;

                        ret = executor->mPushStream->writePkt(pkt);

                        if (ret < 0) {
                            LOGE("writePkt : ret=%d", ret);
                        }

                    }

                }
                else {
                    LOGE("avcodec_send_frame error : ret=%d", ret);
                }

                frameCount++;
            }
            else {
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
        }

        av_packet_unref(pkt);
        pkt = NULL;

        av_freep(&convert_data[0]);
        convert_data[0] = NULL;
        free(convert_data);
        convert_data = NULL;


        av_free(frame_buff);
        frame_buff = NULL;

        av_frame_free(&frame);
        //av_frame_unref(frame);
        frame = NULL;

        swr_free(&swr_ctx_audioConvert);
        swr_ctx_audioConvert = NULL;

    }


    unsigned char AvPushStream::clipValue(unsigned char x, unsigned char min_val, unsigned char  max_val) {

        if (x > max_val) {
            return max_val;
        }
        else if (x < min_val) {
            return min_val;
        }
        else {
            return x;
        }
    }

    bool AvPushStream::bgr24ToYuv420p(unsigned char* bgrBuf, int w, int h, unsigned char* yuvBuf) {

        unsigned char* ptrY, * ptrU, * ptrV, * ptrRGB;
        memset(yuvBuf, 0, w * h * 3 / 2);
        ptrY = yuvBuf;
        ptrU = yuvBuf + w * h;
        ptrV = ptrU + (w * h * 1 / 4);
        unsigned char y, u, v, r, g, b;

        for (int j = 0; j < h; ++j) {

            ptrRGB = bgrBuf + w * j * 3;
            for (int i = 0; i < w; i++) {

                b = *(ptrRGB++);
                g = *(ptrRGB++);
                r = *(ptrRGB++);


                y = (unsigned char)((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
                u = (unsigned char)((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
                v = (unsigned char)((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;
                *(ptrY++) = clipValue(y, 0, 255);
                if (j % 2 == 0 && i % 2 == 0) {
                    *(ptrU++) = clipValue(u, 0, 255);
                }
                else {
                    if (i % 2 == 0) {
                        *(ptrV++) = clipValue(v, 0, 255);
                    }
                }
            }
        }
        return true;

    }
}


