#pragma once
#include "JobConfig.hpp"
#include "Node.hpp"
#include "BucketBuilder.hpp"

namespace Fuko::Job
{
	class JobBucket : public JobBucketBuilder
	{
		friend class SingleQueueExecuter;
		friend class JobPlan;

		JobVector<JobNode*>	m_AllNodes;
		JobString			m_BucketName;
		bool				m_HasPrepare;
	public:
		JobBucket();
		~JobBucket();

		void	Prepare();
		void	Reset();
	private:
	};
}

// Impl
namespace Fuko::Job
{
	JobBucket::JobBucket()
		: JobBucketBuilder(m_AllNodes)
		, m_AllNodes()
		, m_HasPrepare(false)
	{
		m_AllNodes.reserve(64);
	}
	
	JobBucket::~JobBucket()
	{
		for (JobNode* Node : m_AllNodes)
		{
			JobDelete(Node);
		}
	}

	void JobBucket::Prepare()
	{
		if (m_HasPrepare) return;
		for (JobNode* Node : m_AllNodes)
		{
			Node->Prepare();
		}
		m_HasPrepare = true;
	}

	void JobBucket::Reset()
	{
		m_HasPrepare = false;
	}
}
