#pragma once
#include <CoreType.h>
#include <CoreConfig.h>
#include "Allocator.h"
#include <mutex>

namespace Fuko
{
	template<typename T, typename Alloc = PmrAllocator>
	class RingQueue
	{
		using SizeType = Alloc::USizeType;

		Alloc		m_Allocator;
		T*			m_Data;
		SizeType	m_Max;
		SizeType	m_Head;
		SizeType	m_Tail;
		std::mutex	m_Mtx;

		//-----------------------------------Begin help function-----------------------------------
		FORCENOINLINE void _FreeQueue()
		{
			m_Allocator.Free(m_Data);
			m_Max = m_Head = m_Tail = 0;
		}
		FORCENOINLINE void _ResizeGrow()
		{
			// 溢出
			if (m_Head < m_Tail)
			{
				m_Head += m_Max;
				m_Tail += m_Max;
			}
			// 分配 
			SizeType curNum = Num();
			if (curNum > m_Max)
			{
				m_Max = m_Allocator.GetGrow(curNum, m_Max);
				m_Max = m_Allocator.Reserve(m_Data, m_Max);
			}
		}
		FORCENOINLINE void _ResizeShrink()
		{
			SizeType NewMax = m_Allocator.GetShrink(Num(), m_Max);
			_ResizeTo(NewMax);
		}
		FORCENOINLINE void _ResizeTo(SizeType NewMax)
		{
			if (m_Max != NewMax) m_Max = m_Allocator.Reserve(m_Data, NewMax);
		}
		//------------------------------------End help function------------------------------------
	public:
		// construct 
		RingQueue(const Alloc& InAlloc = Alloc())
			: m_Allocator(InAlloc)
			, m_Data(nullptr)
			, m_Max(0)
			, m_Head(0)
			, m_Tail(0)
		{}

		// copy construct
		
		// move construct 
		
		// assign
		
		// move assign
		
		// destruct
		~RingQueue() { _FreeQueue(); }
		

		// get infomation 
		FORCEINLINE T* GetData() { return m_Data; }
		FORCEINLINE const T* GetData() const { return m_Data; }
		FORCEINLINE SizeType Num() const { return m_Head - m_Tail; }
		FORCEINLINE SizeType Max() const { return m_Max; }
		FORCEINLINE SizeType Slack() const { return m_Max - Num(); }
		FORCEINLINE const Alloc& GetAllocator() const { return m_Allocator; }
		FORCEINLINE Alloc& GetAllocator() { return m_Allocator; }

		// reserve 
		FORCEINLINE void Reserve(SizeType Number) { _ResizeTo(Number); }
		
		// normalize memory layout to array
		FORCEINLINE void Normalize()
		{
			const SizeType TailIndex = m_Tail % m_Max;
			const SizeType HeadIndex = m_Head % m_Max;

			// normalized or empty 
			if (TailIndex == 0 || TailIndex == HeadIndex)
			{
				m_Head -= m_Tail;
				m_Tail = 0;
			}

			// normalize
			if (TailIndex > HeadIndex)
			{
				m_Head -= m_Tail;
				m_Tail = 0;
			}
			else
			{
				Memmove(m_Data, m_Data + TailIndex, Num() * sizeof(T));
				m_Head -= m_Tail;
				m_Tail = 0;
			}
		}

		// dequeue, enqueue
		FORCEINLINE T* Enqueue();
	}
}