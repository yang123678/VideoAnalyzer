#include "Analyzer.h"
#include <opencv2/opencv.hpp>
#include "Utils/Log.h"
#include "Utils/Common.h"
#include "Scheduler.h"
#include "Config.h"
#include "Control.h"

namespace AVSAnalyzer {
    Analyzer::Analyzer(Scheduler* scheduler, Control* control) :
        mScheduler(scheduler),
        mControl(control)
    {
        initSDL();
    }

    Analyzer::~Analyzer()
    {
        mDetects.clear();
        destorySDL();

    }

    bool Analyzer::checkVideoFrame(bool check, int64_t frameCount, unsigned char* data, float& happenScore) {
        bool happen = false;

        cv::Mat image(mControl->videoHeight, mControl->videoWidth, CV_8UC3, data);
        //cv::Mat image = cv::imread("D:\\file\\data\\images\\1.jpg");
        //cv::imshow("image", image);
        //cv::waitKey(0);
        //cv::destroyAllWindows();

        //int target_width = 300;
        //int target_height = 200;
        //int target_left = (mWidth - target_width) / 2;
        //int target_top = (mHeight - target_height) / 2;

        //cv::rectangle(image,cv::Rect(target_left,target_top,target_width,target_height),
        //              cv::Scalar(0,255,0),3
        //        ,cv::LINE_8,0);

        if (check) {
            mDetects.clear();
            AVSAlgorithm_ObjectDetect(mControl->videoHeight, mControl->videoWidth, data, mDetects);

            //当检测到视频中有两个人的时候，认为发生了危险行为
            if (mDetects.size() == 2) {
                //LOGI("当前帧发现了危险行为");
                happen = true;
                happenScore = 0.9;
            }

        }
        int x1, y1, x2, y2;
        for (int i = 0; i < mDetects.size(); i++)
        {
            x1 = mDetects[i].x1;
            y1 = mDetects[i].y1;
            x2 = mDetects[i].x2;
            y2 = mDetects[i].y2;
            cv::rectangle(image, cv::Rect(x1, y1, (x2 - x1), (y2 - y1)), cv::Scalar(0, 255, 0), 2, cv::LINE_8, 0);

            std::string class_name = mDetects[i].class_name + "-" + std::to_string(mDetects[i].score);
            cv::putText(image, class_name, cv::Point(x1, y1 + 15), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);

        }


        std::string info = "checkFps:" + std::to_string(mControl->checkFps);
        cv::putText(image, info, cv::Point(20, 40), cv::FONT_HERSHEY_COMPLEX, mControl->videoWidth / 1000, cv::Scalar(0, 0, 255), 1, cv::LINE_AA);


#if PLAY
        SDLShow(data);
#endif

        return happen;

    }
    bool Analyzer::checkAudioFrame(bool check, int64_t frameCount, unsigned char* data, int size) {

        return false;
    }
    void Analyzer::SDLShow(unsigned char* data) {
#if PLAY
        SDL_UpdateTexture(mSDLTexture_BGR24, nullptr, data, mWidth * 3);

        mSDLRect.x = 0;
        mSDLRect.y = 0;
        mSDLRect.w = mWidth;
        mSDLRect.h = mHeight;

        SDL_RenderClear(mSDLRenderer);//清屏
        SDL_RenderCopy(mSDLRenderer, mSDLTexture_BGR24, nullptr, &mSDLRect);//复制材质到渲染器
        SDL_RenderPresent(mSDLRenderer);//渲染
#endif // PLAY

    }

    void Analyzer::SDLShow(int linesize[8], unsigned char* data[8]) {

#if PLAY
        SDL_UpdateYUVTexture(mSDLTexture_IYUV, nullptr,
            data[0], linesize[0],
            data[1], linesize[1],
            data[2], linesize[2]);

        mSDLRect.x = 0;
        mSDLRect.y = 0;
        mSDLRect.w = mWidth;
        mSDLRect.h = mHeight;

        SDL_RenderClear(mSDLRenderer);//清屏
        SDL_RenderCopy(mSDLRenderer, mSDLTexture_IYUV, nullptr, &mSDLRect);//复制材质到渲染器
        SDL_RenderPresent(mSDLRenderer);//渲染

#endif // PLAY
    }

    int Analyzer::initSDL() {

#if PLAY

        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {

            printf("无法初始化 SDL %s \n", SDL_GetError());
            return -1;
        }

        mSDLWindow = SDL_CreateWindow("Analy",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            mWidth, mHeight,
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
        if (!mSDLWindow) {
            LOGE("SDL_CreateWindow error");
            return -1;
        }

        // 创建渲染器
        mSDLRenderer = SDL_CreateRenderer(mSDLWindow, -1, SDL_RENDERER_ACCELERATED);
        if (!mSDLRenderer) {
            LOGE("SDL_CreateRenderer error");
            return -1;
        }

        mSDLTexture_IYUV = SDL_CreateTexture(mSDLRenderer,
            SDL_PIXELFORMAT_IYUV,
            SDL_TEXTUREACCESS_STREAMING,
            mWidth, mHeight
        );
        if (!mSDLTexture_IYUV) {
            LOGE("SDL_CreateTexture mSDLTexture_IYUV error");
            return -1;
        }
        mSDLTexture_BGR24 = SDL_CreateTexture(mSDLRenderer,
            SDL_PIXELFORMAT_BGR24,
            SDL_TEXTUREACCESS_STREAMING,
            mWidth, mHeight
        );
        if (!mSDLTexture_BGR24) {
            LOGE("SDL_CreateTexture mSDLTexture_BGR24 error");
            return -1;
        }


        SDL_WaitEvent(&mSDLEvent);
#endif // PLAY

        return 0;

    }

    int Analyzer::destorySDL() {
#if PLAY
        if (mSDLWindow)
            SDL_DestroyWindow(mSDLWindow);// 销毁window
        if (mSDLRenderer)
            SDL_DestroyRenderer(mSDLRenderer);// 销毁渲染器
        if (mSDLTexture_IYUV)
            SDL_DestroyTexture(mSDLTexture_IYUV);// 销毁纹理
        if (mSDLTexture_BGR24)
            SDL_DestroyTexture(mSDLTexture_BGR24);// 销毁纹理

        SDL_Quit();// 退出
#endif // PLAY
        return 0;
    }

}