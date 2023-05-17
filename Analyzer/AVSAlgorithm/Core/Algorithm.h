#ifndef AVSALGORITHM_ALGORITHM_H
#define AVSALGORITHM_ALGORITHM_H

#include <string>
#include <vector>

namespace AVSAlgorithmLib {

    struct AlgorithmDetectObject;
    struct AlgorithmConfig;

    class Algorithm
    {
    public:
        Algorithm() = delete;
        Algorithm(AlgorithmConfig* config);
        virtual ~Algorithm();
    public:
        virtual bool objectDetect(int height, int width, unsigned char* bgr, std::vector<AlgorithmDetectObject>& detects) = 0;
    protected:
        AlgorithmConfig* mConfig;
        bool parseObjectDetect(std::string& response, std::vector<AlgorithmDetectObject>& detects);

    };

}

#endif //AVSALGORITHM_ALGORITHM_H