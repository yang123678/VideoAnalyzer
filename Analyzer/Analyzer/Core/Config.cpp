#include "Config.h"
#include <fstream>
#include <json/json.h>
#include "Utils/Log.h"
#include "Utils/Version.h"

namespace AVSAnalyzer {
    Config::Config(const char* file, const char* ip, short port) :
        file(file), serverIp(ip), serverPort(port)
    {
        std::ifstream ifs(file, std::ios::binary);

        if (!ifs.is_open()) {
            LOGE("open %s error", file);
            return;
        }
        else {
            Json::CharReaderBuilder builder;
            builder["collectComments"] = true;
            JSONCPP_STRING errs;
            Json::Value root;

            if (parseFromStream(builder, ifs, &root, &errs)) {
                this->adminHost = root["adminHost"].asString();
                this->rootVideoDir = root["rootVideoDir"].asString();
                this->subVideoDirFormat = root["subVideoDirFormat"].asString();
                this->controlExecutorMaxNum = root["controlExecutorMaxNum"].asInt();
                this->supportHardwareVideoDecode = root["supportHardwareVideoDecode"].asBool();
                this->supportHardwareVideoEncode = root["supportHardwareVideoEncode"].asBool();

                this->algorithmType = root["algorithmType"].asString();

                this->algorithmPath = root["algorithmPath"].asString();
                this->algorithmDevice = root["algorithmDevice"].asString();
                this->algorithmInstanceNum = root["algorithmInstanceNum"].asInt();

                Json::Value algorithmApiHosts = root["algorithmApiHosts"];
                for (auto& item : algorithmApiHosts) {
                    this->algorithmApiHosts.push_back(item.asString());
                }
                mState = true;
            }
            else {
                LOGE("parse %s error", file);
            }
            ifs.close();
        }
    }

    Config::~Config()
    {

    }

    void Config::show() {
        printf("--------%s-------- \n", PROJECT_VERSION);

        printf("config.file=%s\n", file);
        printf("config.serverIp=%s\n", serverIp);
        printf("config.serverPort=%d\n", serverPort);
        printf("config.adminHost=%s\n", adminHost.data());
        printf("config.rootVideoDir=%s\n", rootVideoDir.data());
        printf("config.subVideoDirFormat=%s\n", subVideoDirFormat.data());
        printf("config.controlExecutorMaxNum=%d\n", controlExecutorMaxNum);
        printf("config.supportHardwareVideoDecode=%d\n", supportHardwareVideoDecode);
        printf("config.supportHardwareVideoEncode=%d\n", supportHardwareVideoEncode);
        printf("config.algorithmType=%s\n", algorithmType.data());
        printf("config.algorithmPath=%s\n", algorithmPath.data());
        printf("config.algorithmDevice=%s\n", algorithmDevice.data());
        printf("config.algorithmInstanceNum=%d\n", algorithmInstanceNum);
        for (int i = 0; i < algorithmApiHosts.size(); i++)
        {
            printf("config.algorithmApiHosts[%d]=%s\n", i, algorithmApiHosts[i].data());
        }
        printf("--------end \n");
    }
    //void Config::getAlgorithmHost(std::string& host) {
    //
    //    int randIndex = rand() % algorithmApiHosts.size();
    //    host = algorithmApiHosts[randIndex];
    //
    //}
}