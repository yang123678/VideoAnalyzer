#include "AvPullStream.h"
#include "Config.h"
#include "Utils/Log.h"
#include "Utils/Common.h"
#include "Control.h"
#include "ControlExecutor.h"
namespace AVSAnalyzer {
    AvPullStream::AvPullStream(Config* config, Control* control) :
        mConfig(config),
        mControl(control)
    {
        LOGI("");
    }

    AvPullStream::~AvPullStream()
    {
        LOGI("");

        closeConnect();
    }

    bool AvPullStream::connect() {

        mFmtCtx = avformat_alloc_context();

        AVDictionary* fmt_options = NULL;
        av_dict_set(&fmt_options, "rtsp_transport", "tcp", 0); //设置rtsp底层网络协议 tcp or udp
        av_dict_set(&fmt_options, "stimeout", "3000000", 0);   //设置rtsp连接超时（单位 us）
        av_dict_set(&fmt_options, "rw_timeout", "3000000", 0); //设置rtmp/http-flv连接超时（单位 us）
        //av_dict_set(&fmt_options, "timeout", "3000000", 0);//设置udp/http超时（单位 us）

        int ret = avformat_open_input(&mFmtCtx, mControl->streamUrl.data(), NULL, &fmt_options);

        if (ret != 0) {
            LOGE("avformat_open_input error: url=%s ", mControl->streamUrl.data());
            return false;
        }


        if (avformat_find_stream_info(mFmtCtx, NULL) < 0) {
            LOGE("avformat_find_stream_info error");
            return false;
        }

        // video start
        mControl->videoIndex = -1;

        for (int i = 0; i < mFmtCtx->nb_streams; i++)
        {
            if (mFmtCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
            {
                mControl->videoIndex = i;
                break;
            }
        }

        //mVideoIndex = av_find_best_stream(mFmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);

        if (mControl->videoIndex > -1) {
            AVCodecParameters* videoCodecPar = mFmtCtx->streams[mControl->videoIndex]->codecpar;

            AVCodec* videoCodec = NULL;
            if (mConfig->supportHardwareVideoDecode) {
                if (AV_CODEC_ID_H264 == videoCodecPar->codec_id) {
                    if (!videoCodec) {
                        videoCodec = avcodec_find_decoder_by_name("h264_cuvid");// 英伟达独显
                        //videoCodec = avcodec_find_decoder_by_name("h264_qsv");// 酷睿核显
                        if (videoCodec) {
                            LOGI("avcodec_find_decoder_by_name = h264_cuvid");
                        }
                    }
                }
            }

            if (!videoCodec) {
                videoCodec = avcodec_find_decoder(videoCodecPar->codec_id);
                if (!videoCodec) {
                    LOGE("avcodec_find_decoder error");
                    return false;
                }
            }

            mVideoCodecCtx = avcodec_alloc_context3(videoCodec);
            if (avcodec_parameters_to_context(mVideoCodecCtx, videoCodecPar) != 0) {
                LOGE("avcodec_parameters_to_context error");
                return false;
            }
            if (avcodec_open2(mVideoCodecCtx, videoCodec, nullptr) < 0) {
                LOGE("avcodec_open2 error");
                return false;
            }
            //mVideoCodecCtx->thread_count = 1;

            mVideoStream = mFmtCtx->streams[mControl->videoIndex];
            if (0 == mVideoStream->avg_frame_rate.den) {

                LOGE("videoIndex=%d,videoStream->avg_frame_rate.den = 0", mControl->videoIndex);

                mControl->videoFps = 25;
            }
            else {
                mControl->videoFps = mVideoStream->avg_frame_rate.num / mVideoStream->avg_frame_rate.den;
            }


            mControl->videoWidth = mVideoCodecCtx->width;
            mControl->videoHeight = mVideoCodecCtx->height;
            mControl->videoChannel = 3;

        }
        else {
            LOGE("av_find_best_stream video error videoIndex=%d", mControl->videoIndex);
            return false;
        }
        // video end;


        // audio start

        mControl->audioIndex = -1;
        for (int i = 0; i < mFmtCtx->nb_streams; i++)
        {
            if (mFmtCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
            {
                mControl->audioIndex = i;
                break;
            }
        }
        //mControl->audioIndex = av_find_best_stream(mFmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);

        if (mControl->audioIndex > -1) {
            AVCodecParameters* audioCodecPar = mFmtCtx->streams[mControl->audioIndex]->codecpar;
            AVCodec* audiCodec = avcodec_find_decoder(audioCodecPar->codec_id);
            if (!audiCodec) {
                LOGE("avcodec_find_decoder error");
                return false;
            }

            mAudioCodecCtx = avcodec_alloc_context3(audiCodec);
            if (avcodec_parameters_to_context(mAudioCodecCtx, audioCodecPar) != 0) {
                LOGE("avcodec_parameters_to_context error");
                return false;
            }
            if (avcodec_open2(mAudioCodecCtx, audiCodec, nullptr) < 0) {
                LOGE("avcodec_open2 error");
                return false;
            }
            //mAudioCodecCtx->thread_count = 1;

        }
        else {
            LOGE("av_find_best_stream audio error audioIndex=%d", mControl->audioIndex);
        }
        // audio end


        if (mControl->videoIndex <= -1) {
            return false;
        }

        mConnectCount++;

        return true;

    }

    bool AvPullStream::reConnect() {

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
    void AvPullStream::closeConnect() {

        LOGI("");

        clearVideoPktQueue();
        clearAudioPktQueue();

        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (mVideoCodecCtx) {

            avcodec_close(mVideoCodecCtx);
            avcodec_free_context(&mVideoCodecCtx);
            mVideoCodecCtx = NULL;
            mControl->videoIndex = -1;
        }

        if (mAudioCodecCtx) {
            avcodec_close(mAudioCodecCtx);
            avcodec_free_context(&mAudioCodecCtx);
            mAudioCodecCtx = NULL;
            mControl->audioIndex = -1;
        }

        if (mFmtCtx) {
            // 拉流不需要释放start
            //if (mFmtCtx && !(mFmtCtx->oformat->flags & AVFMT_NOFILE)) {
            //    avio_close(mFmtCtx->pb);
            //}
            // 拉流不需要释放end
            avformat_close_input(&mFmtCtx);
            avformat_free_context(mFmtCtx);
            mFmtCtx = NULL;
        }
    }

    bool AvPullStream::pushVideoPkt(const AVPacket& pkt) {

        if (av_packet_make_refcounted((AVPacket*)&pkt) < 0) {
            return false;
        }

        mVideoPktQ_mtx.lock();
        mVideoPktQ.push(pkt);
        mVideoPktQ_mtx.unlock();

        return true;

    }
    bool AvPullStream::getVideoPkt(AVPacket& pkt, int& pktQSize) {

        mVideoPktQ_mtx.lock();

        if (!mVideoPktQ.empty()) {
            pkt = mVideoPktQ.front();
            mVideoPktQ.pop();
            pktQSize = mVideoPktQ.size();
            mVideoPktQ_mtx.unlock();
            return true;

        }
        else {
            mVideoPktQ_mtx.unlock();
            return false;
        }

    }
    void AvPullStream::clearVideoPktQueue() {
        mVideoPktQ_mtx.lock();
        while (!mVideoPktQ.empty())
        {
            AVPacket pkt = mVideoPktQ.front();
            mVideoPktQ.pop();

            av_packet_unref(&pkt);
        }
        mVideoPktQ_mtx.unlock();
    }

    bool AvPullStream::pushAudioPkt(const AVPacket& pkt) {

        if (av_packet_make_refcounted((AVPacket*)&pkt) < 0) {
            return false;
        }

        mAudioPktQ_mtx.lock();
        mAudioPktQ.push(pkt);
        mAudioPktQ_mtx.unlock();

        return true;

    }
    bool AvPullStream::getAudioPkt(AVPacket& pkt, int& pktQSize) {

        mAudioPktQ_mtx.lock();

        if (!mAudioPktQ.empty()) {
            pkt = mAudioPktQ.front();
            mAudioPktQ.pop();
            pktQSize = mAudioPktQ.size();
            mAudioPktQ_mtx.unlock();
            return true;

        }
        else {
            mAudioPktQ_mtx.unlock();
            return false;
        }

    }
    void AvPullStream::clearAudioPktQueue() {
        mAudioPktQ_mtx.lock();

        while (!mAudioPktQ.empty())
        {
            AVPacket pkt = mAudioPktQ.front();
            mAudioPktQ.pop();

            av_packet_unref(&pkt);
        }
        mAudioPktQ_mtx.unlock();

    }

    void AvPullStream::readThread(void* arg) {

        ControlExecutor* executor = (ControlExecutor*)arg;
        int continuity_error_count = 0;

        AVPacket pkt;
        while (executor->getState())
        {
            if (av_read_frame(executor->mPullStream->mFmtCtx, &pkt) >= 0) {
                continuity_error_count = 0;

                if (pkt.stream_index == executor->mControl->videoIndex) {
                    executor->mPullStream->pushVideoPkt(pkt);
                    std::this_thread::sleep_for(std::chrono::milliseconds(30));
                }
                else if (pkt.stream_index == executor->mControl->audioIndex) {
                    executor->mPullStream->pushAudioPkt(pkt);
                }
                else {
                    //av_free_packet(&pkt);//过时
                    av_packet_unref(&pkt);
                }
            }
            else {
                //av_free_packet(&pkt);//过时
                av_packet_unref(&pkt);
                continuity_error_count++;
                if (continuity_error_count > 5) {//大于5秒重启拉流连接

                    LOGE("av_read_frame error, continuity_error_count = %d (s)", continuity_error_count);

                    if (executor->mPullStream->reConnect()) {
                        continuity_error_count = 0;
                        LOGI("reConnect success : mConnectCount=%d", executor->mPullStream->mConnectCount);
                    }
                    else {
                        LOGI("reConnect error : mConnectCount=%d", executor->mPullStream->mConnectCount);
                        executor->setState_remove();
                    }
                }
                else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                }
            }
        }

    }
}
