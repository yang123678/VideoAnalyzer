#include "AlgorithmPool.h"
#include "Algorithm.h"
#include "AlgorithmWithPy.h"
#include "AlgorithmWithApi.h"
#include "Utils/Log.h"
#include "../include/AVSAlgorithm.h"
#include <memory>

namespace AVSAlgorithmLib {

    std::unique_ptr<AlgorithmPool> AlgorithmPool::Create(AlgorithmConfig* config) {

        std::unique_ptr<AlgorithmPool> algorithmPool(new AlgorithmPool(config));

        if (algorithmPool->mInitState) {
            return algorithmPool;
        }
        else {
            return nullptr;
        }
     
    }

    AlgorithmPool::AlgorithmPool(AlgorithmConfig* config) : mConfig(config)
    {
        LOGI("");

        if (ALGORITHMTYPE_PY == mConfig->algorithmType) {
            //-------------------------初始化Py环境start--------------------------
            //Py_SetPythonHome(L"../../../Algorithm/Python");//指定PythonHome
            //Py_SetPythonHome(L"F:\\Project\\AnalyzeVideo\\Algorithm\\Python");//指定PythonHome
            Py_Initialize();
            PyEval_InitThreads();// 启用多线程支持
            PyRun_SimpleString("import sys");
            //PyRun_SimpleString("for p in sys.path:sys.path.remove(p)");
            std::string venv = "sys.path.append('" + config->algorithmPath + "')";
            PyRun_SimpleString(venv.data());
            //-------------------------初始化Py环境end--------------------------


            mAlgorithmQ_mtx.lock();
            for (size_t i = 0; i < mConfig->algorithmInstanceNum; i++)
            {
                Algorithm* algorithm = new AlgorithmWithPy(config);
                mAlgorithmQ.push(algorithm);
            }
            mAlgorithmQ_mtx.unlock();

            // 开启多线程的区域
            if (Py_IsInitialized()) {
                _save = PyEval_SaveThread();
            }

            mInitState = true;

        }
        else if (ALGORITHMTYPE_API == mConfig->algorithmType)
        {
            mAlgorithmQ_mtx.lock();
            int s = mConfig->algorithmApiHosts.size();
            for (size_t i = 0; i < mConfig->algorithmInstanceNum; i++)
            {
                int randIndex = i % s;
                std::string host = mConfig->algorithmApiHosts[randIndex];
                Algorithm* algorithm = new AlgorithmWithApi(config,host);
                mAlgorithmQ.push(algorithm);
            }
            mAlgorithmQ_mtx.unlock();

            mInitState = true;
        }
        else
        {
            mInitState = false;
        }


    }

    AlgorithmPool::~AlgorithmPool()
    {
        LOGI("");
        mAlgorithmQ_mtx.lock();
        while (!mAlgorithmQ.empty())
        {
            Algorithm* algorithm = mAlgorithmQ.front();
            mAlgorithmQ.pop();

            delete algorithm;
            algorithm = nullptr;
        }
        mAlgorithmQ_mtx.unlock();

        if (ALGORITHMTYPE_PY == mConfig->algorithmType) {
            //-------------------------关闭Py环境start--------------------------
            if (Py_IsInitialized()) {
                LOGI("Py_IsInitialized() True");
            }
            if (_save) {
                PyEval_RestoreThread(_save);
            }

            Py_Finalize();
            //-------------------------关闭Py环境end--------------------------
        }
    }


    Algorithm* AlgorithmPool::gain() {
        std::unique_lock <std::mutex> lck(mAlgorithmQ_mtx);
        Algorithm* algorithm = nullptr;

        if (!mAlgorithmQ.empty()) {
            algorithm = mAlgorithmQ.front();
            mAlgorithmQ.pop();
        }
        else {
            while (true)
            {
                mAlgorithmQ_cv.wait(lck);
                if (!mAlgorithmQ.empty()) {
                    algorithm = mAlgorithmQ.front();
                    mAlgorithmQ.pop();
                    break;
                }
            }
        }
        return algorithm;
    }
    void AlgorithmPool::giveBack(Algorithm* algorithm) {

        std::unique_lock <std::mutex> lck(mAlgorithmQ_mtx);
        mAlgorithmQ.push(algorithm);
        //mAlgorithmQ_cv.notify_all();
        mAlgorithmQ_cv.notify_one();

    }
}