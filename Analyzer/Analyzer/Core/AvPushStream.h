#ifndef ANALYZER_AVPUSHSTREAM_H
#define ANALYZER_AVPUSHSTREAM_H
#include <queue>
#include <mutex>
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}
namespace AVSAnalyzer {
	class Config;
	struct Control;
	struct VideoFrame;
	struct AudioFrame;

	class AvPushStream
	{
	public:
		AvPushStream(Config* config, Control* control);
		~AvPushStream();
	public:
		bool connect();     // 连接流媒体服务
		bool reConnect();   // 重连流媒体服务
		void closeConnect();// 关闭流媒体服务的连接
		int mConnectCount = 0;

		AVFormatContext* mFmtCtx = nullptr;

		int writePkt(AVPacket* pkt);

		//视频帧
		AVCodecContext* mVideoCodecCtx = NULL;
		AVStream* mVideoStream = NULL;
		int mVideoIndex = -1;
		void pushVideoFrame(unsigned char* data, int size);

		//音频帧
		AVCodecContext* mAudioCodecCtx = NULL;
		AVStream* mAudioStream = NULL;
		int mAudioIndex = -1;
		void pushAudioFrame(unsigned char* data, int size);

	public:
		static void encodeVideoAndWriteStreamThread(void* arg); // 编码视频帧并推流
		static void encodeAudioAndWriteStreamThread(void* arg); // 编码音频帧并推流

	private:
		Config* mConfig;
		Control* mControl;

		//视频帧
		std::queue <VideoFrame*> mReusedVideoFrameQ;
		std::mutex               mReusedVideoFrameQ_mtx;
		std::queue <VideoFrame*> mVideoFrameQ;
		std::mutex               mVideoFrameQ_mtx;
		void initVideoFrameQueue();
		void pushReusedVideoFrame(VideoFrame* frame);
		bool getVideoFrame(VideoFrame*& frame, int& frameQSize);// 获取的frame，需要pushReusedVideoFrame
		void clearVideoFrameQueue();

		//音频帧
		std::queue <AudioFrame*> mReusedAudioFrameQ;
		std::mutex               mReusedAudioFrameQ_mtx;
		std::queue <AudioFrame*> mAudioFrameQ;
		std::mutex               mAudioFrameQ_mtx;
		void initAudioFrameQueue();
		void pushReusedAudioFrame(AudioFrame* frame);
		bool getAudioFrame(AudioFrame*& frame, int& frameQSize);// 获取的frame，需要pushReusedAudioFrame
		void clearAudioFrameQueue();


		// 推流锁
		std::mutex             mWritePkt_mtx;

		// bgr24转yuv420p
		unsigned char clipValue(unsigned char x, unsigned char min_val, unsigned char  max_val);
		bool bgr24ToYuv420p(unsigned char* bgrBuf, int w, int h, unsigned char* yuvBuf);
	};

}
#endif //ANALYZER_AVPUSHSTREAM_H
