#include "../include/AVSAlarmManage.h"
#include "Utils/Log.h"
#include "Utils/Common.h"
#include "GenerateVideo.h"

#define TESTMESSAGE "build AVSAlarmManageLib"
#pragma message(TESTMESSAGE)

namespace AVSAlarmManageLib {

	AVSAlarmImage* AVSAlarmImage::Create() {
		return new AVSAlarmImage;
	}
	AVSAlarmImage::AVSAlarmImage() {}
	AVSAlarmImage::~AVSAlarmImage() {
		this->freeData();
	}

	void AVSAlarmImage::initData(unsigned char* data, int size, int width, int height, int channels) {

		this->freeData();
		this->size = size;
		this->data = (unsigned char*)malloc(this->size);
		memcpy(this->data, data, this->size);
		this->width = width;
		this->height = height;
		this->channels = channels;

	}

	void AVSAlarmImage::freeData() {

		if (this->data) {
			free(this->data);
			this->data = nullptr;
		}
		this->size = 0;
		this->width = 0;
		this->height = 0;
		this->channels = 0;
	}

	unsigned char* AVSAlarmImage::getData() {
		return this->data;
	}
	int AVSAlarmImage::getSize() {
		return this->size;
	}
	int AVSAlarmImage::getWidth() {
		return this->width;
	}
	int AVSAlarmImage::getHeight() {
		return this->height;
	}
	int AVSAlarmImage::getChannels() {
		return this->channels;
	}
	AVSAlarm* AVSAlarm::Create(int height, int width, int fps, int64_t happen, const char* controlCode) {
	
		AVSAlarm* alarm = new AVSAlarm;

		alarm->height = height;
		alarm->width = width;
		alarm->fps = fps;
		alarm->happen = happen;
		alarm->controlCode = controlCode;

		return alarm;
	}
	AVSAlarm::AVSAlarm() {
		LOGI("");
	}
	AVSAlarm::~AVSAlarm() {
		LOGI("");
	}

	bool AVSAlarmManage_CompressImage(int height, int width, int channels,unsigned char* bgr, AVSAlarmImage* image) {

		return Common_CompressImage(height, width, channels, bgr, image);
	}
	bool AVSAlarmManage_HandleAlarm(AVSAlarm* alarm,
		const char* adminHost,
		const char* rootVideoDir,const char* subVideoDirFormat){

		GenerateVideo gen(alarm);
		gen.run(rootVideoDir, subVideoDirFormat);
		return true;
	}

}