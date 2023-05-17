#ifndef ANALYZER_CONFIG_H
#define ANALYZER_CONFIG_H

#include <string>
#include <vector>

namespace AVSAnalyzer {
	class Config
	{
	public:
		Config(const char* file, const char* ip, short port);
		~Config();
	public:

		bool mState = false;
		void show();
		//void getAlgorithmHost(std::string &host);
	public:
		const char* file = NULL;
		const char* serverIp = NULL;
		short serverPort = 0;
		std::string adminHost{};// 管理后台地址 http://*:9001
		std::string rootVideoDir{};
		std::string subVideoDirFormat{};
		int  controlExecutorMaxNum = 0;// 支持的分析视频最大路数
		bool supportHardwareVideoDecode = false;
		bool supportHardwareVideoEncode = false;

		std::string algorithmType{};
		std::string algorithmPath{};
		std::string algorithmDevice{};
		int algorithmInstanceNum = 1;
		std::vector<std::string> algorithmApiHosts;// 算法服务地址数组



	};
}
#endif //ANALYZER_CONFIG_H