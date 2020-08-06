#pragma once
#include "JobConfig.hpp"
#include "Bucket.hpp"

namespace Fuko::Job
{
	class JobPlan
	{
		friend class JobBucket;
		friend class SingleQueueExecuter;
		template<typename T>
		friend class TPlanBuilder;
	public:
		JobPlan(JobBucket* Bucket);
	private:
		void Prepare();
		void Resume();
	private:
		// own bucket
		JobBucket*		m_Bucket;

		// plan entry 
		JobVector<JobNode*>	m_Entries;
		// promise 
		std::promise<void>	m_Promise;

		// should plan continue? 
		Executable		m_Predicate;
		// on plan begin 
		Executable		m_OnPrepare;
		// on plan done 
		Executable		m_OnDone;

		// if (Other.m_PlanFlag & m_PlanFlag) != 0 
		// the two flag can't execute at same time  
		uint32_t		m_PlanFlag;

		std::atomic<uint32_t>	m_JoinCount;
	};

	template<typename T>
	class TPlanBuilder
	{
		JobPlan&	m_Plan;
		T&			m_Executer;
	public:
		TPlanBuilder(JobPlan& InPlan, T& InExecuter) : m_Plan(InPlan), m_Executer(InExecuter) {}
		~TPlanBuilder() { m_Executer.Execute(&m_Plan); }

		TPlanBuilder(TPlanBuilder&&) = delete;
		TPlanBuilder(const TPlanBuilder&) = delete;
		TPlanBuilder& operator=(const TPlanBuilder&) = delete;
		TPlanBuilder& operator=(TPlanBuilder&&) = delete;

		inline TPlanBuilder& Future(std::future<void>& Future) { Future = m_Plan.m_Promise.get_future(); return *this; }
		inline TPlanBuilder& Sync(uint32_t SyncFlag) { m_Plan.m_PlanFlag |= SyncFlag; return *this; }
		template<typename TFun>
		inline TPlanBuilder& Pred(TFun&& Fun) { m_Plan.m_Predicate.BindBranch(std::forward<TFun>(Fun)); return *this; }
		template<typename TFun>
		inline TPlanBuilder& OnPrepare(TFun&& Fun) { m_Plan.m_OnPrepare.BindStatic(std::forward<TFun>(Fun)); return *this; }
		template<typename TFun>
		inline TPlanBuilder& OnDone(TFun&& Fun) { m_Plan.m_OnDone.BindStatic(std::forward<TFun>(Fun)); return *this; }
		inline TPlanBuilder& DoN(uint32_t N) { m_Plan.m_Predicate.BindBranch([N]() mutable { return --N == 0; }); return *this; }
	};
}

// Impl
namespace Fuko::Job
{
	JobPlan::JobPlan(JobBucket* Bucket)
		: m_Bucket(Bucket)
		, m_JoinCount(0)
		, m_PlanFlag(0)
	{
		Bucket->Prepare();
		m_Entries.reserve(8);
		m_Entries.clear();
		for (JobNode* Node : Bucket->m_AllNodes)
		{
			if (Node->NumDependents() == 0)
				m_Entries.emplace_back(Node);
		}
	}

	void JobPlan::Prepare()
	{
		if (m_OnPrepare.IsValid()) m_OnPrepare.InvokeStatic();
		for (JobNode* Node : m_Bucket->m_AllNodes)
		{
			Node->m_CurPlan = this;
		}
		m_JoinCount = m_Entries.size();
	}

	void JobPlan::Resume()
	{
		m_JoinCount = m_Entries.size();
	}
}
