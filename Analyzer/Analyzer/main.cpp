#include "Core/Config.h"
#include "Core/Scheduler.h"
#include "Core/Server.h"
#include <AVSAlgorithm.h>

using namespace AVSAnalyzer;
using namespace AVSAlgorithmLib;

int main(int argc, char** argv)
{
#ifdef WIN32
	srand(time(NULL));//ʱ���ʼ��
#endif // WIN32

	const char* file = NULL;
	const char* ip = "0.0.0.0";
	short port = 9002;

	for (int i = 1; i < argc; i += 2)
	{
		if (argv[i][0] != '-')
		{
			printf("parameter error:%s\n", argv[i]);
			return -1;
		}
		switch (argv[i][1])
		{
			case 'h': {
				//��ӡhelp��Ϣ
				printf("-h ��ӡ����������Ϣ���˳�\n");
				printf("-f �����ļ�    �磺-f conf.json \n");
				printf("-i api����IP   �磺-i 0.0.0.0 \n");
				printf("-p api����˿� �磺-p 9002 \n");
				system("pause\n"); 
				exit(0); 
				return -1;
			}
			case 'f': {
				file = argv[i + 1];
				break;
			}
			case 'i': {
				ip = argv[i + 1];
				break;
			}
			case 'p': {
				port = atof(argv[i + 1]);
				break;
			}
			default: {
				printf("set parameter error:%s\n", argv[i]);
				return -1;

			}
		}
	}


	Config config(file,ip,port);
	if (!config.mState) {
		printf("failed to read config file: %s\n", file);
		return -1;
	}

	config.show();


	AlgorithmConfig algorithmConfig;
	algorithmConfig.algorithmType = config.algorithmType;
	algorithmConfig.algorithmPath = config.algorithmPath;
	algorithmConfig.algorithmDevice = config.algorithmDevice;
	algorithmConfig.algorithmInstanceNum = config.algorithmInstanceNum;
	algorithmConfig.algorithmApiHosts = config.algorithmApiHosts;

	AVSAlgorithm_Init(&algorithmConfig);

	Scheduler scheduler(&config);
	Server server;
	server.start(&scheduler);
	scheduler.loop();

	AVSAlgorithm_Destory();
	return 0;
}