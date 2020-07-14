#pragma once
#include <Containers/RingQueue.h>

using Fuko::TRingQueue;

void TestRingQueue()
{
	int a = 0;
	// no lock queue 
	{
		TRingQueue<int> A;
		always_check(A.Tail() == nullptr);
		always_check(A.Head() == nullptr);
		A.Enqueue(10);
		always_check(A.Num() == 1);
		always_check(*A.Tail() == 10);
		always_check(*A.Head() == 10);
		A.Enqueue(10);
		A.Dequeue();
		A.Dequeue();

		for (int i = 0; i < 4; i++)
		{
			A.Enqueue(i);
		}
		A.Normalize();
		for (int i = 0; i < 4; i++)
		{
			always_check(A.GetData()[i] == i);
		}
		TRingQueue<int> B = A;
		for (int i = 0; i < 4; i++)
		{
			always_check(B.GetData()[i] == i);
		}
		TRingQueue<int> C = std::move(B);
		always_check(B.GetData() == nullptr);
		always_check(B.Num() == 0);
		always_check(B.Max() == 0);
		A = std::move(C);
		always_check(C.GetData() == nullptr);
		always_check(C.Num() == 0);
		always_check(C.Max() == 0);
		A.Reset();
		always_check(A.GetData() != nullptr);
		always_check(A.Num() == 0);
		for (int i = 0; i < 10; ++i)
		{
			A.Enqueue(i);
		}
		A.Reserve(100);
		always_check(A.Max() == 100);
		for (int i = 10; i < 100; ++i)
		{
			A.Enqueue(i);
		}
		always_check(A.Max() == 100);
		always_check(A.Num() == 100);
		for (int i = 0; i < 100; i++)
		{
			always_check(A.GetData()[i] == i);
		}
	}
}

