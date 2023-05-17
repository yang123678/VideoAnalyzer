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

		Algorithm* gain();// ��ȡһ��ʵ��
		void giveBack(Algorithm* algorithm);// �黹һ��ʵ��
	private:
		bool mInitState;//ʵ�����Ƿ�ɹ�
		AlgorithmConfig* mConfig;
	
		std::queue<Algorithm*>  mAlgorithmQ;
		std::mutex              mAlgorithmQ_mtx;
		std::condition_variable mAlgorithmQ_cv;

		PyThreadState* _save = nullptr; // ������� Py_BEGIN_ALLOW_THREADS;Py_END_ALLOW_THREADS;
	};
}
#endif //AVSALGORITHM_ALGORITHMPOOL_H
