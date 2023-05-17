#include "GenerateAlarm.h"
#include "Config.h"
#include "Utils/Log.h"
#include "Utils/Common.h"
#include "Control.h"
#include "ControlExecutor.h"
#include "Scheduler.h"
#include <AVSAlarmManage.h>
using namespace AVSAlarmManageLib;
namespace AVSAnalyzer {
    GenerateAlarm::GenerateAlarm(Config* config, Control* control) :
        mConfig(config),
        mControl(control)
    {

    }

    GenerateAlarm::~GenerateAlarm()
    {
        clearVideoFrameQueue();
    }


    void GenerateAlarm::pushVideoFrame(unsigned char* data, int size, bool happen, float happenScore) {

        VideoFrame* frame = new VideoFrame(VideoFrame::BGR, size, mControl->videoWidth, mControl->videoHeight);
        frame->size = size;
        memcpy(frame->data, data, size);
        frame->happen = happen;
        frame->happenScore = happenScore;

        mVideoFrameQ_mtx.lock();
        mVideoFrameQ.push(frame);
        mVideoFrameQ_mtx.unlock();

    }
    bool GenerateAlarm::getVideoFrame(VideoFrame*& frame, int& frameQSize) {

        mVideoFrameQ_mtx.lock();

        if (!mVideoFrameQ.empty()) {
            frame = mVideoFrameQ.front();
            mVideoFrameQ.pop();
            frameQSize = mVideoFrameQ.size();
            mVideoFrameQ_mtx.unlock();
            return true;

        }
        else {
            mVideoFrameQ_mtx.unlock();
            return false;
        }

    }
    void GenerateAlarm::clearVideoFrameQueue() {

        mVideoFrameQ_mtx.lock();
        while (!mVideoFrameQ.empty())
        {
            VideoFrame* frame = mVideoFrameQ.front();
            mVideoFrameQ.pop();
            delete frame;
            frame = nullptr;
        }
        mVideoFrameQ_mtx.unlock();

    }



    void GenerateAlarm::generateAlarmThread(void* arg) {
        ControlExecutor* executor = (ControlExecutor*)arg;
        int width = executor->mControl->videoWidth;
        int height = executor->mControl->videoHeight;
        int channels = 3;

        VideoFrame* videoFrame = nullptr; // δ�������Ƶ֡��bgr��ʽ��
        int         videoFrameQSize = 0; // δ������Ƶ֡���е�ǰ����

        std::vector<AVSAlarmImage* > cacheV;
        int cacheV_max_size = 250; //150 = 25 * 6����໺���¼�����ǰ6������ݣ�1��ѹ��ͼƬ100kb
        int cacheV_min_size = 50;  // 50 = 25 * 2, ���ٻ����¼�����ǰ2�������

        bool happening = false;// ��ǰ�Ƿ����ڷ���������Ϊ
        std::vector<AVSAlarmImage* > happenV;
        int     happenV_alarm_max_size = 500;
        int64_t last_alarm_timestamp = 0;// ��һ�α�����ʱ���

        int64_t t1, t2 = 0;

        while (executor->getState())
        {
            if (executor->mGenerateAlarm->getVideoFrame(videoFrame, videoFrameQSize)) {

                t1 = Analyzer_getCurTime();

                AVSAlarmImage* image = executor->mScheduler->gainAlarmImage();

                bool comp = AVSAlarmManage_CompressImage(height, width, channels, videoFrame->data, image);

                if (comp) {
                    image->happen = videoFrame->happen;
                    image->happenScore = videoFrame->happenScore;
                }

                t2 = Analyzer_getCurTime();

                if (happening) {// �����¼��Ѿ����������ڽ�����

                    if (comp) {
                        happenV.push_back(image);
                        //LOGI("h=%d,w=%d,compressSize=%lu,compress spend: %lld(ms),happenV.size=%lld",
                        //    height, width, compressImage->size, (t2 - t1), happenV.size());
                    }
                    else {
                        executor->mScheduler->giveBackAlarmImage(image);
                    }

                    if (happenV.size() >= happenV_alarm_max_size) {
                        last_alarm_timestamp = Analyzer_getCurTimestamp();

                        AVSAlarm* alarm = AVSAlarm::Create(
                            height,
                            width,
                            executor->mControl->videoFps,
                            last_alarm_timestamp,
                            executor->mControl->code.data()
                        );

                        for (size_t i = 0; i < happenV.size(); i++)
                        {
                            alarm->images.push_back(happenV[i]);
                        }
                        happenV.clear();

                        executor->mScheduler->addAlarm(alarm);

                        happening = false;
                    }
                    delete videoFrame;
                    videoFrame = nullptr;

                }
                else {// ��δ���������¼�

                    if (comp) {

                        cacheV.push_back(image);

                        //LOGI("cache h=%d,w=%d,compressSize=%d,compress spend: %lld(ms),cacheQ.size=%lld",height, width, compressImage.getSize(), (t2 - t1), cacheV.size());


                        if (!cacheV.empty() && cacheV.size() > cacheV_max_size) {
                            //���㻺�����֡
                            auto b = cacheV.begin();
                            cacheV.erase(b);
                            AVSAlarmImage* headImage = *b;
                            executor->mScheduler->giveBackAlarmImage(headImage);
                        }


                        if (videoFrame->happen && cacheV.size() > cacheV_min_size &&
                            (Analyzer_getCurTimestamp() - last_alarm_timestamp) > executor->mControl->alarmMinInterval) {
                            //���㱨������֡
                            happening = true;
                            happenV = cacheV;
                            cacheV.clear();
                        }

                    }
                    else {
                        executor->mScheduler->giveBackAlarmImage(image);
                    }
                    delete videoFrame;
                    videoFrame = nullptr;
                }

            }
            else {
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }

        }


    }
}
