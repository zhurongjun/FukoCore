#pragma once
#include <CoreConfig.h>
#include <CoreType.h>
#include "Heap.h"

namespace Fuko::Algo
{
	template <typename T, typename TSize, class TPred>
	void HeapSort(T* First, TSize Num, TPred&& Pred)
	{
		auto ReversePred = [&](const T& A, const T& B)->bool { return !Pred(A, B); };

		// 使用相反的谓语堆化
		Heapify(First, Num, ReversePred);

		for (TSize Index = Num - 1; Index > 0; Index--)
		{
			// 抽出顶部，放在堆底，然后排除这个元素，继续堆化，直到元素耗尽
			Swap(First[0], First[Index]);
			HeapSiftDown(First, (TSize)0, Index, ReversePred);
		}
	}
}