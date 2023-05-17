#include "AlgorithmWithPy.h"
#include <opencv2/opencv.hpp>
#include "Utils/Log.h"
#include "Utils/Common.h"
#include "../include/AVSAlgorithm.h"

#ifndef _DEBUG
#include "numpy/arrayobject.h"
#endif

namespace AVSAlgorithmLib {
    class PythonThreadLocker
    {
        PyGILState_STATE state;
    public:
        PythonThreadLocker() : state(PyGILState_Ensure()) {

        }
        ~PythonThreadLocker() {
            PyGILState_Release(state);

        }
    };

#ifndef _DEBUG
    size_t init_numpy() {
        import_array();
    }
#endif


    AlgorithmWithPy::AlgorithmWithPy(AlgorithmConfig* config) : Algorithm(config)
    {
        LOGI("");

#ifndef _DEBUG
        init_numpy();
#endif

        mModule = PyImport_ImportModule("Algorithm");// python模块名称
        //mModule = PyImport_ImportModule("Cpp");// python模块名称
        mClass = PyObject_GetAttrString(mModule, "Algorithm");// 获取python类

        // 设置类初始化的参数start

        PyObject* mClassArgs = PyTuple_New(2);

        std::string weights_path = config->algorithmPath + "/weights";
        PyObject* mClassArgs_weights_path = Py_BuildValue("s", weights_path.data());

        PyObject* mClassArgs_params = PyDict_New();
        PyDict_SetItemString(mClassArgs_params, "device", Py_BuildValue("s", config->algorithmDevice.data()));
        PyDict_SetItemString(mClassArgs_params, "threshold", Py_BuildValue("f", 0.3));
        PyDict_SetItemString(mClassArgs_params, "detect_points", Py_BuildValue("i", 11));

        PyTuple_SetItem(mClassArgs, 0, mClassArgs_weights_path);
        PyTuple_SetItem(mClassArgs, 1, mClassArgs_params);


        mObject = PyEval_CallObject(mClass, mClassArgs);// 实例化python类
#ifndef _DEBUG
        Py_CLEAR(mClassArgs_weights_path);
        Py_CLEAR(mClassArgs_params);
        Py_CLEAR(mClassArgs);
#endif


        // 设置类初始化的参数start


        mFunc_release = PyObject_GetAttrString(mObject, "release");// 映射python类的方法
        mFunc_objectDetect = PyObject_GetAttrString(mObject, "objectDetect");// 映射python类的方法
        mFunc_objectDetectArgs = PyTuple_New(2);


    }

    AlgorithmWithPy::~AlgorithmWithPy()
    {
        LOGI("");


        PyEval_CallObject(mFunc_release, NULL);

#ifndef _DEBUG
        Py_CLEAR(mFunc_release);
        Py_CLEAR(mFunc_objectDetect);
        Py_CLEAR(mFunc_objectDetectArgs);

        Py_CLEAR(mObject);
        Py_CLEAR(mClass);
        Py_CLEAR(mModule);
#endif


        mFunc_release = NULL;
        mFunc_objectDetect = NULL;
        mFunc_objectDetectArgs = NULL;
        mObject = NULL;
        mClass = NULL;
        mModule = NULL;

    }

    bool AlgorithmWithPy::test() {

        cv::Mat image = cv::imread("D:\\file\\data\\images\\1.jpg");
        //cv::imshow("image", image);
        //cv::waitKey(0);
        //cv::destroyAllWindows();

        //int height = image.rows;
        //int width = image.cols;
        //int chnl = image.channels();
        std::vector<AlgorithmDetectObject> mDetects;

        this->objectDetect(image.rows,image.cols,image.data, mDetects);

        mDetects.clear();

        return false;
    }
    bool AlgorithmWithPy::objectDetect(int height, int width, unsigned char* bgr, std::vector<AlgorithmDetectObject>& detects) {
        cv::Mat image(height, width, CV_8UC3, bgr);

        PythonThreadLocker pyLocker;

        int64_t t1 = getCurTime();

#ifndef _DEBUG
        // cv::Mat->numpy
        int r = image.rows;
        int c = image.cols;
        int chnl = image.channels();
        int nElem = r * c * chnl;
        uchar* imageData = new uchar[nElem];
        std::memcpy(imageData, image.data, nElem * sizeof(uchar));
        npy_intp mdim[] = { r, c, chnl };
        PyObject* pyImage = PyArray_SimpleNewFromData(chnl, mdim, NPY_UINT8, (void*)imageData);

        PyTuple_SetItem(mFunc_objectDetectArgs, 0, Py_BuildValue("i", 0));
        PyTuple_SetItem(mFunc_objectDetectArgs, 1, pyImage);

#else
        //cv::Mat->imageBase64

        std::string imageBase64;
        Common_CompressAndEncodeBase64(image.rows, image.cols, 3, image.data, imageBase64);

        PyTuple_SetItem(mFunc_objectDetectArgs, 0, Py_BuildValue("i", 1));
        PyTuple_SetItem(mFunc_objectDetectArgs, 1, Py_BuildValue("s", imageBase64.data()));

#endif
        int64_t t2 = getCurTime();


        int64_t t3 = getCurTime();
        PyObject* pyResponse = PyEval_CallObject(mFunc_objectDetect, mFunc_objectDetectArgs);
        //PyObject* pyResponse = PyEval_CallFunction(mFunc_objectDetect, "(i,s)",1, imageBase64.data());
        int64_t t4 = getCurTime();

        char* response_c = NULL;
        PyArg_Parse(pyResponse, "s", &response_c);

        bool result = false;
        if (NULL != response_c) {
            std::string response(response_c);
            result = parseObjectDetect(response, detects);
        }

        //LOGI("serialize spend：%lld(ms),call python spend：%lld(ms)", (t2 - t1), (t4 - t3));

#ifndef _DEBUG
        delete[]imageData;
        imageData = NULL;
        Py_CLEAR(pyResponse);
#endif

        pyResponse = NULL;

        return result;
    }

}

