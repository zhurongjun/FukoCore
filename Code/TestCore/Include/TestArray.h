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
		check(TestCopy.Num() == 5);
		check(TestCopy.Max() == 5);
		
		TArray<int> TestCopyWithShink(NumArr,10);
		check(TestCopyWithShink.Num() == 5);
		check(TestCopyWithShink.Max() == 15);
		
		TArray<int> TestOtherCopy(ChArr);
		check(TestOtherCopy.Num() == 5);
		check(TestOtherCopy.Max() == 5);

		CountType::Reset();
		TArray<CountType> Other;
		Other.SetNum(10);
		TArray<CountType> TestCopyCount(Other);
		check(CountType::CopyConstruct == 10);
		check(CountType::Construct == 10);
		check(CountType::Destruct == 0);
		check(CountType::MoveConstruct == 0);
		check(CountType::Assign == 0);
	}

	// assign test
	{
		TArray<CountType> Arr;
		Arr.SetNum(10);
		
		CountType::Reset();
		TArray<CountType> TestAssign;
		TestAssign = { {},{},{},{},{} };
		check(CountType::CopyConstruct == 5);

		CountType::Reset();
		TestAssign = Arr;
		check(CountType::Destruct == 5);
		check(CountType::CopyConstruct == 10);
	}

	// move construct
	{
		TArray<CountType>	Arr;
		Arr.SetNum(10);

		CountType::Reset();
		CountType* LastData = Arr.GetData();

		TArray<CountType> TestMove(std::move(Arr));
		check(CountType::MoveConstruct == 0);
		check(Arr.Max() == 0 && Arr.Num() == 0);
		check(Arr.GetData() == nullptr);
		check(LastData == TestMove.GetData());
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

		check(CountType::Destruct == 15);
		check(CountType::MoveConstruct == 0);
		check(Arr.Max() == 0 && Arr.Num() == 0);
		check(Arr.GetData() == nullptr);
		check(LastData == TestMove.GetData());
	}

	// get information test
	{
		TArray<int> Arr;
		
		Arr.Reserve(100);
		int* LastData = Arr.GetData();
		Arr.SetNum(10);
		check(LastData == Arr.GetData());

		check(Arr.Num() == 10);
		check(Arr.Max() == 100);
		check(Arr.Slack() == 90);
	}

	// compare & get & pop & push & shrink & reset & empty & setnum check 
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
		check(A == B);
		check(A != C);
		check(A == D);
		check(B != C);
		check(B == D);

		for (int i = 0; i < D.Num(); ++i)
		{
			check(D[i] == i + 1);
			check(D.Last(i) == 10 - i);
		}
		check(D.Last() == 10);
		for (int i = 0; i < 5; i++)
		{
			check(D.Pop() == 10 - i);
		}
		for (int i = 0; i < 5; ++i)
		{
			D.Push(i + 6);
		}
		check(A == D);
		D.Shrink();
		check(D.Num() == D.Max());

		D.Reset(2);
		check(D.Num() == 0);
		check(D.Max() == 10);
		A.Reset(100);
		check(A.Max() == 100);
		D.Empty();
		check(D.Max() == 0);
		check(D.GetData() == nullptr);
		auto lastData = A.GetData();
		A.Empty(100);
		check(lastData == A.GetData());
		A.SetNum(20);
		A.Empty(10);
		check(A.Num() == 0);
		check(A.Max() == 10);
		B.Reset();
		B.SetNumZeroed(100);
		for (int i = 0; i < B.Num(); ++i)
		{
			check(B[i] == 0);
		}
	}

	// find, indexof, filter, contain
	{
		TArray<int> Arr = { 1,2,3,4,5,4,3,2,1 };

		check(Arr.Find(1) == &Arr[0]);
		check(Arr.FindLast(1) == &Arr.Last());
		check(Arr.Find(100) == nullptr);
		check(Arr.FindLast(100) == nullptr);
		check(Arr.FindBy([](int n)->bool {return n == 1; }) == &Arr[0]);
		check(Arr.FindLastBy([](int n)->bool {return n == 1; }) == &Arr.Last());
		check(Arr.FindBy([](int n)->bool {return n > 5; }) == nullptr);
		check(Arr.FindLastBy([](int n)->bool {return n > 5; }) == nullptr);

		auto out = Arr.FilterBy([](int n)->bool {return n < 4; });
		check(out.Num() == 6);
		for (int i : out)
		{
			check(i < 4);
		}
		check(out.Contains(1));
		check(out.ContainsBy([](int n)->bool {return n >= 4; }) == false);
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
			check(A[i] == i + 1);
		}
		check(A.Insert_GetRef(std::move(11), 10) == 11);
		for (int i = 0; i < 4; ++i)
		{
			A.Add_GetRef(12 + i);
		}
		check(A.Num() == 15);
		A.EmplaceAt(0, 0);
		check(A.AddUnique(15) == 15);
		check(A.Num() == 16);
		TArray<int> C = { 16,17,18,19 };
		A += std::move(C);
		check(C.Num() == 0);
		check(C.Max() == 0);
		check(C.GetData() == nullptr);
		for (int i = 0; i < 20; ++i)
		{
			check(A[i] == i);
		}
		A.RemoveAt(19);
		check(A.Num() == 19);
		A.RemoveAt(15, 4);
		check(A.Num() == 15);
		A.Add(14);
		A.Add(14);
		A.Add(14);
		check(A.RemoveSingleSwap(14) == 1);
		check(A.Num() == 17);
		check(A.RemoveSwap(14) == 3);
		check(A.Num() == 14);
		check(A.RemoveBy([](int n)->bool {return n == 0 || n > 10; }) == 4);
		for (int i = 0; i < 10; ++i)
		{
			check(A[i] == i + 1);
		}
		A += {11, 12, 13, 14, 15};
		A += TArray<int>(A);
		for (int i = 0; i < 15; ++i)
		{
			check(A[i] == i % 15 + 1);
		}
		A.Init(100, 10);
		check(A.Num() == 10);
		for (int i : A)
		{
			check(i == 100);
		}
	}

	// sort
	{
		TArray<int> A = { 1,2,3,4,5,6,7,8,9,10 };
		A.Sort(TGreater<>());
		for (int i = 0; i < 10; ++i)
		{
			check(A[i] == 10 - i);
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
			check(B[i].Key == (i + 1) / 5);
			if ((i + 2) % 5 != 0 && i < 99)
			{
				check(B[i].Value - B[i + 1].Value == 1);
			}
		}
	}

	// heap
	{
		TArray<int> A = { 1,2,3,4,5,6,7,8,9,10 };
		check(A.IsHeap(TLess<>()));
		A.Heapify(TGreater<>());
		check(A.IsHeap(TGreater<>()));
		check(A.HeapTop() == 10);
		int a;
		A.HeapPop(a,TGreater<>());
		check(a == 10);
		check(A.HeapTop() == 9);
		A.HeapPush(100, TGreater<>());
		check(A.HeapTop() == 100);
	}
}