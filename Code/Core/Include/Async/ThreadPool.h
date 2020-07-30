#pragma once
#include <CoreType.h>
#include <CoreConfig.h>
#include <Containers/RingQueue.h>
#include "Task.h"
#include <mutex>

namespace Fuko
{
	namespace Fuko
	{
		class ThreadPool
		{
			TArray<std::thread>		m_Workers;
			TArray<ITask*>			m_Tasks;

			std::mutex				m_WaitMtx;
			std::condition_variable	m_WaitCond;
		};
	}
}