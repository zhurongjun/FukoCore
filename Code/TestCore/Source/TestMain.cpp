#include <stdlib.h>
#include <vector>
#include <TestArray.h>
#include <TestBitArray.h>
#include <TestSparseArray.h>
#include <TestSet.h>
#include <TestMap.h>
#include <Templates/Functor.h>
#include <Algo/Accumulate.h>

int main()
{
	TestArray();
	TestBitArray();
	TestSparseArray();
	TestSet();
	TestMap();


	system("pause");
	return 0;
}