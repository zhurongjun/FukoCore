#pragma once
#include "JobConfig.hpp"

namespace Fuko::Job
{
	class JobExecuter;

	class SingleQueueExecuter final
	{
		JobVector<std::thread>	m_AllThread;
		MutexQueue<JobNode*>	m_JobQueue;

		std::mutex				m_WaitJobMtx;
		std::condition_variable	m_WaitJobCond;

		std::mutex				m_WaitWorkerMtx;
		std::condition_variable	m_WaitWorkerCond;

		std::atomic<uint32_t>	m_NumActive;
		bool					m_TimeToDie;
	public:
		SingleQueueExecuter(uint32_t NumWorkers = std::thread::hardware_concurrency());

	private:
		void _AddWorker();

		inline bool _WaitForJob(JobNode*& Node);
		inline void _DoJob(JobNode* Node);
		inline void _DoConditionJob(JobNode* Node);
		inline void _DoStaticJob(JobNode* Node);
	};
}


// Impl 
namespace Fuko::Job
{
	SingleQueueExecuter::SingleQueueExecuter(uint32_t NumWorkers)
		: m_AllThread()
		, m_NumActive(0)
		, m_TimeToDie(false)
	{
		uint32_t ReserveWorkerNum = NumWorkers * 4 / 3;
		m_AllThread.reserve(ReserveWorkerNum);
		for (uint32_t i = 0; i < NumWorkers; ++i) _AddWorker();
	}

	void SingleQueueExecuter::_AddWorker()
	{
		m_AllThread.emplace_back(
		[this]()
		{
			// work loop 
			JobNode* Node = nullptr;
			while (true)
			{
				if (!_WaitForJob(Node)) break;
				_DoJob(Node);
			}
		});
	}

	inline bool SingleQueueExecuter::_WaitForJob(JobNode*& Node)
	{
		while (Node == nullptr)
		{
			Node = m_JobQueue.TryDequeue();
			if (Node == nullptr)
			{
				auto Lck = std::unique_lock(m_WaitJobMtx);
				m_WaitJobCond.wait(m_WaitJobMtx);
				if (m_TimeToDie) return false;
			}
		}
		return true;
	}

	inline void SingleQueueExecuter::_DoJob(JobNode* Node)
	{
		if (!Node) return;
		
		
	}

}