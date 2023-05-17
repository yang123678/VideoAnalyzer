#include "Scheduler.h"
#include "Config.h"
#include "Control.h"
#include "ControlExecutor.h"
#include "Utils/Log.h"

namespace AVSAnalyzer {
    Scheduler::Scheduler(Config* config) :mConfig(config), mState(false),
        mLoopAlarmThread(nullptr)
    {
        LOGI("");
    }

    Scheduler::~Scheduler()
    {
        LOGI("");

        clearAlarmQueue();
        mLoopAlarmThread->join();
        delete mLoopAlarmThread;
        mLoopAlarmThread = nullptr;
    }

    Config* Scheduler::getConfig() {
        return mConfig;
    }

    void Scheduler::loop() {

        LOGI("Loop Start");

        mLoopAlarmThread = new std::thread(Scheduler::loopAlarmThread, this);
        mLoopAlarmThread->native_handle();

        int64_t l = 0;
        while (mState)
        {
            ++l;
            handleDeleteExecutor();

        }
        LOGI("Loop End");
    }

    int Scheduler::apiControls(std::vector<Control*>& controls) {
        int len = 0;

        mExecutorMapMtx.lock();
        for (auto f = mExecutorMap.begin(); f != mExecutorMap.end(); ++f)
        {
            ++len;
            controls.push_back(f->second->mControl);

        }
        mExecutorMapMtx.unlock();

        return len;
    }
    Control* Scheduler::apiControl(std::string& code) {
        Control* control = nullptr;
        mExecutorMapMtx.lock();
        for (auto f = mExecutorMap.begin(); f != mExecutorMap.end(); ++f)
        {
            if (f->first == code) {
                control = f->second->mControl;
            }

        }
        mExecutorMapMtx.unlock();

        return control;
    }


    void Scheduler::apiControlAdd(Control* control, int& result_code, std::string& result_msg) {

        if (isAdd(control)) {
            result_msg = "the control is running";
            result_code = 1000;
            return;
        }

        if (getExecutorMapSize() >= mConfig->controlExecutorMaxNum) {
            result_msg = "the number of control exceeds the limit";
            result_code = 0;
        }
        else {
            ControlExecutor* executor = new ControlExecutor(this, control);

            if (executor->start(result_msg)) {
                if (addExecutor(control, executor)) {
                    result_msg = "add success";
                    result_code = 1000;
                }
                else {
                    delete executor;
                    executor = nullptr;
                    result_msg = "add error";
                    result_code = 0;
                }
            }
            else {
                delete executor;
                executor = nullptr;
                result_code = 0;
            }
        }

    }
    void Scheduler::apiControlCancel(Control* control, int& result_code, std::string& result_msg) {

        ControlExecutor* controlExecutor = getExecutor(control);

        if (controlExecutor) {
            if (controlExecutor->getState()) {
                result_msg = "control is running, ";
            }
            else {
                result_msg = "control is not running, ";
            }

            removeExecutor(control);

            result_msg += "remove success";
            result_code = 1000;
            return;

        }
        else {
            result_msg = "there is no such control";
            result_code = 0;
            return;
        }

    }
    void Scheduler::setState(bool state) {
        mState = state;
    }
    bool Scheduler::getState() {
        return mState;
    }

    int Scheduler::getExecutorMapSize() {
        mExecutorMapMtx.lock();
        int size = mExecutorMap.size();
        mExecutorMapMtx.unlock();

        return size;
    }
    bool Scheduler::isAdd(Control* control) {

        mExecutorMapMtx.lock();
        bool isAdd = mExecutorMap.end() != mExecutorMap.find(control->code);
        mExecutorMapMtx.unlock();

        return isAdd;
    }
    bool Scheduler::addExecutor(Control* control, ControlExecutor* controlExecutor) {
        bool add = false;

        mExecutorMapMtx.lock();
        if (mExecutorMap.size() < mConfig->controlExecutorMaxNum) {
            bool isAdd = mExecutorMap.end() != mExecutorMap.find(control->code);
            if (!isAdd) {
                mExecutorMap.insert(std::pair<std::string, ControlExecutor* >(control->code, controlExecutor));
                add = true;
            }
        }
        mExecutorMapMtx.unlock();
        return add;
    }
    bool Scheduler::removeExecutor(Control* control) {
        bool result = false;

        mExecutorMapMtx.lock();
        auto f = mExecutorMap.find(control->code);
        if (mExecutorMap.end() != f) {
            ControlExecutor* executor = f->second;
            // executor 添加到待删除队列start
            std::unique_lock <std::mutex> lck(mTobeDeletedExecutorQ_mtx);
            mTobeDeletedExecutorQ.push(executor);
            //mTobeDeletedExecutorQ_cv.notify_all();
            mTobeDeletedExecutorQ_cv.notify_one();
            // executor 添加到待删除队列end
            result = mExecutorMap.erase(control->code) != 0;
        }
        mExecutorMapMtx.unlock();
        return result;
    }
    ControlExecutor* Scheduler::getExecutor(Control* control) {
        ControlExecutor* executor = nullptr;

        mExecutorMapMtx.lock();
        auto f = mExecutorMap.find(control->code);
        if (mExecutorMap.end() != f) {
            executor = f->second;
        }
        mExecutorMapMtx.unlock();
        return executor;
    }

    void Scheduler::handleDeleteExecutor() {

        std::unique_lock <std::mutex> lck(mTobeDeletedExecutorQ_mtx);
        mTobeDeletedExecutorQ_cv.wait(lck);

        while (!mTobeDeletedExecutorQ.empty()) {
            ControlExecutor* executor = mTobeDeletedExecutorQ.front();
            mTobeDeletedExecutorQ.pop();

            LOGI("code=%s,streamUrl=%s", executor->mControl->code.data(), executor->mControl->streamUrl.data());


            delete executor;
            executor = nullptr;
        }

    }

    void Scheduler::loopAlarmThread(void* arg) {
        Scheduler* scheduler = (Scheduler*)arg;
        AVSAlarm* alarm = nullptr;
        int alarmQSize;

        bool ret = false;
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            ret = scheduler->getAlarm(alarm, alarmQSize);
            if (ret) {
                LOGI("发送（1）条报警，剩余待报警=%d,mAlarmImageInstanceCount=%d",
                    alarmQSize, scheduler->mAlarmImageInstanceCount);


                AVSAlarmManage_HandleAlarm(alarm,"","D:\\Project\\bxc\\BXC_VideoAnalyzer\\data","%Y/%m/%d-%H-%M");


                //释放Alarm的图片资源
                for (int i = 0; i < alarm->images.size(); i++)
                {
                    AVSAlarmImage* image = alarm->images[i];
                    if (image) {
                        scheduler->giveBackAlarmImage(image);
                    }

                }
                alarm->images.clear();

                delete alarm;
                alarm = nullptr;
            }
        }



    }
    void Scheduler::addAlarm(AVSAlarm* alarm) {
        mAlarmQ_mtx.lock();
        mAlarmQ.push(alarm);
        mAlarmQ_mtx.unlock();
    }


    AVSAlarmImage* Scheduler::gainAlarmImage() {

        AVSAlarmImage* image = nullptr;
        mAlarmImageQ_mtx.lock();

        if (mAlarmImageQ.empty()) {
            mAlarmImageQ_mtx.unlock();
            image = AVSAlarmImage::Create();
            mAlarmImageInstanceCount++;

        }
        else {
            image = mAlarmImageQ.front();
            mAlarmImageQ.pop();
            mAlarmImageQ_mtx.unlock();
        }

        return image;
    }
    void Scheduler::giveBackAlarmImage(AVSAlarmImage* image) {

        image->freeData();

        mAlarmImageQ_mtx.lock();
        mAlarmImageQ.push(image);
        mAlarmImageQ_mtx.unlock();
    }

    bool Scheduler::getAlarm(AVSAlarm*& alarm, int& alarmQSize) {
        mAlarmQ_mtx.lock();

        if (!mAlarmQ.empty()) {
            alarm = mAlarmQ.front();
            mAlarmQ.pop();
            alarmQSize = mAlarmQ.size();
            mAlarmQ_mtx.unlock();
            return true;

        }
        else {
            alarmQSize = 0;
            mAlarmQ_mtx.unlock();
            return false;
        }
    }
    void Scheduler::clearAlarmQueue() {}

}
