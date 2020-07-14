#pragma once
#include <Containers/RingQueue.h>

using Fuko::TRingQueue;

struct SlowObj
{
	int		Val;

	operator int() { return Val; }

	SlowObj() : Val(-1) {}

	SlowObj(SlowObj&&) = default;
	SlowObj& operator=(SlowObj&&) = default;

	SlowObj(int InVal)
	{
		for (int i = 0; i < 1000; ++i)
		{
			InVal += 10;
			InVal -= 10;
		}
		Val = InVal;
	}

	~SlowObj()
	{
		for (int i = 0; i < 1000; ++i)
		{
			Val += 10;
			Val -= 10;
		}
		Val = 0;
	}
};

void TestRingQueue()
{
	int a = 0;
	// no lock queue 
	{
		TRingQueue<int> A;
		always_check(A.Tail() == nullptr);
		always_check(A.Head() == nullptr);
		A.Enqueue(10);
		always_check(A.Num() == 1);
		always_check(*A.Tail() == 10);
		always_check(*A.Head() == 10);
		A.Enqueue(10);
		A.Dequeue();
		A.Dequeue();

		for (int i = 0; i < 4; i++)
		{
			A.Enqueue(i);
		}
		A.Normalize();
		for (int i = 0; i < 4; i++)
		{
			always_check(A.GetData()[i] == i);
		}
		TRingQueue<int> B = A;
		for (int i = 0; i < 4; i++)
		{
			always_check(B.GetData()[i] == i);
		}
		TRingQueue<int> C = std::move(B);
		always_check(B.GetData() == nullptr);
		always_check(B.Num() == 0);
		always_check(B.Max() == 0);
		A = std::move(C);
		always_check(C.GetData() == nullptr);
		always_check(C.Num() == 0);
		always_check(C.Max() == 0);
		A.Reset();
		always_check(A.GetData() != nullptr);
		always_check(A.Num() == 0);
		for (int i = 0; i < 10; ++i)
		{
			A.Enqueue(i);
		}
		A.Reserve(100);
		always_check(A.Max() == 100);
		for (int i = 10; i < 100; ++i)
		{
			A.Enqueue(i);
		}
		always_check(A.Max() == 100);
		always_check(A.Num() == 100);
		for (int i = 0; i < 100; i++)
		{
			always_check(A.GetData()[i] == i);
		}
	}

	// lock queue
	{
		TArray<std::atomic<int>>		CountArray;
		TRingQueue<SlowObj, Fuko::TSpinLock<>> SpinLockQueue(64);
		TRingQueue<SlowObj, Fuko::MutexLock> MutexLockQueue(64);

		static constexpr int ThreadCount = 10;
		static constexpr int LoopCount = 10'0000;
		std::thread		ThreadArr[ThreadCount * 2];
		CountArray.Reserve(LoopCount);
		CountArray.SetNumZeroed(LoopCount);

		auto EnqueueThread = [&](const auto& Queue) 
		{
			for (int i = 0; i < LoopCount; ++i) Queue.get().Enqueue(i);
		};

		auto DequeueThread = [&](const auto& Queue)
		{
			for (int i = 0; i < LoopCount; ++i)
			{
				SlowObj n;
				Queue.get().Dequeue(n);
				++CountArray[n];
			}
		};

		// test spin lock
		auto Begin = std::chrono::system_clock::now();
		for (int i = 0; i < ThreadCount; ++i)
		{
			ThreadArr[i] = std::thread(EnqueueThread, std::ref(SpinLockQueue));
			ThreadArr[ThreadCount + i] = std::thread(DequeueThread, std::ref(SpinLockQueue));
		}
		for (int i = 0; i < ThreadCount * 2; ++i)
		{
			ThreadArr[i].join();
		}
		auto End = std::chrono::system_clock::now();
		for (int i = 0; i < LoopCount; ++i)
		{
			always_check(CountArray[i] == ThreadCount);
			CountArray[i] = 0;
		}
		std::cout << "Spin lock cost time : " << std::chrono::duration<double, std::milli>(End - Begin).count() << " ms" << std::endl;

		// test mutex lock
		Begin = std::chrono::system_clock::now();
		for (int i = 0; i < ThreadCount; ++i)
		{
			ThreadArr[i] = std::thread(EnqueueThread, std::ref(MutexLockQueue));
			ThreadArr[ThreadCount + i] = std::thread(DequeueThread, std::ref(MutexLockQueue));
		}
		for (int i = 0; i < ThreadCount * 2; ++i)
		{
			ThreadArr[i].join();
		}
		End = std::chrono::system_clock::now();
		for (int i = 0; i < LoopCount; ++i)
		{
			always_check(CountArray[i] == ThreadCount);
			CountArray[i] = 0;
		}
		std::cout << "Mutex lock cost time : " << std::chrono::duration<double, std::milli>(End - Begin).count() << " ms" << std::endl;
	}
}

