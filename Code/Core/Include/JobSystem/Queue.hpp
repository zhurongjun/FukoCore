#pragma once
#include "JobConfig.hpp"

namespace Fuko::Job
{	
	template<typename T>
	class SimpleArray
	{
	public:
		T*			Data;
		int32_t		Max;
		int32_t		Mask;

		explicit SimpleArray(int32_t InMax)
			: Max(InMax)
			, Mask(InMax - 1)
		{
			Data = (T*)AllocContainer(InMax * sizeof(T), alignof(T));
		}

		~SimpleArray() { FreeContainer(Data); }

		void Set(int32_t Index, T Data) { Data[Index&Mask] = Data; }
		T Get(int32_t Index) { return Data[Index&Mask]; }
	};

	template<typename T>
	class MutexQueue
	{
		static_assert(std::is_pointer_v<T>, "T must be a pointer type!!!");
		T*			m_Data;
		int32_t		m_Max;
		int32_t		m_Mask;
		int32_t		m_Head;
		int32_t		m_Tail;
		std::mutex	m_Mutex;

		MutexQueue(int32_t InMax = 1024)
			: Max(InMax)
			, Mask(InMax - 1)
		{ assert((InMax & (InMax - 1)) == 0 && InMax != 0); }

		bool IsEmpty() { return Num() != 0; }
		int32_t Num() { return m_Tail - m_Head; }
		int32_t Max() { return m_Max; }

		void Enqueue(T InElement)
		{
		Wait:
			SizeType GottenTail;
			// wait for any thread dequeue 
			while (Num() >= m_Max) std::this_thread::yield();
			// lock for enqueue
			{
				auto Lck = std::lock_guard(m_Mutex);
				// some bitch gotten lock before us, and enqueued an element
				if (Num() >= m_Max) goto Wait;
				// now enqueue 
				GottenTail = m_Tail;
				++m_Tail;
				*(m_Data + (GottenTail & m_Mask)) = InElement;
			}
		}
		bool TryEnqueue(T InElement)
		{
			auto Lck = std::lock_guard(m_Mutex);
			if (Num() >= m_Max) return false;
			// now enqueue 
			SizeType GottenTail = m_Tail;
			++m_Tail;
			*(m_Data + (GottenTail & m_Mask)) = InElement;
			return true;
		}
	
		T Dequeue()
		{
		Wait:
			// wait for any thread enqueue
			while (Num() == 0) std::this_thread::yield();
			// lock for dequeue 
			{
				auto Lck = std::lock_guard(m_Mutex);
				// some bitch gotten lock before us, and dequeue an element
				if (Num() == 0) goto Wait;
				// now dequeue
				SizeType GottenHead = m_Head;
				++m_Head;
				return *(m_Data + (GottenHead & m_Mask));
			}
		}
		T TryDequeue()
		{
			auto Lck = std::lock_guard(m_Mutex);
			// some bitch gotten lock before us, and dequeue an element
			if (Num() == 0) return nullptr;
			// now dequeue
			SizeType GottenHead = m_Head;
			++m_Head;
			return *(m_Data + (GottenHead & m_Mask));
		}
	};

	template<typename T>
	class LockFreeQueue
	{

	};

	template<typename T>
	class WorkStealingQueue
	{

	};
}