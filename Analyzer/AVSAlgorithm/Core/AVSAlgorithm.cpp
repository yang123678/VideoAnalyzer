#include "../include/AVSAlgorithm.h"
#include "../Core/Utils/Log.h"
#include "../Core/AlgorithmPool.h"
#include "../Core/Algorithm.h"
#include <memory>

#define TESTMESSAGE "build AVSAlgorithmLib"
#pragma message(TESTMESSAGE)

namespace AVSAlgorithmLib {

    std::unique_ptr<AlgorithmPool> mAlgorithmPool;

    bool AVSAlgorithm_Init(AlgorithmConfig* config) {
        if (mAlgorithmPool.get()) {
        
            return true;
        }

        mAlgorithmPool = AlgorithmPool::Create(config);
        if (mAlgorithmPool.get()) {
            return true;
        }
        else {
            return false;
        }

    }
    bool AVSAlgorithm_Destory() {
        if (mAlgorithmPool.get()) {
            mAlgorithmPool.reset();
            return true;
        }
        else {
            return false;
        }
 
    }
    bool AVSAlgorithm_ObjectDetect(int height, int width, unsigned char* bgr, std::vector<AlgorithmDetectObject>& detects) {
        if (mAlgorithmPool.get()) {
            Algorithm* algorithm = mAlgorithmPool->gain();
            algorithm->objectDetect(height, width, bgr, detects);
            mAlgorithmPool->giveBack(algorithm);

            return true;
        }
        else {
            return false;
        }
    }

}