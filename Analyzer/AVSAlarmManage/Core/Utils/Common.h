#ifndef AVSALARMMANAGE_COMMON_H
#define AVSALARMMANAGE_COMMON_H

#include <string>
#include <vector>
#include <chrono>
#include <corecrt_io.h> // windows平台 access 函数
#include "Base64.h"
#include "../../include/AVSAlarmManage.h"
#ifndef _DEBUG
#include <turbojpeg.h>
#else
#include <opencv2/opencv.hpp>
#endif

namespace AVSAlarmManageLib {

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

    static int Common_GetRandom() {
        std::string numStr;
        numStr.append(std::to_string(rand() % 9 + 1));
        numStr.append(std::to_string(rand() % 10));
        numStr.append(std::to_string(rand() % 10));
        numStr.append(std::to_string(rand() % 10));
        numStr.append(std::to_string(rand() % 10));
        numStr.append(std::to_string(rand() % 10));
        numStr.append(std::to_string(rand() % 10));
        numStr.append(std::to_string(rand() % 10));
        int num = stoi(numStr);

        return num;
    }

    static bool Common_RemoveFile(const std::string& filename) {

        if (remove(filename.data()) == 0) {
            return true;
        }
        else {
            return false;
        }
    }

    static void Common_mkdirs(const std::string& dir) {

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
    

    static bool Common_CompressImage(int height, int width, int channels, unsigned char* bgr, AVSAlarmImage* image) {

#ifndef _DEBUG
        unsigned char* jpeg_data = nullptr;
        unsigned long  jpeg_size = 0;

        __turboJpeg_compress(height, width, channels,bgr, jpeg_data, &jpeg_size);

        if (jpeg_size > 0 && jpeg_data != nullptr) {
            image->initData(jpeg_data, jpeg_size,width,height, channels);
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

        image->initData(jpeg_data.data(), jpeg_data.size(), width, height, channels);

        return true;

#endif // !_DEBUG

    }
    static bool Common_UnCompressImage(AVSAlarmImage* image, unsigned char*& out_bgr, int out_bgrSize) {
#ifndef _DEBUG

        unsigned char* jpeg_data = image->getData();
        unsigned long jpeg_size = image->getSize();
        int height = image->getHeight(); 
        int width = image->getWidth();
        int channels = image->getChannels();


        tjhandle handle = tjInitDecompress();
        if (nullptr == handle) {
            return false;
        }

        int subsamp, cs;
        int ret = tjDecompressHeader3(handle, jpeg_data, jpeg_size, &width, &height, &subsamp, &cs);
        if (cs == TJCS_GRAY) channels = 1;
        else channels = 3;

        int pf = TJCS_RGB;
        int ps = tjPixelSize[pf];

        ret = tjDecompress2(handle, jpeg_data, jpeg_size, out_bgr, width, width * channels, height, TJPF_BGR, TJFLAG_NOREALLOC);

        tjDestroy(handle);

        if (ret != 0) {
            return false;
        }
        return true;

#else
        std::vector<uchar> jpegBuffer(image->getSize());
		memcpy(jpegBuffer.data(), image->getData(), image->getSize());
		cv::Mat bgr_image = cv::imdecode(jpegBuffer, CV_LOAD_IMAGE_UNCHANGED);
        memcpy(out_bgr, bgr_image.data, out_bgrSize);

        return true;

#endif // !_DEBUG

    }

    //将bgr格式的图片写入本地文件
    static bool Common_SaveBgr(int height, int width, int channels, unsigned char* bgr, const std::string& out_filename) {

#ifndef _DEBUG
        unsigned char* imageCompressed = nullptr;
        unsigned long  imageCompressedSize = 0;

        __turboJpeg_compress(height, width, channels, bgr, imageCompressed, &imageCompressedSize);

        if (imageCompressedSize > 0 && imageCompressed != nullptr) {

            FILE* file = fopen(out_filename.data(), "wb");
            fwrite(imageCompressed, 1, imageCompressedSize, file);
            fclose(file);

            free(imageCompressed);
            imageCompressed = nullptr;
            return true;
        }
        else {
            return false;
        }
#else
        cv::Mat image(height, width, CV_8UC3, bgr);

        if (cv::imwrite(out_filename, image)) {
            return true;
        }
        else {
            return false;
        }
#endif

    }


    //将CompressImage压缩的图片写入本地文件
    static bool Common_SaveCompressImage(AVSAlarmImage* image, const std::string& out_filename){
        FILE* file = fopen(out_filename.data(), "wb");
        if (file) {
            fwrite(image->getData(), 1, image->getSize(), file);
            fclose(file);
            return true;
        }
        else {
            return false;
        }

    }

    // bgr24转yuv420p

    static unsigned char clipValue(unsigned char x, unsigned char min_val, unsigned char  max_val) {

        if (x > max_val) {
            return max_val;
        }
        else if (x < min_val) {
            return min_val;
        }
        else {
            return x;
        }
    }

    static bool bgr24ToYuv420p(unsigned char* bgrBuf, int w, int h, unsigned char* yuvBuf) {

        unsigned char* ptrY, * ptrU, * ptrV, * ptrRGB;
        memset(yuvBuf, 0, w * h * 3 / 2);
        ptrY = yuvBuf;
        ptrU = yuvBuf + w * h;
        ptrV = ptrU + (w * h * 1 / 4);
        unsigned char y, u, v, r, g, b;

        for (int j = 0; j < h; ++j) {

            ptrRGB = bgrBuf + w * j * 3;
            for (int i = 0; i < w; i++) {

                b = *(ptrRGB++);
                g = *(ptrRGB++);
                r = *(ptrRGB++);


                y = (unsigned char)((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
                u = (unsigned char)((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
                v = (unsigned char)((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;
                *(ptrY++) = clipValue(y, 0, 255);
                if (j % 2 == 0 && i % 2 == 0) {
                    *(ptrU++) = clipValue(u, 0, 255);
                }
                else {
                    if (i % 2 == 0) {
                        *(ptrV++) = clipValue(v, 0, 255);
                    }
                }
            }
        }
        return true;
    }
};

#endif //AVSALARMMANAGE_COMMON_H