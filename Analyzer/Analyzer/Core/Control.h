#ifndef ANALYZER_CONTROL_H
#define ANALYZER_CONTROL_H

#include <string>

namespace AVSAnalyzer {
	struct Control
	{
		// ��������������
	public:
		std::string code;// ���ر��
		std::string streamUrl;
		bool        pushStream = false;
		std::string pushStreamUrl;
		std::string behaviorCode;

		int64_t alarmMinInterval = 30;// ͬһ������С�ı������ʱ�䣨��λ���룩

	public:
		// ͨ�������õĲ���
		int64_t executorStartTimestamp = 0;// ִ��������ʱ���뼶ʱ�����13λ��
		float   checkFps = 0;// �㷨����֡�ʣ�ÿ����Ĵ�����
		int     videoWidth = 0;  // ������Ƶ�������ؿ�
		int     videoHeight = 0; // ������Ƶ�������ظ�
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
