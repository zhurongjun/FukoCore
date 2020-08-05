#pragma once
#include "JobConfig.hpp"

namespace Fuko::Job
{
	class JobBucketBuilder
	{
		JobVector<JobNode*>&	m_Nodes;
		//=======================Begin help functions=======================
		template<typename TFun>
		JobNode* _Emplace(TFun&& InFun);
		//========================End help functions========================
	public:
		JobBucketBuilder(JobVector<JobNode*>& InVector);

		template<typename...TFuns>
		inline decltype(auto) Emplace(TFuns&&...Funs);

		inline JobNode* PlaceHolder();

	};
}

// Impl
namespace Fuko::Job
{
	JobBucketBuilder::JobBucketBuilder(JobVector<JobNode *>& InVector)
		: m_Nodes(InVector)
	{}

	template<typename TFun>
	JobNode* JobBucketBuilder::_Emplace(TFun&& InFun)
	{
		JobNode* Node = PlaceHolder();
		Node->Bind(std::forward<TFun>(InFun));
		return Node;
	}

	inline JobNode* JobBucketBuilder::PlaceHolder()
	{
		JobNode* Node = (JobNode*)Alloc(sizeof(JobNode), alignof(JobNode));
		new (Node)JobNode();
		m_Nodes.emplace_back(Node);
		return Node;
	}

	template<typename...TFuns>
	inline decltype(auto) JobBucketBuilder::Emplace(TFuns&&...Funs)
	{
		if constexpr (sizeof...(TFuns) == 1)
			return _Emplace(std::forward<TFuns>(Funs)...);
		else
			return std::make_tuple(_Emplace(std::forward<TFuns>(Funs))...);
	}
}
