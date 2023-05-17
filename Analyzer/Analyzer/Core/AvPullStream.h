#ifndef ANALYZER_AVPULLSTREAM_H
#define ANALYZER_AVPULLSTREAM_H
#include <queue>
#include <mutex>
#include <condition_variable>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}
namespace AVSAnalyzer {
	class Config;
	struct Control;

	class AvPullStream
	{
	public:
		AvPullStream(Config* config, Control* control);
		~AvPullStream();

	public:
		bool connect();     // 连接流媒体服务
		bool reConnect();   // 重连流媒体服务
		void closeConnect();// 关闭流媒体服务的连接

		int mConnectCount = 0;

		AVFormatContext* mFmtCtx = NULL;
		// 视频帧
		AVCodecContext* mVideoCodecCtx = NULL;
		AVStream* mVideoStream = NULL;
		bool getVideoPkt(AVPacket& pkt, int& pktQSize);// 从队列获取的pkt，一定要主动释放!!!

		// 音频帧
		AVCodecContext* mAudioCodecCtx = nullptr;
		bool getAudioPkt(AVPacket& pkt, int& pktQSize);// 从队列获取的pkt，一定要主动释放!!!

	public:
		static void readThread(void* arg); // 拉流媒体流
	private:
		Config* mConfig;
		Control* mControl;

		bool pushVideoPkt(const AVPacket& pkt);
		void clearVideoPktQueue();
		std::queue <AVPacket>   mVideoPktQ;
		std::mutex              mVideoPktQ_mtx;

		bool pushAudioPkt(const AVPacket& pkt);
		void clearAudioPktQueue();
		std::queue <AVPacket>   mAudioPktQ;
		std::mutex              mAudioPktQ_mtx;

	};


}
#endif //ANALYZER_AVPULLSTREAM_H