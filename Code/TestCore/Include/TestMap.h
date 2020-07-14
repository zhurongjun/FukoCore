#pragma once
#include <Containers/Map.h>

using Fuko::TMap;
void TestMap()
{
	TMap<int, int> a;
	for (int i = 0; i < 10000; ++i)
	{
		a[i] = i * 15;
	}
	a.Shrink();
	always_check(a.Max() == 10000);
	always_check(a.Num() == 10000);
	a.KeySort(TGreater<>());
	for (auto It = a.begin(); It; ++It)
	{
		always_check(It.Key() == (9999 - It.GetIndex()));
	}
	a.ValueSort(TLess<>());
	for (int i = 0; i < 10000; ++i)
	{
		always_check(a[i] == i * 15);
	}

	int b = 10;
}