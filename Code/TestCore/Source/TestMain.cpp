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
#include <TestPool.h>
#include <JobSystem/Node.hpp>

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
	//TestPool();
	//TestName();
	TestString();

	Fuko::Job::JobNode A, B, C, D, E;

	A.Precede(&B, &C, &D, &E);
	E.Depend(&A, &B, &C, &D);
	A.Bind([]() {std::cout << "shit" << std::endl; });

	system("pause");
	return 0;
}