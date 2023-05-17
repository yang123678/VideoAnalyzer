#ifndef AVSALARMMANAGE_AVSALARMMANAGE_H
#define AVSALARMMANAGE_AVSALARMMANAGE_H
#include <string>
#include <vector>

#ifdef AVSALARMMANAGE_EXPORTS
#define __DECLSPEC_INC __declspec(dllexport)
#else
#define __DECLSPEC_INC __declspec(dllimport)
#endif // !AVSALARMMANAGE_EXPORTS


namespace AVSAlarmManageLib {

#ifdef __cplusplus
    extern "C" {
#endif

	struct __DECLSPEC_INC AVSAlarmImage
	{
	public:
		static AVSAlarmImage* Create();
		~AVSAlarmImage();
	private:
		AVSAlarmImage();
	public:
		void initData(unsigned char* data, int size, int width, int height, int channels);
		void freeData();

		bool  happen = false;// 是否发生事件
		float happenScore = 0;// 发生事件的分数

		unsigned char* getData();
		int getSize();
		int getWidth();
		int getHeight();
		int getChannels();
	private:

		unsigned char* data = nullptr;//图片经过jpg压缩后的数据
		int size = 0;                 //图片经过jpg压缩后的数据长度
		int width = 0;                //原图宽
		int height = 0;               //原图高
		int channels = 0;             //原图通道长
	};
	struct __DECLSPEC_INC AVSAlarm
	{
	public:
		static AVSAlarm* Create(int height,int width,int fps,int64_t happen,const char* controlCode);
		~AVSAlarm();
	private:
		AVSAlarm();
	public:
		int width = 0;
		int height = 0;
		int fps = 0;
		int64_t happen = 0;
		std::string controlCode;// 布控编号

		AVSAlarmImage* headImage = nullptr;//封面图
		std::vector<AVSAlarmImage*> images;//组成报警视频的图片帧
	};
	bool __DECLSPEC_INC AVSAlarmManage_CompressImage(int height, int width, int channels, unsigned char* bgr, AVSAlarmImage* image);
	bool __DECLSPEC_INC AVSAlarmManage_HandleAlarm(AVSAlarm* alarm,
		const char* adminHost, 
		const char* rootVideoDir, 
		const char* subVideoDirFormat);

#ifdef __cplusplus
    }
#endif
}
#endif //AVSALARMMANAGE_AVSALARMMANAGE_H
