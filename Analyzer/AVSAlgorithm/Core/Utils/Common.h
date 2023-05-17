#ifndef AVSALGORITHM_COMMON_H
#define AVSALGORITHM_COMMON_H

#include <string>
#include <vector>
#include <chrono>
#include <corecrt_io.h> // windows平台 access 函数
#include "Base64.h"
#ifndef _DEBUG
#include <turbojpeg.h>
#else
#include <opencv2/opencv.hpp>
#endif

namespace AVSAlgorithmLib {

    static int64_t getCurTime()// 获取当前系统启动以来的毫秒数
    {

//#ifndef WIN32
//        // Linux系统
//        struct timespec now;// tv_sec (s) tv_nsec (ns-纳秒)
//        clock_gettime(CLOCK_MONOTONIC, &now);
//        return (now.tv_sec * 1000 + now.tv_nsec / 1000000);
//#else
//        //Win系统
//        long long now = std::chrono::steady_clock::now().time_since_epoch().count();
//        return now / 1000000;
//#endif // !WIN32

        long long now = std::chrono::steady_clock::now().time_since_epoch().count();
        return now / 1000000;

    }
    static int64_t getCurTimestamp()// 获取毫秒级时间戳（13位）
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).
            count();

    }


    static bool removeFile(const std::string& filename) {

        if (remove(filename.data()) == 0) {
            return true;
        }
        else {
            return false;
        }
    }

    static void mkdirs(const std::string& dir) {

        if (::_access(dir.data(), 0) != 0) {// 文件夹不存在

            std::string command;
            command = "mkdir -p " + dir;
            system(command.c_str());

        }

    }
    static bool __turboJpeg_compress(int height, int width, int channels, unsigned char* bgr, unsigned char*& out_data, unsigned long* out_size) {

#ifndef _DEBUG

        tjhandle handle = tjInitCompress();
        if (nullptr == handle) {
            return false;
        }

        //pixel_format : TJPF::TJPF_BGR or other
        const int JPEG_QUALITY = 75;
        int pixel_format = TJPF::TJPF_BGR;
        int pitch = tjPixelSize[pixel_format] * width;
        int ret = tjCompress2(handle, bgr, width, pitch, height, pixel_format,
            &out_data, out_size, TJSAMP_444, JPEG_QUALITY, TJFLAG_FASTDCT);

        tjDestroy(handle);

        if (ret != 0) {
            return false;
        }
        return true;
#else
        return false;
#endif // !_DEBUG

    }


    static bool Common_CompressAndEncodeBase64(int height, int width,int channels, unsigned char* bgr, std::string& out_base64) {

#ifndef _DEBUG
        unsigned char* jpeg_data = nullptr;
        unsigned long  jpeg_size = 0;

        __turboJpeg_compress(height, width, channels, bgr, jpeg_data, &jpeg_size);

        if (jpeg_size > 0 && jpeg_data != nullptr) {

            Base64Encode(jpeg_data, jpeg_size, out_base64);

            free(jpeg_data);
            jpeg_data = nullptr;

            return true;
        }
        else {
            return false;
        }
#else 

        cv::Mat bgr_image(height, width, CV_8UC3, bgr);

        std::vector<int> quality = { 100 };
        std::vector<uchar> jpeg_data;
        cv::imencode(".jpg", bgr_image, jpeg_data, quality);

        Base64Encode(jpeg_data.data(), jpeg_data.size(), out_base64);

        return true;

#endif // !_DEBUG

    }


};

#endif //AVSALGORITHM_COMMON_H