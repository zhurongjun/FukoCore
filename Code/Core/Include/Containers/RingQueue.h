#pragma once
#include <CoreType.h>
#include <CoreConfig.h>
#include <mutex>
#include "Allocator.h"
#include "LockPolicy.h"

// forward
namespace Fuko
{
	template<typename T, typename TLockPolicy = NoLock, typename Alloc = PmrAlloc>
	class TRingQueue;
}

// no lock ring queue 
namespace Fuko
{
	template<typename T, typename Alloc>
	class TRingQueue<T, NoLock, Alloc>
	{
	protected:
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
		TRingQueue(SizeType InitSize, const Alloc& InAlloc = Alloc())
			: m_Allocator(InAlloc)
			, m_Data(nullptr)
			, m_Max(0)
			, m_Head(0)
			, m_Tail(0)
		{
			Reserve(InitSize);
		}

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
		FORCEINLINE bool IsEmpty() const { return m_Tail == m_Head; }
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
		FORCEINLINE T* Enqueue(Ts&&...Args)
		{
			// alloc new memory
			SizeType CurNum = Num();
			check(CurNum <= m_Max);
			if (CurNum == m_Max)
			{
				// reserve array 
				SizeType NewMax = m_Allocator.GetGrow(CurNum + 1, CurNum);
				Reserve(NewMax);
			}
			// add element 
			auto LastTail = m_Tail;
			++m_Tail;
			return new(GetData() + LastTail % m_Max) T(std::forward<Ts>(Args)...);
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
		FORCEINLINE T* Tail() { return m_Data ? m_Tail >= m_Head ? &m_Data[(m_Tail - 1) % m_Max] : nullptr : nullptr; }
		FORCEINLINE T* Head() { return m_Data ? m_Tail >= m_Head ? &m_Data[m_Head % m_Max] : nullptr : nullptr; }
		FORCEINLINE const T* Tail() const { return const_cast<TRingQueue*>(this)->Tail(); }
		FORCEINLINE const T* Head() const { return const_cast<TRingQueue*>(this)->Head(); }
	};
}

// ring queue with lock
namespace Fuko
{
	inline constexpr bool bUseFlagPad = true;
	template<typename T, typename TLockPolicy, typename Alloc>
	class TRingQueue : protected TRingQueue<T, NoLock, Alloc>
	{
		using SizeType = typename Alloc::USizeType;

		static constexpr uint32 EmptyMask = 0u;
		static constexpr uint32 FullMask = ~EmptyMask;

		Alloc		m_Allocator;
		T*			m_Data;
		SizeType	m_Max;
		std::atomic<SizeType>	m_Head;
		std::atomic<SizeType>	m_Tail;

		TLockPolicy			m_Lock;

		//-----------------------------------Begin help function-----------------------------------
		FORCENOINLINE void _ResizeTo(SizeType Number)
		{
			if (m_Max != Number) m_Max = m_Allocator.Reserve(m_Data, Number);
		}
		FORCEINLINE void _Normalize()
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
		TRingQueue(SizeType InitSize, const Alloc& InAlloc = Alloc())
			: m_Allocator(InAlloc)
			, m_Data(nullptr)
			, m_Max(0)
			, m_Head(0)
			, m_Tail(0)
		{ Reserve(InitSize); }

		// destruct
		~TRingQueue() { if (m_Data) m_Allocator.Free(m_Data); }

		// delete for protect data
		TRingQueue(const TRingQueue& Other, const Alloc& InAlloc = Alloc()) = delete;
		TRingQueue(TRingQueue&& Other, const Alloc& InAlloc = Alloc()) = delete;
		TRingQueue& operator=(const TRingQueue& Other) = delete;
		TRingQueue& operator=(TRingQueue&& Other) = delete;

		// get information 
		// No GetData()
		FORCEINLINE SizeType Num() const { return m_Tail - m_Head; }
		FORCEINLINE SizeType Max() const { return m_Max; }
		FORCEINLINE SizeType Slack() const { return m_Max - Num(); }
		FORCEINLINE bool IsEmpty() const { return Num() == 0; }
		FORCEINLINE const Alloc& GetAllocator() const { m_Allocator; }
		FORCEINLINE Alloc& GetAllocator() { return m_Allocator; }

		// reserve 
		FORCEINLINE void Reserve(SizeType Number)
		{
			m_Lock.lock();
			Number = Math::RoundUpToPowerOfTwo(Number);
			if (Number > m_Max)
			{
				_Normalize();
				_ResizeTo(Number);
			}
			m_Lock.unlock();
		}
		FORCEINLINE void Empty(SizeType InSlack = 0)
		{
			m_Lock.lock();
			InSlack = Math::RoundUpToPowerOfTwo(InSlack);
			while (m_Tail > m_Head)
			{
				DestructItems(&m_Data[m_Head % m_Max]);
				++m_Head;
			}
			m_Tail = m_Head = 0;
			_ResizeTo(InSlack);
			m_Lock.unlock();
		}
		FORCEINLINE void Reset(SizeType NewSize = 0)
		{
			m_Lock.lock();
			NewSize = Math::RoundUpToPowerOfTwo(NewSize);
			while (m_Tail > m_Head)
			{
				DestructItems(&m_Data[m_Head % m_Max]);
				++m_Head;
			}
			m_Tail = m_Head = 0;
			Reserve(NewSize);
			m_Lock.unlock();
		}

		// normalize memory layout to array
		FORCEINLINE void Normalize()
		{
			m_Lock.lock();
			_Normalize();
			m_Lock.unlock();
		}
		
		// block enqueue 
		template<typename...Ts>
		FORCEINLINE void Enqueue(Ts&&...Args)
		{
		Wait:
			SizeType GottenTail;
			// wait for any thread dequeue 
			while (Num() >= m_Max) std::this_thread::yield();
			// lock for enqueue
			{
				std::lock_guard<TLockPolicy> Lck(m_Lock);
				// some bitch gotten lock before us, and enqueued an element
				if (Num() >= m_Max) goto Wait;
				// now enqueue 
				GottenTail = m_Tail;
				++m_Tail;
				new (m_Data + (GottenTail & (m_Max - 1)))T(std::forward<Ts>(Args)...);
			}
		}
		// unblock enqueue
		template<typename...Ts>
		FORCEINLINE bool TryEnqueue(Ts&&...Args)
		{
			std::lock_guard<TLockPolicy> Lck(m_Lock);
			if (Num() >= m_Max) return false;
			// now enqueue 
			SizeType GottenTail = m_Tail;
			++m_Tail;
			new (m_Data + (GottenTail & (m_Max - 1)))T(std::forward<Ts>(Args)...);
			return true;
		}
		// block dequeue
		FORCEINLINE void Dequeue()
		{
		Wait:
			SizeType GottenHead
			// wait for any thread enqueue
			while (Num() == 0) std::this_thread::yield();
			// lock for dequeue 
			{
				std::lock_guard<TLockPolicy> Lck(m_Lock);
				// some bitch gotten lock before us, and dequeue an element
				if (Num() == 0) goto Wait;
				// now dequeue
				GottenHead = m_Head;
				++m_Head;
				m_Data[GottenHead & (m_Max - 1)].~T();
			}
		}
		FORCEINLINE void Dequeue(T& OutElement)
		{
		Wait:
			// wait for any thread enqueue
			while (Num() == 0) std::this_thread::yield();
			// lock for dequeue 
			{
				std::lock_guard<TLockPolicy> Lck(m_Lock);
				// some bitch gotten lock before us, and dequeue an element
				if (Num() == 0) goto Wait;
				// now dequeue
				SizeType GottenHead = m_Head;
				++m_Head;
				OutElement = std::move(m_Data[GottenHead & (m_Max - 1)]);
			}
		}
		// unblock enqueue 
		FORCEINLINE bool TryDequeue()
		{
			std::lock_guard<TLockPolicy> Lck(m_Lock);
			// some bitch gotten lock before us, and dequeue an element
			if (Num() == 0) return false;
			// now dequeue
			SizeType GottenHead = m_Head;
			++m_Head;
			m_Data[GottenHead & (m_Max - 1)].~T();
			return true;
		}
		FORCEINLINE bool TryDequeue(T& OutElement)
		{
			std::lock_guard<TLockPolicy> Lck(m_Lock);
			// some bitch gotten lock before us, and dequeue an element
			if (Num() == 0) return false;
			// now dequeue
			SizeType GottenHead = m_Head;
			++m_Head;
			OutElement = std::move(m_Data[GottenHead & (m_Max - 1)])
			return true;
		}
	};
}