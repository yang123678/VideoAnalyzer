#ifndef ANALYZER_GENERATEALARM_H
#define ANALYZER_GENERATEALARM_H

#include <queue>
#include <mutex>
namespace AVSAnalyzer {
	class Config;
	struct Control;
	struct VideoFrame;
	struct AudioFrame;

	class GenerateAlarm
	{
	public:
		GenerateAlarm(Config* config, Control* control);
		~GenerateAlarm();
	public:
		void pushVideoFrame(unsigned char* data, int size, bool happen, float happenScore);
	public:
		static void generateAlarmThread(void* arg);
	private:
		Config* mConfig;
		Control* mControl;

		// ”∆µ÷°
		std::queue <VideoFrame*> mVideoFrameQ;
		std::mutex               mVideoFrameQ_mtx;
		bool getVideoFrame(VideoFrame*& frame, int& frameQSize);
		void clearVideoFrameQueue();

	};
}

#endif //ANALYZER_GENERATEALARM_H