#include "ControlExecutor.h"
#include "Utils/Log.h"
#include "Utils/Common.h"
#include "Scheduler.h"
#include "Analyzer.h"
#include "Control.h"
#include "AvPullStream.h"
#include "AvPushStream.h"
#include "GenerateAlarm.h"

extern "C" {
#include "libswscale/swscale.h"
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
}

namespace AVSAnalyzer {
    ControlExecutor::ControlExecutor(Scheduler* scheduler, Control* control) :
        mScheduler(scheduler),
        mControl(new Control(*control)),
        mPullStream(nullptr),
        mPushStream(nullptr),
        mGenerateAlarm(nullptr),
        mAnalyzer(nullptr),
        mState(false)
    {
        mControl->executorStartTimestamp = Analyzer_getCurTimestamp();

        LOGI("");
    }

    ControlExecutor::~ControlExecutor()
    {
        LOGI("");

        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        mState = false;// 将执行状态设置为false

        for (auto th : mThreads) {
            th->join();

        }
        for (auto th : mThreads) {
            delete th;
            th = nullptr;
        }
        mThreads.clear();


        if (mPullStream) {
            delete mPullStream;
            mPullStream = nullptr;
        }
        if (mPushStream) {
            delete mPushStream;
            mPushStream = nullptr;
        }

        if (mAnalyzer) {
            delete mAnalyzer;
            mAnalyzer = nullptr;
        }

        if (mGenerateAlarm) {
            delete mGenerateAlarm;
            mGenerateAlarm = nullptr;
        }

        if (mControl) {
            delete mControl;
            mControl = nullptr;
        }
    }
    bool ControlExecutor::start(std::string& msg) {

        this->mPullStream = new AvPullStream(mScheduler->getConfig(), mControl);
        if (this->mPullStream->connect()) {
            if (mControl->pushStream) {
                this->mPushStream = new AvPushStream(mScheduler->getConfig(), mControl);
                if (this->mPushStream->connect()) {
                    // success
                }
                else {
                    msg = "pull stream connect success, push stream connect error";
                    return false;
                }
            }
            else {
                // success
            }
        }
        else {
            msg = "pull stream connect error";
            return false;
        }

        this->mAnalyzer = new Analyzer(mScheduler, mControl);
        this->mGenerateAlarm = new GenerateAlarm(mScheduler->getConfig(), mControl);

        mState = true;// 将执行状态设置为true


        std::thread* th = new std::thread(AvPullStream::readThread, this);
        mThreads.push_back(th);

        th = new std::thread(ControlExecutor::decodeAndAnalyzeVideoThread, this);
        mThreads.push_back(th);

        if (mControl->audioIndex > -1) {
            th = new std::thread(ControlExecutor::decodeAndAnalyzeAudioThread, this);
            mThreads.push_back(th);
        }

        th = new std::thread(GenerateAlarm::generateAlarmThread, this);
        mThreads.push_back(th);


        if (mControl->pushStream) {
            if (mControl->videoIndex > -1) {
                th = new std::thread(AvPushStream::encodeVideoAndWriteStreamThread, this);
                mThreads.push_back(th);
            }

            if (mControl->audioIndex > -1) {
                th = new std::thread(AvPushStream::encodeAudioAndWriteStreamThread, this);
                mThreads.push_back(th);
            }
        }

        for (auto th : mThreads) {
            th->native_handle();
        }


        return true;
    }


    bool ControlExecutor::getState() {
        return mState;
    }
    void ControlExecutor::setState_remove() {
        this->mState = false;
        this->mScheduler->removeExecutor(mControl);
    }

    void ControlExecutor::decodeAndAnalyzeVideoThread(void* arg) {

        ControlExecutor* executor = (ControlExecutor*)arg;
        int width = executor->mPullStream->mVideoCodecCtx->width;
        int height = executor->mPullStream->mVideoCodecCtx->height;

        AVPacket pkt; // 未解码的视频帧
        int      pktQSize = 0; // 未解码视频帧队列当前长度

        AVFrame* frame_yuv420p = av_frame_alloc();// pkt->解码->frame
        AVFrame* frame_bgr = av_frame_alloc();

        int frame_bgr_buff_size = av_image_get_buffer_size(AV_PIX_FMT_BGR24, width, height, 1);
        uint8_t* frame_bgr_buff = (uint8_t*)av_malloc(frame_bgr_buff_size);
        av_image_fill_arrays(frame_bgr->data, frame_bgr->linesize, frame_bgr_buff, AV_PIX_FMT_BGR24, width, height, 1);

        SwsContext* sws_ctx_yuv420p2bgr = sws_getContext(width, height,
            executor->mPullStream->mVideoCodecCtx->pix_fmt,
            executor->mPullStream->mVideoCodecCtx->width,
            executor->mPullStream->mVideoCodecCtx->height,
            AV_PIX_FMT_BGR24,
            SWS_BICUBIC, nullptr, nullptr, nullptr);

        int fps = executor->mControl->videoFps;

        //算法检测参数start
        bool cur_is_check = false;// 当前帧是否进行算法检测
        int  continuity_check_count = 0;// 当前连续进行算法检测的帧数
        int  continuity_check_max_time = 3000;//连续进行算法检测，允许最长的时间。单位毫秒
        int64_t continuity_check_start = Analyzer_getCurTime();//单位毫秒
        int64_t continuity_check_end = 0;
        //算法检测参数end

        int ret = -1;
        int64_t frameCount = 0;
        while (executor->getState())
        {
            if (executor->mPullStream->getVideoPkt(pkt, pktQSize)) {

                if (executor->mControl->videoIndex > -1) {

                    ret = avcodec_send_packet(executor->mPullStream->mVideoCodecCtx, &pkt);
                    if (ret == 0) {
                        ret = avcodec_receive_frame(executor->mPullStream->mVideoCodecCtx, frame_yuv420p);

                        if (ret == 0) {
                            frameCount++;

                            // frame（yuv420p） 转 frame_bgr
                            sws_scale(sws_ctx_yuv420p2bgr,
                                frame_yuv420p->data, frame_yuv420p->linesize, 0, height,
                                frame_bgr->data, frame_bgr->linesize);

                            if (pktQSize == 0) {
                                cur_is_check = true;
                            }
                            else {
                                cur_is_check = false;
                            }

                            if (cur_is_check) {
                                continuity_check_count += 1;
                            }

                            continuity_check_end = Analyzer_getCurTime();
                            if (continuity_check_end - continuity_check_start > continuity_check_max_time) {
                                executor->mControl->checkFps = float(continuity_check_count) / (float(continuity_check_end - continuity_check_start) / 1000);
                                continuity_check_count = 0;
                                continuity_check_start = Analyzer_getCurTime();
                            }

                            float happenScore;
                            bool happen = executor->mAnalyzer->checkVideoFrame(cur_is_check, frameCount, frame_bgr->data[0], happenScore);
                            //executor->mAnalyzer->SDLShow(frame_bgr->data[0]);
                            //executor->mAnalyzer->SDLShow(frame_yuv420p->linesize, frame_yuv420p->data);


                            //LOGI("decode 1 frame frameCount=%lld,pktQSize=%d,fps=%d,check=%d,checkFps=%f",
                            //    frameCount, pktQSize, fps, check, executor->mControl->checkFps);

                            if (executor->mControl->pushStream) {
                                executor->mPushStream->pushVideoFrame(frame_bgr->data[0], frame_bgr_buff_size);
                            }
                            executor->mGenerateAlarm->pushVideoFrame(frame_bgr->data[0], frame_bgr_buff_size, happen, happenScore);
                        }
                        else {
                            LOGE("avcodec_receive_frame error : ret=%d", ret);
                        }
                    }
                    else {
                        LOGE("avcodec_send_packet error : ret=%d", ret);
                    }
                }

                // 队列获取的pkt，必须释放!!!
                //av_free_packet(&pkt);//过时
                av_packet_unref(&pkt);
            }
            else {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }


        av_frame_free(&frame_yuv420p);
        //av_frame_unref(frame_yuv420p);
        frame_yuv420p = NULL;

        av_frame_free(&frame_bgr);
        //av_frame_unref(frame_bgr);
        frame_bgr = NULL;


        av_free(frame_bgr_buff);
        frame_bgr_buff = NULL;


        sws_freeContext(sws_ctx_yuv420p2bgr);
        sws_ctx_yuv420p2bgr = NULL;

    }

    void ControlExecutor::decodeAndAnalyzeAudioThread(void* arg) {

        ControlExecutor* executor = (ControlExecutor*)arg;

        AVPacket pkt; // 未解码的音频帧
        int      pktQSize = 0; // 未解码音频帧队列当前长度
        AVFrame* frame = av_frame_alloc();// pkt->解码->frame

        // 音频输入参数start
        int in_channels = executor->mPullStream->mAudioCodecCtx->channels;// 输入声道数
        uint64_t in_channel_layout = av_get_default_channel_layout(in_channels);// 输入声道层
        AVSampleFormat in_sample_fmt = executor->mPullStream->mAudioCodecCtx->sample_fmt;
        int in_sample_rate = executor->mPullStream->mAudioCodecCtx->sample_rate;
        int in_nb_samples = executor->mPullStream->mAudioCodecCtx->frame_size;
        // 音频输入参数end

        // 音频重采样输出参数start
        uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO;
        int out_channels = av_get_channel_layout_nb_channels(out_channel_layout);
        AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;//ffmpeg对于AAC编码的采样点格式默认只支持AV_SAMPLE_FMT_FLTP，通常PCM文件或者播放器播放的音频采样点格式是 AV_SAMPLE_FMT_S16
        int out_sample_rate = 44100;//采样率
        int out_nb_samples = 1024;//每帧单个通道的采样点数
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


        int out_buff_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_samples, out_sample_fmt, 1);
        uint8_t* out_buff = (uint8_t*)av_malloc(out_buff_size);// 重采样得到的PCM

        int ret = -1;
        int64_t frameCount = 0;
        int64_t t3, t4 = 0;
        while (executor->getState())
        {
            if (executor->mPullStream->getAudioPkt(pkt, pktQSize)) {
                if (executor->mControl->audioIndex > -1) {
                    t3 = Analyzer_getCurTime();
                    ret = avcodec_send_packet(executor->mPullStream->mAudioCodecCtx, &pkt);
                    if (ret == 0) {
                        while (avcodec_receive_frame(executor->mPullStream->mAudioCodecCtx, frame) == 0) {
                            t4 = Analyzer_getCurTime();
                            frameCount++;

                            // 重采样
                            swr_convert(swr_ctx_audioConvert, &out_buff, out_buff_size, (const uint8_t**)frame->data, frame->nb_samples);
                            //LOGI("decode 1 frame frameCount=%lld,decode spend：%lld(ms),pktQSize=%d",
                            //    frameCount, (t4 - t3), pktQSize);

                            if (executor->mControl->pushStream) {
                                // 重采样的参数决定着一帧音频的数据是out_buff_size=4096
                                executor->mPushStream->pushAudioFrame(out_buff, out_buff_size);
                            }

                        }
                    }
                    else {
                        LOGE("avcodec_send_packet : ret=%d", ret);
                    }

                }
                // 队列获取的pkt，必须释放!!!
                //av_free_packet(&pkt);//过时
                av_packet_unref(&pkt);
            }
            else {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }

        }

        av_frame_free(&frame);
        //av_frame_unref(frame);
        frame = NULL;

        av_free(out_buff);
        out_buff = NULL;

        swr_free(&swr_ctx_audioConvert);
        swr_ctx_audioConvert = NULL;

    }
}