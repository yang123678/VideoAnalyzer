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
		bool connect();     // ������ý�����
		bool reConnect();   // ������ý�����
		void closeConnect();// �ر���ý����������
		int mConnectCount = 0;

		AVFormatContext* mFmtCtx = nullptr;

		int writePkt(AVPacket* pkt);

		//��Ƶ֡
		AVCodecContext* mVideoCodecCtx = NULL;
		AVStream* mVideoStream = NULL;
		int mVideoIndex = -1;
		void pushVideoFrame(unsigned char* data, int size);

		//��Ƶ֡
		AVCodecContext* mAudioCodecCtx = NULL;
		AVStream* mAudioStream = NULL;
		int mAudioIndex = -1;
		void pushAudioFrame(unsigned char* data, int size);

	public:
		static void encodeVideoAndWriteStreamThread(void* arg); // ������Ƶ֡������
		static void encodeAudioAndWriteStreamThread(void* arg); // ������Ƶ֡������

	private:
		Config* mConfig;
		Control* mControl;

		//��Ƶ֡
		std::queue <VideoFrame*> mReusedVideoFrameQ;
		std::mutex               mReusedVideoFrameQ_mtx;
		std::queue <VideoFrame*> mVideoFrameQ;
		std::mutex               mVideoFrameQ_mtx;
		void initVideoFrameQueue();
		void pushReusedVideoFrame(VideoFrame* frame);
		bool getVideoFrame(VideoFrame*& frame, int& frameQSize);// ��ȡ��frame����ҪpushReusedVideoFrame
		void clearVideoFrameQueue();

		//��Ƶ֡
		std::queue <AudioFrame*> mReusedAudioFrameQ;
		std::mutex               mReusedAudioFrameQ_mtx;
		std::queue <AudioFrame*> mAudioFrameQ;
		std::mutex               mAudioFrameQ_mtx;
		void initAudioFrameQueue();
		void pushReusedAudioFrame(AudioFrame* frame);
		bool getAudioFrame(AudioFrame*& frame, int& frameQSize);// ��ȡ��frame����ҪpushReusedAudioFrame
		void clearAudioFrameQueue();


		// ������
		std::mutex             mWritePkt_mtx;

		// bgr24תyuv420p
		unsigned char clipValue(unsigned char x, unsigned char min_val, unsigned char  max_val);
		bool bgr24ToYuv420p(unsigned char* bgrBuf, int w, int h, unsigned char* yuvBuf);
	};

}
#endif //ANALYZER_AVPUSHSTREAM_H
