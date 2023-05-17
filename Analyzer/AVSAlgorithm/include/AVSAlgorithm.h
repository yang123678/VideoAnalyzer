#ifndef AVSALGORITHM_AVSALGORITHM_H
#define AVSALGORITHM_AVSALGORITHM_H
#include <string>
#include <vector>

#ifdef AVSALGORITHM_EXPORTS
#define __DECLSPEC_INC __declspec(dllexport)
#else
#define __DECLSPEC_INC __declspec(dllimport)
#endif // !AVSALGORITHM_EXPORTS

#define ALGORITHMTYPE_PY "py"
#define ALGORITHMTYPE_API "api"

namespace AVSAlgorithmLib {

#ifdef __cplusplus
    extern "C" {
#endif
    struct AlgorithmConfig
    {
        std::string algorithmType;
        std::string algorithmPath;
        std::string algorithmDevice;
        int algorithmInstanceNum = 1;
        std::vector<std::string> algorithmApiHosts;// 算法服务地址数组
    };

    struct AlgorithmDetectObject
    {
        int x1;
        int y1;
        int x2;
        int y2;
        float score;
        std::string class_name;
    };

    bool __DECLSPEC_INC AVSAlgorithm_Init(AlgorithmConfig* config);
    bool __DECLSPEC_INC AVSAlgorithm_Destory();
    bool __DECLSPEC_INC AVSAlgorithm_ObjectDetect(int height, int width, unsigned char* bgr, std::vector<AlgorithmDetectObject>& detects);

#ifdef __cplusplus
    }
#endif
}
#endif //AVSALGORITHM_AVSALGORITHM_H
