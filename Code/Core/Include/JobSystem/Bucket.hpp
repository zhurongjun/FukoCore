#pragma once
#include "JobConfig.hpp"
#include "Node.hpp"
#include "Plan.hpp"

namespace Fuko::Job
{
	class JobBucket
	{
		JobVector<JobNode*>	m_AllNodes;

		//=======================Begin help functions=======================
		template<typename TFun> 
		JobNode* _Emplace(TFun&& InFun);
		//========================End help functions========================
	public:
		JobBucket();
		~JobBucket();

		template<typename...TFuns> 
		inline decltype(auto) Emplace(TFuns&&...Funs);

		inline JobNode* PlaceHolder();

	private:
		inline void Prepare();
	};
}

// Impl
namespace Fuko::Job
{
	JobBucket::JobBucket()
		: m_AllNodes()
	{
		m_AllNodes.reserve(64);
	}
	
	JobBucket::~JobBucket()
	{
	}

	template<typename TFun>
	JobNode* JobBucket::_Emplace(TFun&& InFun)
	{
		JobNode* Node = PlaceHolder();
		Node->Bind(std::forward<TFun>(InFun));
		return Node;
	}

	inline JobNode* JobBucket::PlaceHolder()
	{
		JobNode* Node = (JobNode*)Alloc(sizeof(JobNode), alignof(JobNode));
		new (Node)JobNode();
		m_AllNodes.emplace_back(Node);
		return Node;
	}

	template<typename...TFuns>
	inline decltype(auto) JobBucket::Emplace(TFuns&&...Funs)
	{
		return std::make_tuple(_Emplace(std::forward<TFuns>(Funs))...);
	}

}
