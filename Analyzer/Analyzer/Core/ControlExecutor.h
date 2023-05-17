#ifndef ANALYZER_CONTROLEXECUTOR_H
#define ANALYZER_CONTROLEXECUTOR_H
#include <thread>
#include <queue>
#include <mutex>
namespace AVSAnalyzer {
	class Scheduler;
	class AvPullStream;
	class AvPushStream;
	class GenerateAlarm;
	class Analyzer;
	struct Control;

	struct VideoFrame
	{
	public:
		enum VideoFrameType
		{
			BGR = 0,
			YUV420P,

		};
		VideoFrame(VideoFrameType type, int size, int width, int height) {
			this->type = type;
			this->size = size;
			this->width = width;
			this->height = height;
			this->data = new uint8_t[this->size];

		}
		~VideoFrame() {
			delete[] this->data;
			this->data = nullptr;
		}

		VideoFrameType type;
		int size;
		int width;
		int height;
		uint8_t* data;
		bool happen = false;// 是否发生事件
		float happenScore = 0;// 发生事件的分数


	};

	struct AudioFrame
	{
	public:
		AudioFrame(int size) {
			this->size = size;
			this->data = new uint8_t[this->size];
		}
		~AudioFrame() {
			delete[] this->data;
			this->data = NULL;
		}

		int size;
		uint8_t* data;
	};

	class ControlExecutor
	{
	public:
		explicit ControlExecutor(Scheduler* scheduler, Control* control);
		~ControlExecutor();
	public:
		static void decodeAndAnalyzeVideoThread(void* arg);// 解码视频帧和实时分析视频帧
		static void decodeAndAnalyzeAudioThread(void* arg);// 解码音频帧和实时分析音频帧

	public:
		bool start(std::string& msg);

		bool getState();
		void setState_remove();
	public:
		Control* mControl;
		Scheduler* mScheduler;
		AvPullStream* mPullStream;
		AvPushStream* mPushStream;
		GenerateAlarm* mGenerateAlarm;
		Analyzer* mAnalyzer;

	private:
		bool mState = false;
		std::vector<std::thread*> mThreads;

	};
}
#endif //ANALYZER_CONTROLEXECUTOR_H