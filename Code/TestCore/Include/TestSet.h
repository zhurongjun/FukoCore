#pragma once
#include <Containers/Set.h>

using Fuko::TSet;
void TestSet()
{
	{
		TArray<int> Arr = { 1,2,3,4,5 };
		TSet<int> SA(Arr);
		check(SA.Num() == 5);
		check(SA.Max() == 5);
		check(Arr.Num() == 5);
		check(Arr.Max() == 5);
		TSet<int> SB(std::move(Arr));
		check(SB.Num() == 5);
		check(SB.Max() == 5);
		check(Arr.Num() == 0);
		check(Arr.Max() == 5);
	}
	TSet<int> A;
	TSet<int> B = { 1,2,3,4,5 };
	check(B.Num() == 5);
	check(B.Max() == 5);
	TSet<int> C(B);
	check(B.Num() == 5);
	check(B.Max() == 5);
	check(C.Num() == 5);
	check(C.Max() == 5);
	TSet<int> D(std::move(C));
	check(C.Num() == 0);
	check(C.Max() == 0);
	check(D.Num() == 5);
	check(D.Max() == 5);
	C = D;
	check(C.Num() == 5);
	check(C.Max() == 5);
	A = std::move(D);
	check(D.Num() == 0);
	check(D.Max() == 0);
	check(A.Num() == 5);
	check(A.Max() == 5);
	B.Reset();
	check(B.Num() == 0);
	check(B.Max() != 0);
	A.Empty();
	B.Empty();
	C.Empty();
	D.Empty();
	check(A.Num() == 0);
	check(A.Max() == 0);
	A.Reserve(100);
	check(A.Max() == 100);
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
		check(A[i] == i);
	}
	for (int i = 0; i < 50; ++i)
	{
		A.Remove(50 + i);
	}
	check(A.Num() == 50);
	check(A.GetMaxIndex() == 100);
	check(A.Max() == 100);
	A.Shrink();
	check(A.GetMaxIndex() == 50);
	A.Reset(100);
	for (int i = 0; i < 100; ++i)
	{
		A.Add(i);
	}
	A.Sort();
	A.StableSort();

	check(A.Includes(B));
	check(A.Includes(C));
	check(B.Includes(D));
	auto AAndB = A.Intersect(B);
	auto AOrB = A.Union(B);
	check(AOrB == A);
	check(AAndB == B);
	A.Difference(C);
	auto Arr = A.Array();

	for (int i = 0; i < 100; ++i)
	{
		check(Arr[i] == i);
	}

	A += Arr;
	A += B;
	A += std::move(C); 
	A += std::move(Arr);
	A += {1, 2, 3, 4};
}
