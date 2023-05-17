#ifndef AVSALARMMANAGE_GENERATEVIDEO_H
#define AVSALARMMANAGE_GENERATEVIDEO_H

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}
namespace AVSAlarmManageLib {
	struct AVSAlarmImage;
	struct AVSAlarm;

	class GenerateVideo
	{
	public:
		GenerateVideo() = delete;
		GenerateVideo(AVSAlarm* alarm);
		~GenerateVideo();
	
	public:
		bool run(const char* rootVideoDir, const char* subVideoDirFormat);
	private:
		AVSAlarm* mAlarm;
		bool initCodecCtx(const char * url);
		void destoryCodecCtx();

		AVFormatContext* mFmtCtx = nullptr;
		// ”∆µ÷°
		AVCodecContext* mVideoCodecCtx = nullptr;
		AVStream* mVideoStream = nullptr;
		int mVideoIndex = -1;
	};

}
#endif //AVSALARMMANAGE_GENERATEVIDEO_H
