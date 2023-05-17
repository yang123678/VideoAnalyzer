#ifndef AVSALGORITHM_ALGORITHMWITHPY_H
#define AVSALGORITHM_ALGORITHMWITHPY_H

#include "Algorithm.h"
#include <Python.h>

namespace AVSAlgorithmLib {
    struct AlgorithmDetectObject;

    class AlgorithmWithPy : public Algorithm
    {
    public:
        AlgorithmWithPy(AlgorithmConfig* config);
        virtual ~AlgorithmWithPy();
    public:
        bool test();
        virtual bool objectDetect(int height, int width, unsigned char* bgr, std::vector<AlgorithmDetectObject>& detects);

    private:

        PyObject* mModule = nullptr;

        PyObject* mClass = nullptr;
        PyObject* mObject = nullptr;

        PyObject* mFunc_release = nullptr;
        PyObject* mFunc_objectDetect = nullptr;
        PyObject* mFunc_objectDetectArgs = nullptr;
    };

}

#endif //AVSALGORITHM_ALGORITHMWITHPY_H
