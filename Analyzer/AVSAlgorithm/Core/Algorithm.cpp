#include "Algorithm.h"
#include <json/json.h>
#include "Utils/Log.h"
#include "../include/AVSAlgorithm.h"

namespace AVSAlgorithmLib {

    Algorithm::Algorithm(AlgorithmConfig* config):mConfig(config)
    {
        LOGI("");
    }

    Algorithm::~Algorithm()
    {
        LOGI("");
    }

    bool Algorithm::parseObjectDetect(std::string& response, std::vector<AlgorithmDetectObject>& detects) {

        Json::CharReaderBuilder builder;
        const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());

        //Json::CharReaderBuilder b;
        //Json::CharReader* reader(b.newCharReader());

        Json::Value root;
        JSONCPP_STRING errs;

        bool result = false;

        if (reader->parse(response.data(), response.data() + std::strlen(response.data()),
            &root, &errs) && errs.empty()) {


            if (root["code"].isInt()) {
                int code = root["code"].asInt();
                std::string msg = root["msg"].asString();

                if (code == 1000) {
                    Json::Value root_result = root["result"];
                    int detect_num = root_result["detect_num"].asInt();
                    Json::Value detect_data = root_result["detect_data"];

                    for (auto i : detect_data) {
                        AlgorithmDetectObject object;

                        Json::Value loc = i["location"];
                        object.x1 = loc["x1"].asInt();
                        object.y1 = loc["y1"].asInt();
                        object.x2 = loc["x2"].asInt();
                        object.y2 = loc["y2"].asInt();
                        object.score = i["score"].asFloat();
                        object.class_name = i["class_name"].asString();

                        detects.push_back(object);

                    }
                    result = true;
                }
            }


        }

        root = NULL;
        //delete reader;
        //reader = NULL;

        return result;
    }
}


