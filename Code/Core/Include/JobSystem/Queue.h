#pragma once
#include "JobConfig.h"

namespace Fuko::Job
{
	// 单读多写安全的队列 
	template<typename T>
	class JobQueue
	{
		static_assert(std::is_pointer_v<T>, "T must be a pointer type");

		struct Arr
		{
			using PData = std::atomic<T>*;
			uint32_t	Max;
			uint32_t	Mask;
			PData		Data;

			inline Arr(int32_t InMax)
				: Max(InMax)
				, Mask(InMax - 1)
				, Data((PData)AllocContainer(InMax * sizeof(PData), alignof(PData)))
			{}
			inline ~Arr() { FreeContainer(Data); }

			uint32_t MaskIndex(uint32_t InIndex) { return InIndex & Mask; }

			void Write(uint32_t i, T InVal) { Data[i & Mask].store(InVal, std::memory_order_relaxed); }
			T Read(uint32_t i) { return Data[i & Mask].load(memory_order_relaxed); }
		};

		std::atomic<Arr*>		m_Arr;
		std::atomic<uint32_t>	m_Head;
		std::atomic<uint32_t>	m_Tail;

		// sizeof(uint32_t) is 32, so garbage never bigger then 32 
		Arr*		m_Garbage[32];
		uint32_t	m_GarbageIndex;

		//=============================Begin help function=============================
		//==============================End help function==============================
	public:
		inline explicit JobQueue(uint32_t InMax = 1024)
			: m_Head(0)
			, m_Tail(0)
			, m_GarbageIndex(0)
		{
			assert(InMax && (!(InMax & (InMax - 1))));
			void* Memory = AllocContainer(sizeof(Arr), alignof(Arr));
			new(Memory) Arr(InMax);
			m_Arr.store((Arr*)Memory, std::memory_order_relaxed);
		}

		inline ~JobQueue() 
		{ 
			CleanGarbage(); 
			auto CurArr = m_Arr.load(std::memory_order_relaxed);
			CurArr->~Arr();
			FreeContainer(CurArr);
		}
		 
		inline JobQueue(const JobQueue&) = delete;
		inline JobQueue(JobQueue&&) = delete;
		inline JobQueue& operator=(const JobQueue&) = delete;
		inline JobQueue& operator=(JobQueue&&) = delete;

		inline void CleanGarbage()
		{
			for (uint32_t i = 0; i < m_GarbageIndex; ++i)
			{
				Arr* pArr = m_Garbage[i];
				pArr->~Arr();
				FreeContainer(pArr);
			}
		}

		inline bool IsEmpty()
		{
			int64_t CurTail = m_Tail.load(std::memory_order_relaxed);
			int64_t CurHead = m_Head.load(std::memory_order_relaxed);
			return CurTail == CurHead;
		}

		inline uint32_t Num()
		{
			int64_t CurTail = m_Tail.load(std::memory_order_relaxed);
			int64_t CurHead = m_Head.load(std::memory_order_relaxed);
			return CurTail - CurHead;
		}
		inline uint32_t Max() { return m_Arr.load(std::memory_order_relaxed)->Max; }

		inline void Enqueue(T Item)
		{
			uint32_t CurTail = m_Tail.load(std::memory_order_relaxed);
			uint32_t CurHead = m_Head.load(std::memory_order_acquire);
			Arr* CurArr = m_Arr.load(std::memory_order_relaxed);

			if (CurArr->Max <= (CurTail - CurHead))
			{
				// alloc new array 
				Arr* LastArr = CurArr;
				CurArr = (Arr*)AllocContainer(sizeof(Arr), alignof(Arr));
				new(CurArr)Arr(LastArr->Max * 2);
				m_Garbage[m_GarbageIndex] = LastArr;
				++m_GarbageIndex;
				
				// copy data 
				for (uint32_t i = CurHead; i < CurTail; ++i)
				{
					CurArr->Write(i, LastArr->Read(i));
				}

				// store array  
				m_Arr.store(CurArr, std::memory_order_relaxed);
			}
			
			CurArr->Data[CurArr->MaskIndex(CurTail)] = Item;

			// before reader get data, writer must wrote data to array 
			std::atomic_thread_fence(std::memory_order_release);
			m_Tail.store(CurTail + 1, std::memory_order_relaxed);
		}

		inline T Dequeue()
		{
			uint32_t GotTail = m_Tail.load(std::memory_order_relaxed) - 1;
			Arr* CurArr = m_Arr.load(std::memory_order_relaxed);
			m_Tail.store(GotTail, std::memory_order_relaxed);

			std::atomic_thread_fence(std::memory_order_seq_cst);

			uint32_t CurHead = m_Head.load(std::memory_order_relaxed);

		}
	};
}