#ifndef ANALYZER_SCHEDULER_H
#define ANALYZER_SCHEDULER_H
#include <map>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <thread>
#include <AVSAlarmManage.h>
using namespace AVSAlarmManageLib;
namespace AVSAnalyzer {
	class Config;
	class ControlExecutor;
	struct Control;


	class Scheduler
	{
	public:
		friend class ControlExecutor;

		Scheduler(Config* config);
		~Scheduler();
	public:
		Config* getConfig();
		void loop();

		void setState(bool state);
		bool getState();

		void addAlarm(AVSAlarm* alarm);

		AVSAlarmImage* gainAlarmImage();// �Ӷ��гػ�ȡһ��ѹ��ͼƬ��ʵ��
		void giveBackAlarmImage(AVSAlarmImage* image);//��һ��ѹ��ͼƬ��ʵ���黹�����г�
		int mAlarmImageInstanceCount = 0;

		// ApiServer ��Ӧ�ĺ��� start
		int  apiControls(std::vector<Control*>& controls);
		Control* apiControl(std::string& code);
		void apiControlAdd(Control* control, int& result_code, std::string& result_msg);
		void apiControlCancel(Control* control, int& result_code, std::string& result_msg);
		// ApiServer ��Ӧ�ĺ��� end

	private:
		Config* mConfig;

		bool  mState;

		std::map<std::string, ControlExecutor*> mExecutorMap; // <control.code,ControlExecutor*>
		std::mutex                              mExecutorMapMtx;
		int  getExecutorMapSize();
		bool isAdd(Control* control);
		bool addExecutor(Control* control, ControlExecutor* controlExecutor);
		bool removeExecutor(Control* control);//���뵽��ʵ��ɾ������
		ControlExecutor* getExecutor(Control* control);

		std::queue<ControlExecutor*> mTobeDeletedExecutorQ;
		std::mutex                   mTobeDeletedExecutorQ_mtx;
		std::condition_variable      mTobeDeletedExecutorQ_cv;
		void handleDeleteExecutor();

		//�������� start
		std::thread* mLoopAlarmThread;
		static void loopAlarmThread(void* arg);
		std::queue<AVSAlarm*> mAlarmQ;
		std::mutex            mAlarmQ_mtx;
		bool getAlarm(AVSAlarm*& alarm, int& alarmQSize);
		void clearAlarmQueue();

		std::queue<AVSAlarmImage* > mAlarmImageQ;
		std::mutex                  mAlarmImageQ_mtx;

		//�������� end

	};
}
#endif //ANALYZER_SCHEDULER_H