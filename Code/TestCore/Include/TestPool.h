#pragma once
#include <Containers/Pool.h>

using Fuko::TPool;
using MemBlock = char[16];

template<typename LockPolicy>
void _PoolWorker(int N, bool* bExit, std::atomic<uint32>* OpCount, TPool<MemBlock, LockPolicy>* InPool)
{
	MemBlock** BlockArr = new MemBlock*[N];
	int Index = 0;
	bool bAlloc = true;
	bool NeedFree = true;

	while (!(*bExit))
	{
		NeedFree = true;
		if (bAlloc)
		{
			BlockArr[Index++] = InPool->Alloc();
			OpCount->fetch_add(1);
			if (Index == N)
			{
				Index = N - 1;
				bAlloc = false;
				continue;
			}
		}
		else
		{
			InPool->Free(BlockArr[Index--]);
			OpCount->fetch_add(1);
			if (Index == -1)
			{
				bAlloc = true;
				Index = 0;
				NeedFree = false;
				continue;
			}
		}
	}

	if (bAlloc) --Index;
	while (NeedFree && Index != -1)
	{
		InPool->Free(BlockArr[Index--]);
	}
	delete BlockArr;
}

void TestPool()
{
	TPool<MemBlock, Fuko::LockFree> Pool(100, 30);
	Pool.ValidateData();

	static constexpr int ThreadNum = 16;

	std::thread WorkThreadArr[ThreadNum];
	bool Exit = false;
	std::atomic<uint32> Count = 0;

	{
		for (int i = 0; i < ThreadNum; ++i)
		{
			WorkThreadArr[i] = std::thread(&_PoolWorker<Fuko::LockFree>, i * 16 + 5, &Exit, &Count, &Pool);
		}

		{
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(3s);
		}

		Exit = true;
		for (int i = 0; i < ThreadNum; ++i)
		{
			WorkThreadArr[i].join();
		}
		Pool.ValidateData();
		std::cout << "LockFree Count: " << Count.load() << std::endl;
	}

	Count.store(0);
	Exit = false;
	TPool<MemBlock, Fuko::MutexLock> MtPool(100, 30);

	{
		for (int i = 0; i < ThreadNum; ++i)
		{
			WorkThreadArr[i] = std::thread(&_PoolWorker<Fuko::MutexLock>, i * 16 + 5, &Exit, &Count, &MtPool);
		}

		{
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(3s);
		}

		Exit = true;
		for (int i = 0; i < ThreadNum; ++i)
		{
			WorkThreadArr[i].join();
		}
		MtPool.ValidateData();
		std::cout << "Mutex Count:    " << Count.load() << std::endl;
	}



}
