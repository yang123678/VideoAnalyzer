#ifndef AVSALGORITHM_ALGORITHMWITHAPI_H
#define AVSALGORITHM_ALGORITHMWITHAPI_H

#include "Algorithm.h"
#include <vector>

namespace AVSAlgorithmLib {

    struct AlgorithmDetectObject;

    class AlgorithmWithApi : public Algorithm
    {
    public:
        AlgorithmWithApi(AlgorithmConfig* config,std::string& host);
        virtual ~AlgorithmWithApi();
    public:
        bool test();
        virtual bool objectDetect(int height, int width, unsigned char* bgr, std::vector<AlgorithmDetectObject>& detects);
    private:
        std::string mHost;
    };
}
#endif //AVSALGORITHM_ALGORITHMWITHAPI_H