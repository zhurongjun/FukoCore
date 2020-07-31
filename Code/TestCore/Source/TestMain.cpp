#include <stdlib.h>
#include <iostream>
#include <vector>
#include <thread>
#include <atomic>

#include <TestArray.h>
#include <TestBitArray.h>
#include <TestSparseArray.h>
#include <TestSet.h>
#include <TestMap.h>
#include <TestRingQueue.h>
#include <TestDelegate.h>
#include <TestName.h>
#include <TestString.h>
#include <future>
#include <Async/ThreadPool.h>
#include <JobSystem/Queue.h>

int n = 0;

void TaskFun()
{
	for (uint64 i = 0; i < 100000'0000; ++i) 
	{
		++n;
		n /= 100;
		--n;
	}
}

void TaskFun2()
{
	for (int i = 0; i < 100000'0000; ++i)
	{
		++n;
		n /= 100;
		--n;
	}
}

int main()
{
	TestArray();
	TestBitArray();
	TestSparseArray();
	TestSet();
	TestMap();
	//TestRingQueue();

	//TestDelegate();
	//TestName();
	TestString();
	auto Begin = std::chrono::high_resolution_clock::now();
	{
		Fuko::ThreadPool Pool;
		Begin = std::chrono::high_resolution_clock::now();
		for (int i = 0; i < 20; ++i)
		{
			Pool.ExecTask(&TaskFun);
		}
	}
	auto End = std::chrono::high_resolution_clock::now();

	std::cout << "Time: " << std::chrono::duration<double, std::milli>(End - Begin).count() << "ms" << std::endl;

	Fuko::Job::JobQueue<void*> a;

	system("pause");
	return 0;
}