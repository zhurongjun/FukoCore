#pragma once
#include <Containers/BitArray.h>

using Fuko::TBitArray;

void TestBitArray()
{
	TBitArray A;
	TBitArray B(true, 2);
	check(B.Num() == 2);
	check(B.Max() == 32);
	for (int i = 0; i < 2; ++i)
	{
		check(B[i] == true);
	}
	TBitArray C(B);
	check(C.Num() == 2);
	check(C.Max() == 32);
	for (int i = 0; i < 2; ++i)
	{
		check(C[i] == true);
	}
	C.Init(false, 10);
	check(C.Num() == 10);
	check(C.Max() == 32);
	for (int i = 0; i < 10; ++i)
	{
		check(C[i] == false);
	}
	TBitArray D(std::move(C));
	check(D.Num() == 10);
	check(D.Max() == 32);
	check(C.Num() == 0);
	check(C.Max() == 0);
	for (int i = 0; i < 10; ++i)
	{
		check(D[i] == false);
	}
	C = std::move(D);
	check(C.Num() == 10);
	check(C.Max() == 32);
	check(D.Num() == 0);
	check(D.Max() == 0);
	for (int i = 0; i < 10; ++i)
	{
		check(C[i] == false);
	}
	D = C;
	check(D.Num() == 10);
	check(D.Max() == 32);
	check(C.Num() == 10);
	check(C.Max() == 32);
	for (int i = 0; i < 10; ++i)
	{
		check(D[i] == false);
	}
	check(D == C);
	check(D != A);
	A.Init(false, 10);
	check(D == A);
	A.Init(true, 10);
	check(D != A);
	check(B < A);
	check(D < A);
	check(!(D < C));

	A.Empty();
	check(A.Num() == 0);
	check(A.Max() == 0);
	B.Empty(10);
	check(B.Num() == 0);
	check(B.Max() == 32);
	D.Reset();
	check(D.Num() == 0);
	check(D.Max() == 32);
	D.Empty();
	C.Empty();
	C.Reserve(100);
	check(C.Num() == 0);
	check(C.Max() == 128);
	C.Empty();
	D.Add(false);
	check(D.Num() == 1);
	D.Add(false,10);
	check(D.Num() == 11);

	A.Empty();
	B.Empty();
	C.Empty();
	D.Empty();

	A.Init(false, 20);
	for (int i = 0; i < 20; ++i)
	{
		if (i % 2 == 0) A[i] = true;
	}
	for (int i = 0; i < 20; ++i)
	{
		if (i % 2 == 0) check(A[i] == true);
	}
	A.RemoveAt(0);
	check(A.Num() == 19);
	for (int i = 0; i < 19; ++i)
	{
		if (i % 2 == 0) check(A[i] == false);
	}
	A.Add(true);
	for (int i = 0; i < 20; ++i)
	{
		A[i] = i % 3 == 2;
	}
	check(A.Find(true) == 2);
	check(A.FindAndSetFirstZeroBit() == 0);
	check(A[0] == true);
	A[0] = false;
	check(A.FindAndSetLastZeroBit() == 19);
	check(A[19] == true);
	A[19] = false;
	A.RemoveAt(0, 3);
	check(A.Num() == 17);
	for (int i = 0; i < 17; ++i)
	{
		check(A[i] == (i % 3 == 2));
	}
	A.RemoveAtSwap(0);
	check(A.Num() == 16);
	A.RemoveAtSwap(0, 3);
	check(A.Num() == 13);
	check(A.Contains(true));
	check(A.Contains(false));
	A.Init(false, 20);
	check(A.Contains(false));
	check(!A.Contains(true));
	A.Add(true);
	check(A.Find(true) == 20);
	A.SetRange(10, 11, false);
	check(!A.Contains(true));
	check(A[20] == false);
	A.Init(true, 100);

	A.Init(false,100);
	B.Init(false,100);
	C.Init(false,100);
	for (int i = 0; i < 100; ++i)
	{
		check(A[i] == false);
		check(B[i] == false);
		A[i] = (i % 5) == 0;
		B[i] = (i % 3) == 0;
		C[i] = (i % 15) == 0;
	}

	int count = 0;
	for (TBitArray<>::ConstSetBitIterator It(A, 10); It; ++It)
	{
		check(It.GetIndex() % 5 == 0);
		++count;
	}
	check(count == 18);
	count = 0;
	for (TBitArray<>::ConstDualSetBitIterator It(A, B); It; ++It)
	{
		check(It.GetIndex() % 15 == 0);
		++count;
	}
	check(count == 7);

}