#pragma once
#include "JobConfig.hpp"
#include "Bucket.hpp"
#include "Plan.hpp"
#include "Queue.hpp"

namespace Fuko::Job
{
	class SingleQueueExecuter final
	{
		using PlanBuilder = TPlanBuilder<SingleQueueExecuter>;

		JobVector<std::thread>	m_AllThread;
		MutexQueue<JobNode*>	m_JobQueue;

		JobVector<JobPlan*>		m_WaitingPlan;
		JobVector<JobPlan*>		m_DoingPlan;

		std::mutex				m_PlanMtx;

		std::mutex				m_WaitJobMtx;
		std::condition_variable	m_WaitJobCond;

		std::mutex				m_WaitWorkerMtx;
		std::condition_variable	m_WaitWorkerCond;

		std::atomic<uint32_t>	m_NumActive;
		bool					m_TimeToDie;
	public:
		SingleQueueExecuter(uint32_t NumWorkers = std::thread::hardware_concurrency());
		~SingleQueueExecuter();

		PlanBuilder Execute(JobBucket* Bucket);
		void Execute(JobPlan* Plan);

		void WaitForAll();

	private:
		void _AddWorker();

		inline bool _WaitForJob(JobNode*& Node);
		inline void _Schedule(JobNode* Node);
		inline void _Schedule(const JobVector<JobNode*>& Nodes);
		inline void _TryUpdatePlan(JobPlan* Plan);

		inline void _DoJob(JobNode*& Node);
		inline JobNode* _DoConditionJob(JobNode* Node);
		inline void _DoStaticJob(JobNode* Node);
	};

	using JobExecuter = SingleQueueExecuter;
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

	SingleQueueExecuter::~SingleQueueExecuter()
	{
		WaitForAll();
		m_TimeToDie = true;
		{
			auto Lck = std::unique_lock(m_WaitJobMtx);
			m_WaitJobCond.notify_all();
		}
		for (std::thread& it : m_AllThread)
			it.join();
	}

	TPlanBuilder<SingleQueueExecuter> SingleQueueExecuter::Execute(JobBucket* Bucket)
	{
		// create plan 
		return TPlanBuilder<SingleQueueExecuter>(*JobNew<JobPlan>(Bucket), *this);
	}
	
	void SingleQueueExecuter::Execute(JobPlan* Plan)
	{
		// the plan will never done 
		if (Plan->m_Entries.empty())
		{
			JobDelete(Plan);
			return;
		}

		auto Lck = std::unique_lock(m_PlanMtx);
		
		// collect mask 
		uint32_t CurMask = 0;
		for (JobPlan* It : m_DoingPlan)
		{
			CurMask |= It->m_PlanFlag;
			// plan of same bucket 
			if (Plan->m_Bucket == It->m_Bucket)
			{
				m_WaitingPlan.emplace_back(Plan);
				return;
			}
		}

		// add plan 
		if (Plan->m_PlanFlag & CurMask)
		{
			m_WaitingPlan.emplace_back(Plan);
		}
		else
		{
			Plan->Prepare();
			m_DoingPlan.emplace_back(Plan);
			_Schedule(Plan->m_Entries);
		}
	}

	void SingleQueueExecuter::WaitForAll()
	{
		auto Lck = std::unique_lock(m_WaitWorkerMtx);
		while (m_DoingPlan.size() || m_WaitingPlan.size() || m_JobQueue.Num())
		{
			m_WaitWorkerCond.wait(Lck);
		}
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
		auto Lck = std::unique_lock(m_WaitJobMtx);
		if (m_TimeToDie) return false;
		while (Node == nullptr)
		{
			Node = m_JobQueue.TryDequeue();
			if (Node == nullptr)
			{
				// notify main thread 
				{
					auto Lck = std::lock_guard(m_WaitWorkerMtx);
					m_WaitWorkerCond.notify_all();
				}
				m_WaitJobCond.wait(Lck);
				if (m_TimeToDie) return false;
			}
		}
		return true;
	}

	inline void SingleQueueExecuter::_Schedule(JobNode* Node)
	{
		m_JobQueue.Enqueue(Node);
		m_WaitJobCond.notify_one();
	}

	inline void SingleQueueExecuter::_Schedule(const JobVector<JobNode*>& Nodes)
	{
		if (m_JobQueue.Max() < Nodes.size())
		{
			int count = 0;
			auto NodesSize = Nodes.size();
			while (NodesSize)
			{
				++count;
				NodesSize >>= 1;
			}
			m_JobQueue.Reserve(1 << count);
		}
		for (JobNode* Node : Nodes)
		{
			_Schedule(Node);
		}
	}

	inline void SingleQueueExecuter::_TryUpdatePlan(JobPlan* CurPlan)
	{
		// check done 
		if (CurPlan->m_Predicate.IsValid() && !CurPlan->m_Predicate.InvokeBranch())
		{
			// continue plan  
			CurPlan->Resume();
			_Schedule(CurPlan->m_Entries);
		}
		else
		{
			// end plan and add a new plan 
			auto Lck = std::lock_guard(m_PlanMtx);

			// end plan 
			if (CurPlan->m_OnDone.IsValid())
			{
				CurPlan->m_OnDone.InvokeStatic();
			}
			CurPlan->m_Promise.set_value();

			// erase plan and collect flag 
			uint32_t CurFlag = 0;
			{
				for (auto it = m_DoingPlan.begin(); it != m_DoingPlan.end();)
				{
					if (*it == CurPlan)
					{
						JobDelete(*it);
						it = m_DoingPlan.erase(it);
					}
					else
					{
						CurFlag |= (*it)->m_PlanFlag;
						++it;
					}
				}
			}

			// next plan 
			for (auto it = m_WaitingPlan.begin(); it != m_WaitingPlan.end();)
			{
				// plan of same bucket 
				for (JobPlan* Plan : m_DoingPlan)
				{
					if (Plan->m_Bucket == (*it)->m_Bucket) goto LOOP_END;
				}
				// check flag 
				if (!((*it)->m_PlanFlag & CurFlag))
				{
					(*it)->Prepare();
					m_DoingPlan.emplace_back(*it);
					CurFlag |= (*it)->m_PlanFlag;
					_Schedule((*it)->m_Entries);
					it = m_WaitingPlan.erase(it);
					continue;
				}
			LOOP_END:
				++it;
			}
		}
	}

	inline void SingleQueueExecuter::_DoJob(JobNode*& Node)
	{
	DOJOB:
		if (!Node) return;
		// do job 
		switch (Node->Type())
		{
		case EJobType::PlaceHolder: break;
		case EJobType::Static:
			_DoStaticJob(Node);
			break;
		case EJobType::Condition:
		{
			Node = _DoConditionJob(Node);
			goto DOJOB;	// condition job needn't join next job 
		}
		default: JobAssert(false);
		}
		
		// reset job 
		Node->Resume();

		// join next jobs 
		JobNode* NextJob = nullptr;
		for (JobNode* It : Node->m_JobsDependSelf)
		{
			if (--(It->m_JoinCount)) continue;
			if (!NextJob)
			{
				NextJob = It;
			}
			else
			{
				_Schedule(It);
			}
			It->m_CurPlan->m_JoinCount.fetch_add(1);
		}

		// change plan 
		if (Node->m_CurPlan->m_JoinCount.fetch_sub(1) == 1)
			_TryUpdatePlan(Node->m_CurPlan);

		// continue do job 
		Node = NextJob;
		goto DOJOB;
	}

	inline JobNode* SingleQueueExecuter::_DoConditionJob(JobNode* Node)
	{
		uint32_t NextIndex = Node->m_Executable.InvokeBranch();
		Node->Resume();
		if (NextIndex < Node->m_JobsDependSelf.Num)
		{
			return Node->m_JobsDependSelf[NextIndex];
		}
		return nullptr;
	}

	inline void SingleQueueExecuter::_DoStaticJob(JobNode* Node)
	{
		Node->m_Executable.InvokeStatic();
	}
}