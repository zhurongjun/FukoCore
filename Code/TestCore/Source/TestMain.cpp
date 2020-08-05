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
// #include <TestDelegate.h>
#include <TestName.h>
#include <TestString.h>
#include <TestPool.h>
#include <JobSystem/Bucket.hpp>
#include <Async/ThreadPool.h>
#include <JobSystem/Executer.hpp>

int n = 0;

void TaskFun()
{
	for (uint64_t i = 0; i < 100000; ++i)
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
	//TestPool();
	//TestName();
	//TestString();

	Fuko::Job::JobExecuter Exec;
	Fuko::Job::JobBucket BucketA;
	Fuko::Job::JobBucket BucketB;

	for (int i = 0; i < 100000; ++i)
	{
		BucketA.Emplace(&TaskFun);
		BucketB.Emplace(&TaskFun);
		BucketB.Emplace(&TaskFun);
	}

	auto Begin = std::chrono::high_resolution_clock::now();
	Exec.Execute(&BucketA);
	Exec.Execute(&BucketB);
	Exec.Execute(&BucketB);
	Exec.WaitForAll();
	auto End = std::chrono::high_resolution_clock::now();
	std::cout << "JobSystemTime: " << std::chrono::duration<float, std::milli>(End - Begin).count() << std::endl;

	system("pause");
	return 0;
}