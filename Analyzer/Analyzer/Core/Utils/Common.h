#ifndef ANALYZER_COMMON_H
#define ANALYZER_COMMON_H

#include <string>
#include <vector>
#include <chrono>

#include <corecrt_io.h> // windows平台 access 函数

namespace AVSAnalyzer {
    static int64_t Analyzer_getCurTime()// 获取当前系统启动以来的毫秒数
    {
#ifndef WIN32
        // Linux系统
        struct timespec now;// tv_sec (s) tv_nsec (ns-纳秒)
        clock_gettime(CLOCK_MONOTONIC, &now);
        return (now.tv_sec * 1000 + now.tv_nsec / 1000000);
#else
        long long now = std::chrono::steady_clock::now().time_since_epoch().count();
        return now / 1000000;
#endif // !WIN32

    }
    static int64_t Analyzer_getCurTimestamp()// 获取毫秒级时间戳（13位）
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).
            count();

    }

    static std::vector<std::string> Analyzer_split(const std::string& str, const std::string& sep) {

        std::vector<std::string> arr;
        int sepSize = sep.size();

        int lastPosition = 0, index = -1;
        while (-1 != (index = str.find(sep, lastPosition)))
        {
            arr.push_back(str.substr(lastPosition, index - lastPosition));
            lastPosition = index + sepSize;
        }
        std::string lastStr = str.substr(lastPosition);//截取最后一个分隔符后的内容

        if (!lastStr.empty()) {
            arr.push_back(lastStr);//如果最后一个分隔符后还有内容就入队
        }

        return arr;
    }


    static bool Analyzer_removeFile(const std::string& filename) {

        if (remove(filename.data()) == 0) {
            return true;
        }
        else {
            return false;
        }
    }

    static void Analyzer_mkdirs(const std::string& dir) {

        if (::_access(dir.data(), 0) != 0) {// 文件夹不存在

            std::string command;
            command = "mkdir -p " + dir;
            system(command.c_str());

        }

    }


};

#endif //ANALYZER_COMMON_H