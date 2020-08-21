#pragma once
#include "JobConfig.hpp"

namespace Fuko::Job
{
	class JobBucketBuilder
	{
		JobVector<JobNode*>&	m_Nodes;
		//=======================Begin help functions=======================
		template<typename TFun>
		inline Job _Emplace(TFun&& InFun);
		//========================End help functions========================
	public:
		inline JobBucketBuilder(JobVector<JobNode*>& InVector);

		// 创建一个占位符Job 
		inline Job PlaceHolder();

		// 创建一个Job 
		template<typename...TFuns>
		inline decltype(auto) Emplace(TFuns&&...Funs);

		// 并行for循环 
		template<typename TFun>
		inline std::pair<Job, Job> ParallelFor(uint32_t InLoopNum, TFun&& InFun, uint32_t InChunkNum = 0);

		// 并行foreach循环 
		template<typename It, typename TFun>
		inline std::pair<Job, Job> ParallelEach(It Begin, It End, TFun&& InFun, uint32_t InChunkNum = 0);

	};
}

// Impl
namespace Fuko::Job
{
	inline JobBucketBuilder::JobBucketBuilder(JobVector<JobNode *>& InVector)
		: m_Nodes(InVector)
	{}

	template<typename TFun>
	inline Job JobBucketBuilder::_Emplace(TFun&& InFun)
	{
		Job Node = PlaceHolder();
		Node.Bind(std::forward<TFun>(InFun));
		return Node;
	}

	inline Job JobBucketBuilder::PlaceHolder()
	{
		JobNode* Node = (JobNode*)Alloc(sizeof(JobNode), alignof(JobNode));
		new (Node)JobNode();
		m_Nodes.emplace_back(Node);
		return Job(Node);
	}

	template<typename...TFuns>
	inline decltype(auto) JobBucketBuilder::Emplace(TFuns&&...Funs)
	{
		if constexpr (sizeof...(TFuns) == 1)
			return _Emplace(std::forward<TFuns>(Funs)...);
		else
			return std::make_tuple(_Emplace(std::forward<TFuns>(Funs))...);
	}

	template<typename TFun>
	inline std::pair<Job, Job> JobBucketBuilder::ParallelFor(uint32_t InLoopNum, TFun&& InFun, uint32_t InChunkNum)
	{
		Job Begin = PlaceHolder();
		Job End = PlaceHolder();
		InChunkNum = InChunkNum ? InChunkNum : std::thread::hardware_concurrency();
		if (InLoopNum <= InChunkNum)
		{
			InChunkNum = InLoopNum;
			uint32_t Step = 1;
			for (uint32_t i = 0; i < InChunkNum; ++i)
			{
				Emplace([=] { InFun(i); }).Precede(End).Depend(Begin);
			}
		}
		else
		{
			uint32_t Step = InLoopNum / InChunkNum;
			uint32_t AdvanceNum = InLoopNum - InChunkNum * Step;
			uint32_t AdvanceStep = Step + 1;
			uint32_t BaseCount = 0;
			for (uint32_t i = 0; i < InChunkNum; ++i)
			{
				if (i < AdvanceNum)
				{
					Emplace([=] { for (uint32_t j = 0; j < AdvanceStep; ++j) InFun(j + BaseCount); }).Precede(End).Depend(Begin);
					BaseCount += AdvanceStep;
				}
				else
				{
					Emplace([=] { for (uint32_t j = 0; j < Step; ++j) InFun(j + BaseCount); }).Precede(End).Depend(Begin);
					BaseCount += Step;
				}
			}
		}
		return std::make_pair(Begin, End);
	}

	template<typename It, typename TFun>
	inline std::pair<Job, Job> JobBucketBuilder::ParallelEach(It BeginIt, It EndIt, TFun&& InFun, uint32_t InChunkNum)
	{
		Job Begin = PlaceHolder();
		Job End = PlaceHolder();
		InChunkNum = InChunkNum ? InChunkNum : std::thread::hardware_concurrency();

		uint32_t LoopNum = std::distance(BeginIt, EndIt);
		if (LoopNum <= InChunkNum)
		{
			InChunkNum = LoopNum;
			uint32_t Step = 1;
			for (uint32_t i = 0; i < InChunkNum; ++i)
			{
				Emplace([=] { InFun(*BeginIt); }).Precede(End).Depend(Begin);
				++BeginIt;
			}
		}
		else
		{
			uint32_t Step = LoopNum / InChunkNum;
			uint32_t AdvanceNum = LoopNum - InChunkNum * Step;
			uint32_t AdvanceStep = Step + 1;
			for (uint32_t i = 0; i < InChunkNum; ++i)
			{
				if (i < AdvanceNum)
				{
					Emplace([BeginIt, InFun, AdvanceStep] { auto EndIt = BeginIt; std::advance(EndIt, AdvanceStep); std::for_each(BeginIt, EndIt, InFun); }).Precede(End).Depend(Begin);
					std::advance(BeginIt, AdvanceStep);
				}
				else
				{
					Emplace([BeginIt, InFun, Step] { auto EndIt = BeginIt; std::advance(EndIt, Step); std::for_each(BeginIt, EndIt, InFun); }).Precede(End).Depend(Begin);
					std::advance(BeginIt, Step);
				}
			}
		}


		return std::make_pair(Begin, End);
	}
}
