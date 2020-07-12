#pragma once
#include <Containers/Map.h>

using Fuko::TMap;
void TestMap()
{
	TMap<int, int> a;
	for (int i = 0; i < 1000'0000; ++i)
	{
		a[i] = i * 15;
	}
	a.Shrink();
	check(a.Max() == 1000'0000);
	check(a.Num() == 1000'0000);
	a.KeySort(TGreater<>());
	for (auto It = a.begin(); It; ++It)
	{
		check(It.Key() == (999'9999 - It.GetIndex()));
	}
	a.ValueSort(TLess<>());
	for (int i = 0; i < 1000'0000; ++i)
	{
		check(a[i] == i * 15);
	}

	int b = 10;
}