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
		bool connect();     // ������ý�����
		bool reConnect();   // ������ý�����
		void closeConnect();// �ر���ý����������

		int mConnectCount = 0;

		AVFormatContext* mFmtCtx = NULL;
		// ��Ƶ֡
		AVCodecContext* mVideoCodecCtx = NULL;
		AVStream* mVideoStream = NULL;
		bool getVideoPkt(AVPacket& pkt, int& pktQSize);// �Ӷ��л�ȡ��pkt��һ��Ҫ�����ͷ�!!!

		// ��Ƶ֡
		AVCodecContext* mAudioCodecCtx = nullptr;
		bool getAudioPkt(AVPacket& pkt, int& pktQSize);// �Ӷ��л�ȡ��pkt��һ��Ҫ�����ͷ�!!!

	public:
		static void readThread(void* arg); // ����ý����
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