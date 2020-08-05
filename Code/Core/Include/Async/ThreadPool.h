#pragma once
#include <CoreType.h>
#include <CoreConfig.h>
#include <Containers/RingQueue.h>
#include <mutex>
#include <CoreMinimal/Delegate.h>

namespace Fuko
{
	class ThreadPool
	{
		using Task = TDelegate<void()>;

		TArray<std::thread>		m_Workers;
		TRingQueue<Task>		m_Tasks;

		// mutex for protect task queue, and condition for wait queued task 
		std::mutex				m_QueueMutex;
		std::condition_variable	m_QueueWait;

		int32	m_NumDoingTask;
		bool	m_TimeToDie;

		//============================Begin help function============================
		void _Run()
		{
			std::unique_lock<std::mutex> Lck(m_QueueMutex);
			while (true)
			{
				if (!m_Tasks.IsEmpty())
				{
					// get task 
					Task CurTask;
					if (!m_Tasks.Dequeue(CurTask)) continue;
					++m_NumDoingTask;

					// unlock, so that other thread can get task when we are doing task 
					Lck.unlock();

					// do task 
					CurTask();

					// lock for next loop 
					Lck.lock();
					--m_NumDoingTask;
				}
				else if (m_TimeToDie)
				{
					break;
				}
				else
				{
					m_QueueWait.wait(Lck);
				}
			}
		}
		//=============================End help function=============================
	public:
		ThreadPool(int32 NumWorkers = std::thread::hardware_concurrency())
			: m_Workers(NumWorkers * 2)
			, m_Tasks(NumWorkers * 3)
			, m_NumDoingTask(0)
			, m_TimeToDie(false)
		{
			AddWorker(NumWorkers);
		}

		ThreadPool(const ThreadPool&) = delete;
		ThreadPool(ThreadPool&&) = delete;

		~ThreadPool() 
		{
			std::unique_lock<std::mutex> Lck(m_QueueMutex);

			// notify for die
			m_TimeToDie = true;
			m_QueueWait.notify_all();

			// if we have doing task, wait them done 
			while (m_NumDoingTask)
			{
				using namespace std::chrono_literals;
				Lck.unlock();
				std::this_thread::sleep_for(1ms);
				Lck.lock();
			}
			
			Lck.unlock();

			for (std::thread& th : m_Workers)
			{
				th.join();
			}
		}

		template<typename T>
		void ExecTask(T&& InTask)
		{
			std::lock_guard<std::mutex> Lck(m_QueueMutex);
			m_Tasks.Enqueue(InTask);
			m_QueueWait.notify_one();
		}

		void ReserveTaskQueue(uint32 Size)
		{
			std::lock_guard<std::mutex> Lck(m_QueueMutex);
			m_Tasks.Reserve(Size);
		}

		void AddWorker(int32 NumWorkers)
		{
			for (int32 i = 0; i < NumWorkers; ++i)
			{
				m_Workers.Emplace_GetRef(&ThreadPool::_Run, this);
			}
		}
	};
}