#pragma once
#include <Containers/SparseArray.h>
#include <iostream>

using Fuko::TSparseArray;
void TestSparseArray()
{
	TSparseArray<int> A;
	A.Add(10);
	A.Add(100);
	always_check(A.Num() == 2);
	TSparseArray<int> B(A);
	always_check(B.Num() == 2);
	TSparseArray<int> C(std::move(B));
	always_check(B.Num() == 0);
	always_check(C.Num() == 2);
	B = A;
	always_check(A.Num() == 2);
	always_check(B.Num() == 2);
	A.Empty();
	B.Empty();
	A = std::move(C);
	always_check(C.Num() == 0);
	always_check(A.Num() == 2);
	A.Empty(100);
	always_check(A.Num() == 0);
	always_check(A.Max() >= 100);
	
	for (int i = 0; i < 10; ++i)
	{
		A.Add(i);
	}
	always_check(A.IsCompact());
	A.RemoveAt(0, 5);
	always_check(!A.IsCompact());
	always_check(A.Num() == 5);
	always_check(A.GetMaxIndex() == 10);
	for (int i = 0; i < 5; ++i)
	{
		A.Add(i);
	}
	always_check(A.Num() == 10);
	always_check(A.GetMaxIndex() == 10);
	always_check(A.IsCompact());
	A.Reset(5);
	always_check(A.Max() == 100);
	always_check(A.Num() == 0);
	A.Empty();
	A.Add(1);
	A.Reset(100);
	always_check(A.Max() == 100);
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
	always_check(A.Num() == 9);
	always_check(A.GetMaxIndex() == 99);
	A.Shrink();
	always_check(A.GetMaxIndex() == 90);
	A.Compact();
	always_check(A.GetMaxIndex() == 9);
	always_check(A.Num() == 9);
	always_check(A.Max() < 50);
	always_check(A.IsCompact());
	for (int n : A)
	{
		always_check(n % 10 == 0);
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
		always_check(A[i] == i * 10);
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
		always_check(A[i] == (i + 1) * 10);
	}
	A.StableSort();

}