#pragma once
#include <CoreConfig.h>
#include <CoreType.h>
#include <mutex>
#include <atomic>

namespace Fuko
{
	struct NoLock
	{
		FORCEINLINE void lock() {}
		FORCEINLINE void unlock() {}
		FORCEINLINE void try_lock() {}
	};
	struct LockFree : public NoLock {};

	// 互斥锁 
	struct MutexLock
	{
		FORCEINLINE void lock() { Mtx.lock(); }
		FORCEINLINE void unlock() { Mtx.unlock(); }
		FORCEINLINE bool try_lock() { return Mtx.try_lock(); }
	private:
		std::mutex Mtx;
	};
	
	// 自旋锁 
	template<int YieldTime = -1>
	struct TSpinLock
	{
		FORCEINLINE void lock()
		{
			while (LockFlag.test_and_set())
			{
				if constexpr (YieldTime == -1) std::this_thread::yield();
				else if constexpr (YieldTime != 0) std::this_thread::sleep_for(std::chrono::microseconds(YieldTime));
			}
		}
		FORCEINLINE void unlock() { LockFlag.clear(); }
		FORCEINLINE bool try_lock() { return !LockFlag.test_and_set(); }
	private:
		std::atomic_flag LockFlag = ATOMIC_FLAG_INIT;
	};
}