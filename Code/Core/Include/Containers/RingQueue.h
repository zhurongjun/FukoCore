#pragma once
#include <CoreType.h>
#include <CoreConfig.h>
#include "Allocator.h"
#include <mutex>

// RingQueueLockPolicy
namespace Fuko
{
	enum ELockPolicy
	{
		ELCK_NoLock,
		ELCK_SReadSWrite,
		ELCK_MReadSWrite,
		ELCK_MReadMWrite,
	};
}

// forward
namespace Fuko
{
	template<typename T,ELockPolicy LockPolicy = ELCK_NoLock,typename Alloc = PmrAllocator>
	class TRingQueue;
}

// no lock ring queue 
namespace Fuko
{
	template<typename T, typename Alloc>
	class TRingQueue<T, ELCK_NoLock, Alloc>
	{
		using SizeType = typename Alloc::USizeType;

		Alloc		m_Allocator;
		T*			m_Data;
		SizeType	m_Max;
		SizeType	m_Head;
		SizeType	m_Tail;

		//-----------------------------------Begin help function-----------------------------------
		FORCENOINLINE void _ResizeTo(SizeType Number)
		{
			if (m_Max != Number) m_Max = m_Allocator.Reserve(m_Data, Number);
		}
		//------------------------------------End help function------------------------------------
	public:
		// construct 
		TRingQueue(const Alloc& InAlloc = Alloc())
			: m_Allocator(InAlloc)
			, m_Data(nullptr)
			, m_Max(0)
			, m_Head(0)
			, m_Tail(0)
		{}

		// copy construct
		TRingQueue(const TRingQueue& Other,const Alloc& InAlloc = Alloc())
			: m_Allocator(InAlloc)
			, m_Data(nullptr)
			, m_Max(0)
			, m_Head(0)
			, m_Tail(0)
		{
			*this = Other;
		}
		
		// move construct 
		TRingQueue(TRingQueue&& Other, const Alloc& InAlloc = Alloc())
			: m_Allocator(InAlloc)
			, m_Data(nullptr)
			, m_Max(0)
			, m_Head(0)
			, m_Tail(0)
		{
			*this = std::move(Other);
		}
		
		// assign
		TRingQueue& operator=(const TRingQueue& Other)
		{
			Reset(Other.Max());
			ConstructItems(m_Data, Other.m_Data, Other.Max());
			m_Max = Other.Max();
			m_Head = Other.m_Head;
			m_Tail = Other.m_Tail;
			return *this;
		}

		// move assign
		TRingQueue& operator=(TRingQueue&& Other)
		{
			// clear queue 
			Empty();

			// copy 
			m_Allocator = Other.m_Allocator;
			m_Data = Other.m_Data;
			m_Max = Other.m_Max;
			m_Head = Other.m_Head;
			m_Tail = Other.m_Tail;

			// invalid other 
			Other.m_Data = nullptr;
			Other.m_Head = Other.m_Tail = Other.m_Max = 0;
			return *this;
		}

		// destruct
		~TRingQueue() 
		{
			while (m_Tail > m_Head)
			{
				DestructItems(&m_Data[m_Head % m_Max]);
				++m_Head;
			}
			m_Allocator.Free(m_Data);
			m_Max = m_Head = m_Tail = 0;
		}
		
		// get information 
		FORCEINLINE T* GetData() { return m_Data; }
		FORCEINLINE const T* GetData() const { return m_Data; }
		FORCEINLINE SizeType Num() const { return m_Tail - m_Head; }
		FORCEINLINE SizeType Max() const { return m_Max; }
		FORCEINLINE SizeType Slack() const { return m_Max - Num(); }
		FORCEINLINE const Alloc& GetAllocator() const { return m_Allocator; }
		FORCEINLINE Alloc& GetAllocator() { return m_Allocator; }

		// reserve 
		FORCEINLINE void Reserve(SizeType Number)
		{
			if (Number > m_Max)
			{
				Normalize();
				_ResizeTo(Number);
			}
		}
		FORCEINLINE void Empty(SizeType InSlack = 0)
		{
			while (m_Tail > m_Head)
			{
				DestructItems(&m_Data[m_Head % m_Max]);
				++m_Head;
			}
			m_Tail = m_Head = 0;
			_ResizeTo(InSlack);
		}
		FORCEINLINE void Reset(SizeType NewSize = 0)
		{
			while (m_Tail > m_Head)
			{
				DestructItems(&m_Data[m_Head % m_Max]);
				++m_Head;
			}
			m_Tail = m_Head = 0;
			Reserve(NewSize);
		}

		// normalize memory layout to array
		FORCEINLINE void Normalize()
		{
			if (!m_Max) return;
			const SizeType HeadIndex = m_Head % m_Max;
			const SizeType TailIndex = m_Tail % m_Max;

			// normalized or empty 
			if (HeadIndex == 0 || m_Head == m_Tail)
			{
				m_Tail -= m_Head;
				m_Head = 0;
				return;
			}

			// normalize
			if (HeadIndex >= TailIndex)
			{
				Algo::Rotate(m_Data, m_Max, HeadIndex);
				m_Tail -= m_Head;
				m_Head = 0;
			}
			else
			{
				Memmove(m_Data, m_Data + TailIndex, Num() * sizeof(T));
				m_Tail -= m_Head;
				m_Head = 0;
			}
		}

		// dequeue, enqueue
		template<typename...Ts>
		FORCEINLINE T* Enqueue(Ts...Args)
		{
			// alloc new memory
			SizeType CurNum = Num();
			check(CurNum <= m_Max);
			if (CurNum == m_Max)
			{
				// reserve array 
				SizeType NewMax = m_Allocator.GetGrow(CurNum + 1, CurNum);
				m_Max = m_Allocator.Reserve(m_Data, NewMax);
			}
			// add element 
			++m_Tail;
			// overflow
			if (m_Tail < m_Head)
			{
				m_Head += m_Max;
				m_Tail += m_Max;
			}
			return new(GetData() + (m_Tail - 1)% m_Max) T(std::forward<Ts>(Args)...);
		}
		FORCEINLINE bool Dequeue()
		{
			T* CurHead = Head();
			if (!CurHead) return false;
			DestructItems(CurHead);
			++m_Head;
			return true;
		}
		FORCEINLINE bool Dequeue(T& OutElement)
		{
			T* CurHead = Head();
			if (!CurHead) return false;
			OutElement = std::move(*CurHead);
			++m_Head;
			return true;
		}

		// access 
		FORCEINLINE T* Tail() { return m_Tail > m_Head ? &m_Data[(m_Tail - 1) % m_Max] : nullptr; }
		FORCEINLINE T* Head() { return m_Tail > m_Head ? &m_Data[m_Head % m_Max] : nullptr; }
		FORCEINLINE const T* Tail() const { return const_cast<TRingQueue*>(this)->Tail(); }
		FORCEINLINE const T* Head() const { return const_cast<TRingQueue*>(this)->Head(); }
	};
}

// single read single write
namespace Fuko
{
	template<typename T, typename Alloc>
	class TRingQueue<T, ELCK_SReadSWrite, Alloc>
	{

	};
}