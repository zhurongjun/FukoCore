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
#include <JobSystem/JobSystem.h>

int main()
{
// 	TestArray();
// 	TestBitArray();
// 	TestSparseArray();
// 	TestSet();
// 	TestMap();
// 	TestRingQueue();
// 	
// 	TestDelegate();
// 	TestPool();
// 	TestName();
// 	TestString();
	{
		Fuko::Job::JobExecuter Executer;
		Fuko::Job::JobBucket BucketA;
		Fuko::Job::JobBucket BucketB;

		Executer.Execute(BucketA).Sync(1);
		Executer.Execute(BucketB).Sync(1);
		Executer.WaitForAll();
	}

	system("pause");
	return 0;
}