#pragma once
#include "JobConfig.hpp"
#include "Executable.hpp"

namespace Fuko::Job
{
	enum class EJobState
	{
		Building ,		// 构建 
		Waiting ,		// 等待
		Doing ,			// 执行
		Pending ,		// 挂起
		Done ,			// 完成
	};

	class JobNode final
	{
		struct JobArr
		{
			JobNode**	Data;
			uint32_t	Num;
			uint32_t	Max;

			JobArr()
				: Data(nullptr)
				, Num(0)
				, Max(0)
			{}
			~JobArr()
			{
				if (Data)
				{
					FreeContainer(Data);
					Data = nullptr;
				}
			}
			inline void Resize(uint32_t InNum)
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
			inline void Grow(uint32 InGrow)
			{
				uint32_t NewNum = Num + InGrow;
				Resize(NewNum);
			}
			inline void Add(JobNode* InNode)
			{
				Grow(1);
				Data[Num] = InNode;
				++Num;
			}
			inline void Reset() { Num = 0; }
			inline JobNode*& operator[](int32_t Index) { return Data[Index]; }
			inline JobNode* const & operator[](int32_t Index) const { return Data[Index]; }
		};

		//========================Begin build step state========================
		// 执行体 
		Executable		m_Executable;
		EExecutableType	m_ExecutableType;

		// 依赖自己的节点数组 
		JobArr			m_JobsDependSelf;
		// 自己依赖的节点 
		JobArr			m_SelfDependJobs;
		//=========================End build step state=========================

		//=========================Begin run step state=========================
		// 依赖计数 
		std::atomic<uint32>			m_SelfDependJobCount;
		// Job状态
		std::atomic<EJobState>		m_JobState;
		//==========================End run step state==========================

		//=========================Begin help function==========================		
		inline void _Precede(JobNode* InNode)
		{
			m_JobsDependSelf.Add(InNode);
			InNode->m_SelfDependJobs.Add(this);
		}
		//==========================End help function===========================
	public:
		JobNode(const JobNode&) = delete;
		JobNode(JobNode&&) = delete;

		//======================Begin build step function=======================
		// 绑定一个函数 
		template<typename TFun>
		void Bind(TFun&& Fun)
		{
			using TRet = decltype(Fun());

			if constexpr (std::is_same_v<TRet, uint32_t>)
			{
				// condition 
				m_ExecutableType = EExecutableType::Condition;

				if (m_Executable.IsValid()) m_Executable.Destroy();
				m_Executable.BindCondition(std::forward<TFun>(Fun));
			}
			else
			{
				// Normal 
				m_ExecutableType = EExecutableType::Normal;

				if (m_Executable.IsValid()) m_Executable.Destroy();
				m_Executable.BindNormal(std::forward<TFun>(Fun));
			}
		}

		// 领先其它Job执行 
		template<typename...Ts>
		void Precede(Ts...Args)
		{
			switch (m_ExecutableType)
			{
			case EExecutableType::Condition:
			{
				m_JobsDependSelf.Resize(sizeof...(Args));
				m_JobsDependSelf.Reset();
				int Temp[] = { 0,(_Precede(Args),0)... };
				(void)Temp;
				break;
			}
			case EExecutableType::Normal:
			{
				m_JobsDependSelf.Grow(sizeof...(Args));
				int Temp[] = { 0,(_Precede(Args),0)... };
				(void)Temp;
				break;
			}
			default: assert(false);
			}
		}

		// 依赖其它Job(Other.Precede(this)) 
		template<typename...Ts>
		void Depend(Ts...Args)
		{
			m_SelfDependJobs.Grow(sizeof...(Args));
			int Temp[] = { 0,(Args->Precede(this),0)... };
			(void)Temp;
		}
		//=======================End build step function========================

		//=======================Begin run step function========================
		inline void Prepare()
		{
			assert(false);
		}
		inline void BeginDoing()
		{
			assert(false);
		}
		inline void TryDone()
		{
			assert(false);
		}
		//========================End run step function=========================

	public:
		friend class JobBucket;
		friend class JobExecuter;

		JobNode()
			: m_SelfDependJobCount(0)
			, m_ExecutableType(EExecutableType::Normal)
		{}

		// 执行Job 
		JobNode* DoJob()
		{
			JobNode* RetJob = nullptr;
			switch (m_ExecutableType)
			{
			case EExecutableType::Condition:
			{
				uint32_t Index = m_Executable.InvokeCondition();
				RetJob = Index < m_JobsDependSelf.Num ? m_JobsDependSelf[Index] : nullptr;
				TryDone();
				break;
			}
			case EExecutableType::Normal:
			{
				m_Executable.InvokeNormal();
				for (uint32_t i = 0; i < m_JobsDependSelf.Num; ++i)
				{
					if (--m_JobsDependSelf[i]->m_SelfDependJobCount == 0 && !RetJob)
					{
						RetJob = m_JobsDependSelf[i];
					}
				}
				TryDone();
				break;
			}
			default: assert(false);
			}
			return RetJob;
		}
	};
}