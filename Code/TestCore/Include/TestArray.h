#pragma once
#include <Containers/Array.h>
#include <Templates/Functor.h>
#include <Templates/Pair.h>
#include <iostream>

using Fuko::TArray;
using Fuko::TPair;
struct CountType
{
	static int Construct;
	static int CopyConstruct;
	static int MoveConstruct;

	static int Assign;
	static int MoveAssign;

	static int Destruct;
	static void Reset() { Construct = CopyConstruct = MoveConstruct = Assign = MoveAssign = Destruct = 0; }

	CountType() { ++Construct; }
	CountType(const CountType&) { ++CopyConstruct; }
	CountType(CountType&&) { ++MoveConstruct; }
	CountType& operator=(const CountType&) { ++Assign; }
	CountType& operator=(CountType&&) { ++MoveAssign; }
	~CountType() { ++Destruct; }
};
int CountType::Construct = 0;
int CountType::CopyConstruct = 0;
int CountType::MoveConstruct = 0;
int CountType::Assign = 0;
int CountType::MoveAssign = 0;
int CountType::Destruct = 0;

void TestArray()
{
	// construct test
	{
		int Arr[] = { 1,2,3,4,5 };
		TArray<int> TestRaw(Arr, sizeof(Arr) / sizeof(int));
		always_check(TestRaw.Num() == 5);
		always_check(TestRaw.Max() == 5);
		for (int i = 1; i <= 5; ++i)
		{
			always_check(TestRaw[i - 1] == i);
		}

		TArray<int> TestInitializeList({ 1,2,3,4,5 });
		always_check(TestInitializeList.Num() == 5);
		always_check(TestInitializeList.Max() == 5);
		for (int i = 1; i <= 5; ++i)
		{
			always_check(TestInitializeList[i - 1] == i);
		}
	}

	// copy construct test
	{
		TArray<char> ChArr = { 1,2,3,4,5 };
		TArray<int>  NumArr = { 1,2,3,4,5 };

		TArray<int> TestCopy(NumArr);
		always_check(TestCopy.Num() == 5);
		always_check(TestCopy.Max() == 5);
		
		TArray<int> TestCopyWithShink(NumArr,10);
		always_check(TestCopyWithShink.Num() == 5);
		always_check(TestCopyWithShink.Max() == 15);
		
		TArray<int> TestOtherCopy(ChArr);
		always_check(TestOtherCopy.Num() == 5);
		always_check(TestOtherCopy.Max() == 5);

		CountType::Reset();
		TArray<CountType> Other;
		Other.SetNum(10);
		TArray<CountType> TestCopyCount(Other);
		always_check(CountType::CopyConstruct == 10);
		always_check(CountType::Construct == 10);
		always_check(CountType::Destruct == 0);
		always_check(CountType::MoveConstruct == 0);
		always_check(CountType::Assign == 0);
	}

	// assign test
	{
		TArray<CountType> Arr;
		Arr.SetNum(10);
		
		CountType::Reset();
		TArray<CountType> TestAssign;
		TestAssign = { {},{},{},{},{} };
		always_check(CountType::CopyConstruct == 5);

		CountType::Reset();
		TestAssign = Arr;
		always_check(CountType::Destruct == 5);
		always_check(CountType::CopyConstruct == 10);
	}

	// move construct
	{
		TArray<CountType>	Arr;
		Arr.SetNum(10);

		CountType::Reset();
		CountType* LastData = Arr.GetData();

		TArray<CountType> TestMove(std::move(Arr));
		always_check(CountType::MoveConstruct == 0);
		always_check(Arr.Max() == 0 && Arr.Num() == 0);
		always_check(Arr.GetData() == nullptr);
		always_check(LastData == TestMove.GetData());
	}

	// move assign
	{
		TArray<CountType> Arr;
		Arr.SetNum(10);
		TArray<CountType> TestMove;
		TestMove.SetNum(15);
		
		CountType::Reset();
		CountType* LastData = Arr.GetData();
		TestMove = std::move(Arr);

		always_check(CountType::Destruct == 15);
		always_check(CountType::MoveConstruct == 0);
		always_check(Arr.Max() == 0 && Arr.Num() == 0);
		always_check(Arr.GetData() == nullptr);
		always_check(LastData == TestMove.GetData());
	}

	// get information test
	{
		TArray<int> Arr;
		
		Arr.Reserve(100);
		int* LastData = Arr.GetData();
		Arr.SetNum(10);
		always_check(LastData == Arr.GetData());

		always_check(Arr.Num() == 10);
		always_check(Arr.Max() == 100);
		always_check(Arr.Slack() == 90);
	}

	// compare & get & pop & push & shrink & reset & empty & setnum always_check 
	{
		TArray<int> A = { 1,2,3,4,5,6,7,8,9,10 };
		TArray<int> B = A;
		TArray<int> C = B;
		C.SetNum(5);
		TArray<int> D = C;
		for (int i = 0; i < 5; i++)
		{
			D.Emplace(i + 6);
		}
		always_check(A == B);
		always_check(A != C);
		always_check(A == D);
		always_check(B != C);
		always_check(B == D);

		for (int i = 0; i < D.Num(); ++i)
		{
			always_check(D[i] == i + 1);
			always_check(D.Last(i) == 10 - i);
		}
		always_check(D.Last() == 10);
		for (int i = 0; i < 5; i++)
		{
			always_check(D.Pop() == 10 - i);
		}
		for (int i = 0; i < 5; ++i)
		{
			D.Push(i + 6);
		}
		always_check(A == D);
		D.Shrink();
		always_check(D.Num() == D.Max());

		D.Reset(2);
		always_check(D.Num() == 0);
		always_check(D.Max() == 10);
		A.Reset(100);
		always_check(A.Max() == 100);
		D.Empty();
		always_check(D.Max() == 0);
		always_check(D.GetData() == nullptr);
		auto lastData = A.GetData();
		A.Empty(100);
		always_check(lastData == A.GetData());
		A.SetNum(20);
		A.Empty(10);
		always_check(A.Num() == 0);
		always_check(A.Max() == 10);
		B.Reset();
		B.SetNumZeroed(100);
		for (int i = 0; i < B.Num(); ++i)
		{
			always_check(B[i] == 0);
		}
	}

	// find, indexof, filter, contain
	{
		TArray<int> Arr = { 1,2,3,4,5,4,3,2,1 };

		always_check(Arr.Find(1) == &Arr[0]);
		always_check(Arr.FindLast(1) == &Arr.Last());
		always_check(Arr.Find(100) == nullptr);
		always_check(Arr.FindLast(100) == nullptr);
		always_check(Arr.FindBy([](int n)->bool {return n == 1; }) == &Arr[0]);
		always_check(Arr.FindLastBy([](int n)->bool {return n == 1; }) == &Arr.Last());
		always_check(Arr.FindBy([](int n)->bool {return n > 5; }) == nullptr);
		always_check(Arr.FindLastBy([](int n)->bool {return n > 5; }) == nullptr);

		auto out = Arr.FilterBy([](int n)->bool {return n < 4; });
		always_check(out.Num() == 6);
		for (int i : out)
		{
			always_check(i < 4);
		}
		always_check(out.Contains(1));
		always_check(out.ContainsBy([](int n)->bool {return n >= 4; }) == false);
	}

	// insert, remove, emplace, add, append, init
	{
		TArray<int> A = { 1,2,3,8,9 };
		TArray<int> B = { 10 };
		A.Insert({ 5,6,7 }, 3);
		A.Insert(4, 3);
		A.Insert(B, 9);
		for (int i = 0; i < 10; ++i)
		{
			always_check(A[i] == i + 1);
		}
		always_check(A.Insert_GetRef(std::move(11), 10) == 11);
		for (int i = 0; i < 4; ++i)
		{
			A.Add_GetRef(12 + i);
		}
		always_check(A.Num() == 15);
		A.EmplaceAt(0, 0);
		always_check(A.AddUnique(15) == 15);
		always_check(A.Num() == 16);
		TArray<int> C = { 16,17,18,19 };
		A += std::move(C);
		always_check(C.Num() == 0);
		always_check(C.Max() == 0);
		always_check(C.GetData() == nullptr);
		for (int i = 0; i < 20; ++i)
		{
			always_check(A[i] == i);
		}
		A.RemoveAt(19);
		always_check(A.Num() == 19);
		A.RemoveAt(15, 4);
		always_check(A.Num() == 15);
		A.Add(14);
		A.Add(14);
		A.Add(14);
		always_check(A.RemoveSingleSwap(14) == 1);
		always_check(A.Num() == 17);
		always_check(A.RemoveSwap(14) == 3);
		always_check(A.Num() == 14);
		always_check(A.RemoveBy([](int n)->bool {return n == 0 || n > 10; }) == 4);
		for (int i = 0; i < 10; ++i)
		{
			always_check(A[i] == i + 1);
		}
		A += {11, 12, 13, 14, 15};
		A += TArray<int>(A);
		for (int i = 0; i < 15; ++i)
		{
			always_check(A[i] == i % 15 + 1);
		}
		A.Init(100, 10);
		always_check(A.Num() == 10);
		for (int i : A)
		{
			always_check(i == 100);
		}
	}

	// sort
	{
		TArray<int> A = { 1,2,3,4,5,6,7,8,9,10 };
		A.Sort(TGreater<>());
		for (int i = 0; i < 10; ++i)
		{
			always_check(A[i] == 10 - i);
		}
		TArray<TPair<int, int>> B;
		for (int i = 0; i < 100; ++i)
		{
			B.Add({ (100 - i) / 5, 100 - i });
		}
		A.StableSort();
		B.StableSort([](auto Lhs, auto Rhs)->bool {return Lhs.Key < Rhs.Key; });
		for (int i = 0; i < 100; ++i)
		{
			always_check(B[i].Key == (i + 1) / 5);
			if ((i + 2) % 5 != 0 && i < 99)
			{
				always_check(B[i].Value - B[i + 1].Value == 1);
			}
		}
	}

	// heap
	{
		TArray<int> A = { 1,2,3,4,5,6,7,8,9,10 };
		always_check(A.IsHeap(TLess<>()));
		A.Heapify(TGreater<>());
		always_check(A.IsHeap(TGreater<>()));
		always_check(A.HeapTop() == 10);
		int a;
		A.HeapPop(a,TGreater<>());
		always_check(a == 10);
		always_check(A.HeapTop() == 9);
		A.HeapPush(100, TGreater<>());
		always_check(A.HeapTop() == 100);
	}
}