#include "AlgorithmWithApi.h"
#include <json/json.h>
#include <opencv2/opencv.hpp>
#include "Utils/Log.h"
#include "Utils/Common.h"
#include "Utils/Request.h"
#include "../include/AVSAlgorithm.h"

namespace AVSAlgorithmLib {

    AlgorithmWithApi::AlgorithmWithApi(AlgorithmConfig* config, std::string& host) : Algorithm(config),
        mHost(host)
    {
        LOGI("");
    }

    AlgorithmWithApi::~AlgorithmWithApi()
    {
        LOGI("");
    }

    bool AlgorithmWithApi::test() {
        std::string response;
        const char* url = "http://192.168.1.3:9003";

        Request request;
        bool ret = request.get(url, response);

        //LOGI("ret=%d,response=%s",ret,response.data());
        return ret;

    }

    bool AlgorithmWithApi::objectDetect(int height, int width, unsigned char* bgr, std::vector<AlgorithmDetectObject>& detects) {
        cv::Mat image(height, width, CV_8UC3, bgr);

        int64_t t1 = getCurTime();

        std::string imageBase64;
        Common_CompressAndEncodeBase64(image.rows, image.cols, 3, image.data, imageBase64);
        int64_t t2 = getCurTime();

        std::string response;
        std::string url = mHost + "/image/objectDetect";

        Json::Value param;
        param["appKey"] = "s84dsd#7hf34r3jsk@fs$d#$dd";
        param["algorithm"] = "openvino_yolov5";
        param["image_base64"] = imageBase64;
        std::string data = param.toStyledString();
        param = NULL;

        int64_t t3 = getCurTime();
        Request request;
        bool result = request.post(url.data(), data.data(), response);
        int64_t t4 = getCurTime();

        if (result) {
            result = this->parseObjectDetect(response, detects);
        }

        //LOGI("serialize spend: %lld(ms),call api spend: %lld(ms)", (t2 - t1), (t4 - t3));

        return result;
    }
}