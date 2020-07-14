#pragma once
#include <Containers/Set.h>

using Fuko::TSet;
void TestSet()
{
	{
		TArray<int> Arr = { 1,2,3,4,5 };
		TSet<int> SA(Arr);
		always_check(SA.Num() == 5);
		always_check(SA.Max() == 5);
		always_check(Arr.Num() == 5);
		always_check(Arr.Max() == 5);
		TSet<int> SB(std::move(Arr));
		always_check(SB.Num() == 5);
		always_check(SB.Max() == 5);
		always_check(Arr.Num() == 0);
		always_check(Arr.Max() == 5);
	}
	TSet<int> A;
	TSet<int> B = { 1,2,3,4,5 };
	always_check(B.Num() == 5);
	always_check(B.Max() == 5);
	TSet<int> C(B);
	always_check(B.Num() == 5);
	always_check(B.Max() == 5);
	always_check(C.Num() == 5);
	always_check(C.Max() == 5);
	TSet<int> D(std::move(C));
	always_check(C.Num() == 0);
	always_check(C.Max() == 0);
	always_check(D.Num() == 5);
	always_check(D.Max() == 5);
	C = D;
	always_check(C.Num() == 5);
	always_check(C.Max() == 5);
	A = std::move(D);
	always_check(D.Num() == 0);
	always_check(D.Max() == 0);
	always_check(A.Num() == 5);
	always_check(A.Max() == 5);
	B.Reset();
	always_check(B.Num() == 0);
	always_check(B.Max() != 0);
	A.Empty();
	B.Empty();
	C.Empty();
	D.Empty();
	always_check(A.Num() == 0);
	always_check(A.Max() == 0);
	A.Reserve(100);
	always_check(A.Max() == 100);
	for (int i = 0; i < 100; ++i)
	{
		A.Add(i);
		if (i % 3 == 0)
			B.Add(i);
		if (i % 5 == 0)
			C.Add(i);
		if (i % 15 == 0)
			D.Add(i);
	}
	for (int i = 0; i < 100; ++i)
	{
		always_check(A[i] == i);
	}
	for (int i = 0; i < 50; ++i)
	{
		A.Remove(50 + i);
	}
	always_check(A.Num() == 50);
	always_check(A.GetMaxIndex() == 100);
	always_check(A.Max() == 100);
	A.Shrink();
	always_check(A.GetMaxIndex() == 50);
	A.Reset(100);
	for (int i = 0; i < 100; ++i)
	{
		A.Add(i);
	}
	A.Sort();
	A.StableSort();

	always_check(A.Includes(B));
	always_check(A.Includes(C));
	always_check(B.Includes(D));
	auto AAndB = A.Intersect(B);
	auto AOrB = A.Union(B);
	always_check(AOrB == A);
	always_check(AAndB == B);
	A.Difference(C);
	auto Arr = A.Array();

	for (int i = 0; i < 100; ++i)
	{
		always_check(Arr[i] == i);
	}

	A += Arr;
	A += B;
	A += std::move(C); 
	A += std::move(Arr);
	A += {1, 2, 3, 4};
}
