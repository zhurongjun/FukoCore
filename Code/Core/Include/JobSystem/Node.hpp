#pragma once
#include "JobConfig.hpp"
#include "Executable.hpp"

namespace Fuko::Job
{
	enum class EJobType
	{
		PlaceHolder ,	// PlaceHolder task 
		Static,			// Static task
		Condition,		// Condition task
	};

	enum EJobFlag
	{
		BRANCH = 1,
	};

	class JobPlan;

	class JobNode final
	{
		friend class JobBucket;
		friend class JobBucketBuilder;
		friend class JobPlan;
		friend class SingleQueueExecuter;

		struct JobArr
		{
			JobNode**	Data;
			uint32_t	Num;
			uint32_t	Max;

			inline JobArr();
			inline ~JobArr();
			inline void Resize(uint32_t InNum);
			inline void Grow(uint32 InGrow);
			inline void Add(JobNode* InNode);
			inline void Reset() { Num = 0; }
			inline JobNode*& operator[](int32_t Index) { return Data[Index]; }
			inline JobNode* const & operator[](int32_t Index) const { return Data[Index]; }

			inline JobNode** begin() { return Data; }
			inline JobNode* const* begin() const { return Data; }
			inline JobNode** end() { return Data + Num; }
			inline JobNode* const* end() const { return Data + Num; }
		};

		// 执行体 
		Executable	m_Executable;
		EJobType	m_JobType;

		// 依赖自己的节点数组 
		JobArr		m_JobsDependSelf;
		// 自己依赖的节点 
		JobArr		m_SelfDependJobs;
		// Flag 
		uint32_t	m_Flags;

		// 当前所属的计划表 
		JobPlan*	m_CurPlan;
		// 依赖计数 
		std::atomic<uint32>			m_JoinCount;

		//=========================Begin help function==========================		
		inline void _Precede(JobNode* InNode);
		//==========================End help function===========================
	public:
		JobNode(const JobNode&) = delete;
		JobNode(JobNode&&) = delete;

		// Build process 
		template<typename TFun> inline void Bind(TFun&& Fun);
		template<typename...Ts> inline void Precede(Ts...Args);
		template<typename...Ts> inline void Depend(Ts...Args);
		inline void Unbind();

		// Get Info
		inline uint32_t NumDependentsStroge() const;
		inline uint32_t NumDependentsWeak() const;
		inline uint32_t NumDependents() const { return m_SelfDependJobs.Num; }
		inline uint32_t NumPrecede() const { return m_JobsDependSelf.Num; }
		inline EJobType	Type() const { return m_JobType; }
		inline bool		IsValid() const { return m_Executable.IsValid(); }
		inline bool		HasFlag(EJobFlag Flag) const { return (m_Flags & Flag) == Flag; }
		inline void		SetFlag(EJobFlag Flag) { m_Flags |= Flag; }
		inline void		EraseFlag(EJobFlag Flag) { m_Flags &= ~Flag; }
		inline void		ClearFlag() { m_Flags = 0; }

		// Execute process 
		inline void		Prepare();	// Prepare for execute 
		inline void		Resume();	// Resume job for next execute 

	private:
		JobNode();
	};
}

// Job Arr 
namespace Fuko::Job
{
	inline JobNode::JobArr::JobArr()
		: Data(nullptr)
		, Num(0)
		, Max(0)
	{}

	inline JobNode::JobArr::~JobArr()
	{
		if (Data)
		{
			FreeContainer(Data);
			Data = nullptr;
		}
	}

	inline void JobNode::JobArr::Resize(uint32_t InNum)
	{
		uint32_t NewMax;
		if (InNum > Max)
		{
			NewMax = InNum > 8 ? InNum * 4 / 3 : 8;
			JobNode** NewData = (JobNode**)AllocContainer(NewMax * sizeof(JobNode*), alignof(JobNode*));
			if (Data)
			{
				Memcpy(NewData, Data, sizeof(JobNode*) * Num);
				FreeContainer(Data);
			}
			Data = NewData;
			Max = NewMax;
		}
	}

	inline void JobNode::JobArr::Grow(uint32 InGrow)
	{
		uint32_t NewNum = Num + InGrow;
		Resize(NewNum);
	}

	inline void JobNode::JobArr::Add(JobNode* InNode)
	{
		Grow(1);
		Data[Num] = InNode;
		++Num;
	}
}

// Job Node
namespace Fuko::Job
{
	//=================================Private Functions=================================
	inline JobNode::JobNode()
		: m_JoinCount(0)
		, m_Flags(0)
		, m_JobType(EJobType::PlaceHolder)
	{}

	inline void JobNode::_Precede(JobNode* InNode)
	{
		m_JobsDependSelf.Add(InNode);
		InNode->m_SelfDependJobs.Add(this);
	}

	//==================================Public Functions=================================
	template<typename TFun>
	inline void JobNode::Bind(TFun&& Fun)
	{
		using TRet = decltype(Fun());

		if constexpr (std::is_integral_v<TRet>)
		{
			// condition 
			m_JobType = EJobType::Condition;
			m_Executable.BindCondition(std::forward<TFun>(Fun));
		}
		else
		{
			// Static 
			m_JobType = EJobType::Static;
			m_Executable.BindStatic(std::forward<TFun>(Fun));
		}
	}

	inline void JobNode::Unbind()
	{
		m_Executable.Destroy();
		m_JobType = EJobType::PlaceHolder;
	}

	template<typename...Ts>
	inline void JobNode::Precede(Ts...Args)
	{
 		switch (m_JobType)
		{
		case EJobType::Condition:
		{
			m_JobsDependSelf.Resize(sizeof...(Args));
			m_JobsDependSelf.Reset();
			int Temp[] = { 0,(_Precede(Args),0)... };
			(void)Temp;
			break;
		}
		case EJobType::Static:
		case EJobType::PlaceHolder:
		{
			m_JobsDependSelf.Grow(sizeof...(Args));
			int Temp[] = { 0,(_Precede(Args),0)... };
			(void)Temp;
			break;
		}
		default: JobAssert(false);
		}
	}

	template<typename...Ts>
	inline void JobNode::Depend(Ts...Args)
	{
		m_SelfDependJobs.Grow(sizeof...(Args));
		int Temp[] = { 0,(Args->Precede(this),0)... };
		(void)Temp;
	}

	inline uint32_t JobNode::NumDependentsStroge() const
	{
		uint32_t Count = 0;
		for (JobNode** It = m_SelfDependJobs.Data,
			**End = m_SelfDependJobs.Data + m_SelfDependJobs.Num;
			It != End; ++It)
		{
			if ((*It)->m_JobType != EJobType::Condition) ++Count;
		}
		return Count;
	}
	
	inline uint32_t JobNode::NumDependentsWeak() const
	{
		uint32_t Count = 0;
		for (JobNode** It = m_SelfDependJobs.Data,
			 **End = m_SelfDependJobs.Data + m_SelfDependJobs.Num;
			It != End; ++It)
		{
			if ((*It)->m_JobType == EJobType::Condition) ++Count;
		}
		return Count;
	}

	inline void JobNode::Prepare()
	{
		ClearFlag();
		for (JobNode* It : m_SelfDependJobs)
		{
			if (It->Type() == EJobType::Condition) SetFlag(EJobFlag::BRANCH);
		}
		m_JoinCount = NumDependentsStroge();
	}

	inline void JobNode::Resume()
	{
		if (HasFlag(EJobFlag::BRANCH))
			m_JoinCount = NumDependentsStroge();
		else
			m_JoinCount = NumDependents();
	}
}





