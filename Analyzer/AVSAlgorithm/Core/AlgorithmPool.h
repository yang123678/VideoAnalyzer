#ifndef AVSALGORITHM_ALGORITHMPOOL_H
#define AVSALGORITHM_ALGORITHMPOOL_H
#include <Python.h>
#include <mutex>
#include <condition_variable>
#include <queue>

namespace AVSAlgorithmLib {

	struct AlgorithmConfig;
	class Algorithm;

	class AlgorithmPool
	{
	public:
		static std::unique_ptr<AlgorithmPool> Create(AlgorithmConfig* config);

		AlgorithmPool() = delete;
		~AlgorithmPool();
	private:
		AlgorithmPool(AlgorithmConfig* config);
	public:

		Algorithm* gain();// 获取一个实例
		void giveBack(Algorithm* algorithm);// 归还一个实例
	private:
		bool mInitState;//实例化是否成功
		AlgorithmConfig* mConfig;
	
		std::queue<Algorithm*>  mAlgorithmQ;
		std::mutex              mAlgorithmQ_mtx;
		std::condition_variable mAlgorithmQ_cv;

		PyThreadState* _save = nullptr; // 用于替代 Py_BEGIN_ALLOW_THREADS;Py_END_ALLOW_THREADS;
	};
}
#endif //AVSALGORITHM_ALGORITHMPOOL_H
