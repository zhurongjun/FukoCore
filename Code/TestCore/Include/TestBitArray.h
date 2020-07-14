#pragma once
#include <Containers/BitArray.h>

using Fuko::TBitArray;

void TestBitArray()
{
	TBitArray A;
	TBitArray B(true, 2);
	always_check(B.Num() == 2);
	always_check(B.Max() == 32);
	for (int i = 0; i < 2; ++i)
	{
		always_check(B[i] == true);
	}
	TBitArray C(B);
	always_check(C.Num() == 2);
	always_check(C.Max() == 32);
	for (int i = 0; i < 2; ++i)
	{
		always_check(C[i] == true);
	}
	C.Init(false, 10);
	always_check(C.Num() == 10);
	always_check(C.Max() == 32);
	for (int i = 0; i < 10; ++i)
	{
		always_check(C[i] == false);
	}
	TBitArray D(std::move(C));
	always_check(D.Num() == 10);
	always_check(D.Max() == 32);
	always_check(C.Num() == 0);
	always_check(C.Max() == 0);
	for (int i = 0; i < 10; ++i)
	{
		always_check(D[i] == false);
	}
	C = std::move(D);
	always_check(C.Num() == 10);
	always_check(C.Max() == 32);
	always_check(D.Num() == 0);
	always_check(D.Max() == 0);
	for (int i = 0; i < 10; ++i)
	{
		always_check(C[i] == false);
	}
	D = C;
	always_check(D.Num() == 10);
	always_check(D.Max() == 32);
	always_check(C.Num() == 10);
	always_check(C.Max() == 32);
	for (int i = 0; i < 10; ++i)
	{
		always_check(D[i] == false);
	}
	always_check(D == C);
	always_check(D != A);
	A.Init(false, 10);
	always_check(D == A);
	A.Init(true, 10);
	always_check(D != A);
	always_check(B < A);
	always_check(D < A);
	always_check(!(D < C));

	A.Empty();
	always_check(A.Num() == 0);
	always_check(A.Max() == 0);
	B.Empty(10);
	always_check(B.Num() == 0);
	always_check(B.Max() == 32);
	D.Reset();
	always_check(D.Num() == 0);
	always_check(D.Max() == 32);
	D.Empty();
	C.Empty();
	C.Reserve(100);
	always_check(C.Num() == 0);
	always_check(C.Max() == 128);
	C.Empty();
	D.Add(false);
	always_check(D.Num() == 1);
	D.Add(false,10);
	always_check(D.Num() == 11);

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
		if (i % 2 == 0) always_check(A[i] == true);
	}
	A.RemoveAt(0);
	always_check(A.Num() == 19);
	for (int i = 0; i < 19; ++i)
	{
		if (i % 2 == 0) always_check(A[i] == false);
	}
	A.Add(true);
	for (int i = 0; i < 20; ++i)
	{
		A[i] = i % 3 == 2;
	}
	always_check(A.Find(true) == 2);
	always_check(A.FindAndSetFirstZeroBit() == 0);
	always_check(A[0] == true);
	A[0] = false;
	always_check(A.FindAndSetLastZeroBit() == 19);
	always_check(A[19] == true);
	A[19] = false;
	A.RemoveAt(0, 3);
	always_check(A.Num() == 17);
	for (int i = 0; i < 17; ++i)
	{
		always_check(A[i] == (i % 3 == 2));
	}
	A.RemoveAtSwap(0);
	always_check(A.Num() == 16);
	A.RemoveAtSwap(0, 3);
	always_check(A.Num() == 13);
	always_check(A.Contains(true));
	always_check(A.Contains(false));
	A.Init(false, 20);
	always_check(A.Contains(false));
	always_check(!A.Contains(true));
	A.Add(true);
	always_check(A.Find(true) == 20);
	A.SetRange(10, 11, false);
	always_check(!A.Contains(true));
	always_check(A[20] == false);
	A.Init(true, 100);

	A.Init(false,100);
	B.Init(false,100);
	C.Init(false,100);
	for (int i = 0; i < 100; ++i)
	{
		always_check(A[i] == false);
		always_check(B[i] == false);
		A[i] = (i % 5) == 0;
		B[i] = (i % 3) == 0;
		C[i] = (i % 15) == 0;
	}

	int count = 0;
	for (TBitArray<>::ConstSetBitIterator It(A, 10); It; ++It)
	{
		always_check(It.GetIndex() % 5 == 0);
		++count;
	}
	always_check(count == 18);
	count = 0;
	for (TBitArray<>::ConstDualSetBitIterator It(A, B); It; ++It)
	{
		always_check(It.GetIndex() % 15 == 0);
		++count;
	}
	always_check(count == 7);

}