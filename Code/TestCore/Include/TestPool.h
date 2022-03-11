#pragma once
#include <Containers/Pool.h>

using Fuko::TPool;
using MemBlock = char[16];

template<typename LockPolicy>
void _PoolWorker(int N, bool* bExit, bool* bStart, std::atomic<uint32>* OpCount, TPool<MemBlock, LockPolicy>* InPool)
{
	MemBlock** BlockArr = new MemBlock*[N];
	int Index = 0;
	bool bAlloc = true;
	bool NeedFree = true;

    // wait
    while(!(*bStart)) std::this_thread::yield();

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
    bool Start = false;
	std::atomic<uint32> Count = 0;
    size_t time_per_second;

	{
		for (int i = 0; i < ThreadNum; ++i)
		{
			WorkThreadArr[i] = std::thread(&_PoolWorker<Fuko::LockFree>, i * 16 + 5, &Exit, &Start, &Count, &Pool);
		}

		{
			using namespace std::chrono_literals;
            auto begin = std::chrono::high_resolution_clock::now();
            Start = true;
			std::this_thread::sleep_for(3s);
		    Exit = true;
            for (int i = 0; i < ThreadNum; ++i)
            {
                WorkThreadArr[i].join();
            }
            auto end = std::chrono::high_resolution_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
            time_per_second = (size_t)((float)Count.load() / ms * 1000.0f);
		}

		Pool.ValidateData();
		std::cout << "LockFree Count: " << time_per_second << std::endl;
	}

	Count.store(0);
	Exit = false;
    Start = false;
	TPool<MemBlock, Fuko::MutexLock> MtPool(100, 30);

	{
		for (int i = 0; i < ThreadNum; ++i)
		{
			WorkThreadArr[i] = std::thread(&_PoolWorker<Fuko::MutexLock>, i * 16 + 5, &Exit, &Start, &Count, &MtPool);
		}

		{
            using namespace std::chrono_literals;
            auto begin = std::chrono::high_resolution_clock::now();
            Start = true;
            std::this_thread::sleep_for(3s);
            Exit = true;
            for (int i = 0; i < ThreadNum; ++i)
            {
                WorkThreadArr[i].join();
            }
            auto end = std::chrono::high_resolution_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
            time_per_second = (size_t)((float)Count.load() / ms * 1000.0f);
		}

		MtPool.ValidateData();
		std::cout << "Mutex Count:    " << time_per_second << std::endl;
	}



}
