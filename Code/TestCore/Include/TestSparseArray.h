#pragma once
#include <Containers/SparseArray.h>
#include <iostream>

using Fuko::TSparseArray;
void TestSparseArray()
{
	TSparseArray<int> A;
	A.Add(10);
	A.Add(100);
	check(A.Num() == 2);
	TSparseArray<int> B(A);
	check(B.Num() == 2);
	TSparseArray<int> C(std::move(B));
	check(B.Num() == 0);
	check(C.Num() == 2);
	B = A;
	check(A.Num() == 2);
	check(B.Num() == 2);
	A.Empty();
	B.Empty();
	A = std::move(C);
	check(C.Num() == 0);
	check(A.Num() == 2);
	A.Empty(100);
	check(A.Num() == 0);
	check(A.Max() >= 100);
	
	for (int i = 0; i < 10; ++i)
	{
		A.Add(i);
	}
	check(A.IsCompact());
	A.RemoveAt(0, 5);
	check(!A.IsCompact());
	check(A.Num() == 5);
	check(A.GetMaxIndex() == 10);
	for (int i = 0; i < 5; ++i)
	{
		A.Add(i);
	}
	check(A.Num() == 10);
	check(A.GetMaxIndex() == 10);
	check(A.IsCompact());
	A.Reset(5);
	check(A.Max() == 100);
	check(A.Num() == 0);
	A.Empty();
	A.Add(1);
	A.Reset(100);
	check(A.Max() == 100);
	for (int i = 0; i < 99; ++i)
	{
		A.Add(i + 1);
	}
	for (int i = 0; i < 99; ++i)
	{
		if (!((i + 1) % 10) == 0)
		{
			A.RemoveAt(i);
		}
	}
	check(A.Num() == 9);
	check(A.GetMaxIndex() == 99);
	A.Shrink();
	check(A.GetMaxIndex() == 90);
	A.Compact();
	check(A.GetMaxIndex() == 9);
	check(A.Num() == 9);
	check(A.Max() < 50);
	check(A.IsCompact());
	for (int n : A)
	{
		check(n % 10 == 0);
	}
	A.Reset();
	for (int i = 0; i < 99; ++i)
	{
		A.Add(i);
	}
	for (int i = 0; i < 99; ++i)
	{
		if (!((i % 10) == 0))
		{
			A.RemoveAt(i);
		}
	}
	A.CompactStable();
	for (int i = 0; i < A.Num(); ++i)
	{
		check(A[i] == i * 10);
	}
	A.Reset();
	for (int i = 0; i < 100; ++i)
	{
		A.Add(100 - i);
	}
	for (int i = 0; i < 100; ++i)
	{
		if (!((A[i] % 10) == 0))
		{
			A.RemoveAt(i);
		}
	}
	A.Sort();
	for (int i = 0; i < A.Num(); ++i)
	{
		check(A[i] == (i + 1) * 10);
	}
	A.StableSort();

}