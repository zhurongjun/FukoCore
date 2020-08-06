#pragma once
#include "JobConfig.hpp"

namespace Fuko::Job
{	
	template<typename T>
	class MutexQueue
	{
		static constexpr uint32_t LOOP_NUM = 300;
		static constexpr uint32_t YIELD_NUM = 100;

		static_assert(std::is_pointer_v<T>, "T must be a pointer type!!!");
		T*			m_Data;
		uint32_t		m_Max;
		uint32_t		m_Mask;
		uint32_t		m_Head;
		uint32_t		m_Tail;
		std::mutex	m_Mutex;
	public:
		MutexQueue(uint32_t InMax = 1024)
			: m_Max(InMax)
			, m_Mask(InMax - 1)
			, m_Head(0)
			, m_Tail(0)
			, m_Data(nullptr)
		{
			Reserve(InMax);
		}
		~MutexQueue()
		{
			if (m_Data)
			{
				FreeContainer(m_Data);
				m_Data = nullptr;
			}
		}

		void Reserve(uint32_t InMax)
		{
			JobAssert((InMax & (InMax - 1)) == 0 && InMax != 0);
			auto Lck = std::lock_guard(m_Mutex);
			T* NewData = (T*)AllocContainer(InMax * sizeof(T*), alignof(T*));
			uint32_t NewMask = InMax - 1;
			if (m_Data)
			{
				for (uint32_t It = m_Head; It != m_Tail; ++It)
				{
					NewData[It & NewMask] = m_Data[It & m_Mask];
				}
				FreeContainer(m_Data);
			}
			m_Data = NewData;
		}

		bool IsEmpty() { return Num() == 0; }
		uint32_t Num() { return m_Tail - m_Head; }
		uint32_t Max() { return m_Max; }

		void Enqueue(T InElement)
		{
		Wait:
			uint32_t GottenTail;
			// wait for any thread dequeue 
			int LoopCount = 0;
			// spin 
			while (Num() >= m_Max)
			{
				if (++LoopCount >= LOOP_NUM)
				{
					LoopCount = 0;
					break;
				}
			}
			// yield 
			while (Num() >= m_Max)
			{
				if (++LoopCount >= YIELD_NUM)
				{
					LoopCount = 0;
					break;
				}
				std::this_thread::yield();
			}
			// sleep 
			while (Num() >= m_Max)
			{
				using namespace std::chrono_literals;
				std::this_thread::sleep_for(1ms);
			}
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
			if (Num() >= m_Max) return false;
			{
				auto Lck = std::lock_guard(m_Mutex);
				if (Num() >= m_Max) return false;
				// now enqueue 
				uint32_t GottenTail = m_Tail;
				++m_Tail;
				*(m_Data + (GottenTail & m_Mask)) = InElement;
				return true;
			}
		}
	
		T Dequeue()
		{
		Wait:
			// wait for any thread enqueue
			// spin 
			while (Num() == 0)
			{
				if (++LoopCount >= LOOP_NUM)
				{
					LoopCount = 0;
					break;
				}
			}
			// yield 
			while (Num() == 0)
			{
				if (++LoopCount >= YIELD_NUM)
				{
					LoopCount = 0;
					break;
				}
				std::this_thread::yield();
			}
			// sleep 
			while (Num() == 0)
			{
				using std::chrono_literals;
				std::this_thread::sleep_for(1ms);
			}
			// lock for dequeue 
			{
				auto Lck = std::lock_guard(m_Mutex);
				// some bitch gotten lock before us, and dequeue an element
				if (Num() == 0) goto Wait;
				// now dequeue
				uint32_t GottenHead = m_Head;
				++m_Head;
				return *(m_Data + (GottenHead & m_Mask));
			}
		}
		T TryDequeue()
		{
			if (Num() == 0) return nullptr;
			{
				auto Lck = std::lock_guard(m_Mutex);
				// some bitch gotten lock before us, and dequeue an element
				if (Num() == 0) return nullptr;
				// now dequeue
				uint32_t GottenHead = m_Head;
				++m_Head;
				return *(m_Data + (GottenHead & m_Mask));
			}
		}
	};

	template<typename T>
	class LockFreeQueue
	{

	};

	template<typename T>
	class WorkStealingQueue
	{
		static_assert(std::is_pointer_v<T>, "T must be a pointer type");


	};
}