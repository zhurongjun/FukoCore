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
	
	std::filesystem::path Path(L"c:/");
	std::cout << std::filesystem::exists(Path) << std::endl;

	system("pause");
	return 0;
}