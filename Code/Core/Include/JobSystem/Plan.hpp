#pragma once
#include "JobConfig.hpp"

namespace Fuko::Job
{
	class JobBucket;

	class JobPlan
	{
		friend class JobBucket;
		friend class JobExecuter;
	public:
		template<typename TPred,typename TOnDone>
		JobPlan(JobBucket* Bucket, TPred&& Pred, TOnDone&& OnDone);

		template<typename TPred>
		JobPlan(JobBucket* Bucket, TPred&& Pred);
		
		JobPlan(JobBucket* Bucket);
	private:
		// own bucket
		JobBucket*		m_Bucket;

		// promise 
		std::promise<void>	m_Promise;

		// should plan continue? 
		Executable		m_Predicate;
		// on plan done 
		Executable		m_OnDone;

		std::atomic<uint32_t>	m_JoinCount;
	};
}

// Impl
namespace Fuko::Job
{
	template<typename TPred, typename TOnDone>
	JobPlan::JobPlan(JobBucket* Bucket, TPred&& Pred, TOnDone&& OnDone)
		: m_Bucket(Bucket)
		, m_JoinCount(0)
	{
		m_Predicate.BindBranch(std::forward<TPred>(Pred));
		m_OnDone.BindNormal(std::forward<TOnDone>(OnDone));
	}

	template<typename TPred>
	JobPlan::JobPlan(JobBucket* Bucket, TPred&& Pred)
		: m_Bucket(Bucket)
		, m_JoinCount(0)
	{
		m_Predicate.BindBranch(std::forward<TPred>(Pred));
	}

	JobPlan::JobPlan(JobBucket* Bucket)
		: m_Bucket(Bucket)
		, m_JoinCount(0)
	{}
}
