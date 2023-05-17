#ifndef ANALYZER_CONTROL_H
#define ANALYZER_CONTROL_H

#include <string>

namespace AVSAnalyzer {
	struct Control
	{
		// 布控请求必需参数
	public:
		std::string code;// 布控编号
		std::string streamUrl;
		bool        pushStream = false;
		std::string pushStreamUrl;
		std::string behaviorCode;

		int64_t alarmMinInterval = 30;// 同一布控最小的报警间隔时间（单位毫秒）

	public:
		// 通过计算获得的参数
		int64_t executorStartTimestamp = 0;// 执行器启动时毫秒级时间戳（13位）
		float   checkFps = 0;// 算法检测的帧率（每秒检测的次数）
		int     videoWidth = 0;  // 布控视频流的像素宽
		int     videoHeight = 0; // 布控视频流的像素高
		int     videoChannel = 0;
		int     videoIndex = -1;
		int     videoFps = 0;
		int     audioIndex = -1;
		int     audioFps = 0;

	public:

		bool validateAdd(std::string& result_msg) {
			if (code.empty() || streamUrl.empty() || behaviorCode.empty()) {
				result_msg = "validate parameter error";
				return false;
			}
			if (pushStream) {
				if (pushStreamUrl.empty()) {
					result_msg = "validate parameter pushStreamUrl is error: " + pushStreamUrl;
					return false;
				}

			}
			result_msg = "validate success";
			return true;
		}
		bool validateCancel(std::string& result_msg) {

			if (code.empty()) {
				result_msg = "validate parameter error";
				return false;
			}
			result_msg = "validate success";
			return true;
		}


	};
}
#endif //ANALYZER_CONTROL_H
