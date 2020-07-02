#include <iostream>
#include "Templates/TypeTraits.h"
#include "Include/Templates/Align.h"
#include "Include/Containers/Allocators.h"
#include "Include/Math/MathUtility.h"
#include <windows.h>
#include "Include/Containers/ContainerFwd.h"
#include "Include/Containers/Array.h"
#include "Templates/UtilityTemp.h"
#include "Algo/BinarySearch.h"
#include "String/CString.h"
#include "Algo/Sort.h"
#include "Templates/Functor.h"
#include <string>
#include <vector>
#include "Algo/ForEach.h"
#include <atomic>
#include "Templates/Atomic.h"
#include "Templates/Tuple.h"
#include "Templates/Pair.h"
#include "Containers/SparseArray.h"
#include "Templates/Models.h"
#include "Containers/Set.h"
#include "Containers/Map.h"

int main()
{
	Fuko::TMap<int, float> Mapbb;
	Fuko::TArray<int> arr;
	
	Fuko::TTuple<int, int> tpa = Fuko::MakeTuple(10, 10);

	Mapbb.FindOrAdd(10, 4654546.f);
	Mapbb.FindOrAdd(100, 463545.f);
	Mapbb.FindOrAdd(222, 26454.f);

	for (auto pair : Mapbb)
	{
		std::cout << "Key: " << pair.Key << "\tValue: " << pair.Value << std::endl;
	}

	system("pause");
	return 0;
}