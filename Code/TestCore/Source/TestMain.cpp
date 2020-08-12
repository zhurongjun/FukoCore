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
#include <filesystem>
#include <CoreMinimal/SmartPtr.h>

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

	Fuko::PtrCore a(nullptr, nullptr);
	std::cout << a.TimeToDie() << std::endl;
	std::cout << a.IsValid() << std::endl;

	Fuko::UP<int> p();

	system("pause");
	return 0;
}