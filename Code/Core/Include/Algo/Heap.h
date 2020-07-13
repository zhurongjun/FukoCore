#pragma once
#include <CoreConfig.h>
#include <CoreType.h>
#include <Templates/UtilityTemp.h>

// Heap Tool
namespace Fuko::Algo
{
	// 得到左孩子 
	template<typename TSize>
	FORCEINLINE constexpr TSize HeapGetLeftChildIndex(TSize Index) { return Index * 2 + 1; }

	// 得到右孩子 
	template<typename TSize>
	FORCEINLINE constexpr TSize HeapGetRightChildIndex(TSize Index) { return HeapGetLeftChildIndex(Index) + 1; }

	// 得到父亲节点　
	template<typename TSize>
	FORCEINLINE constexpr TSize HeapGetParentIndex(TSize Index) { return Index ? (Index - 1) / 2 : 0; }

	// 是否是叶子节点 
	template<typename TSize>
	FORCEINLINE constexpr bool HeapIsLeaf(TSize Index, TSize Count) { return HeapGetLeftChildIndex(Index) >= Count; }
	
	// sift down 
	template <typename T, typename TSize, typename TPred>
	FORCEINLINE void HeapSiftDown(T* Heap, TSize Index, TSize Count, TPred&& Pred)
	{
		while (!HeapIsLeaf(Index, Count))
		{
			const TSize LeftChildIndex = HeapGetLeftChildIndex(Index);
			const TSize RightChildIndex = LeftChildIndex + 1;

			// find min child 
			TSize MinChildIndex = LeftChildIndex;
			if (RightChildIndex < Count)
			{
				MinChildIndex = Pred(Heap[LeftChildIndex], Heap[RightChildIndex]) ? LeftChildIndex : RightChildIndex;
			}

			// now element is on his location
			if (!Pred(Heap[MinChildIndex], Heap[Index])) break;

			Swap(Heap[Index], Heap[MinChildIndex]);
			Index = MinChildIndex;
		}
	}

	// sift up
	template <class T, typename TSize, class TPred>
	FORCEINLINE TSize HeapSiftUp(T* Heap, TSize RootIndex, TSize NodeIndex, TPred&& Pred)
	{
		while (NodeIndex > RootIndex)
		{
			TSize ParentIndex = HeapGetParentIndex(NodeIndex);
			
			// now element is on his location
			if (!Pred(Heap[NodeIndex], Heap[ParentIndex])) break;

			Swap(Heap[NodeIndex], Heap[ParentIndex]);
			NodeIndex = ParentIndex;
		}

		return NodeIndex;
	}

	// is heap
	template <typename T, typename TSize, typename TPred>
	bool IsHeap(T* Heap, TSize Num, TPred&& Pred)
	{
		for (TSize i = 1; i < Num; ++i)
		{
			// 假设现在是小根堆，如果子节点小于父节点，则必然不是堆
			if (Pred(Heap[i], Heap[HeapGetParentIndex(i)])) return false;
		}
		return true;
	}
}

// heapify
namespace Fuko::Algo
{
	template <typename T, typename TSize, typename TPred>
	FORCEINLINE void Heapify(T* First, TSize Num, TPred&& Pred)
	{
		if (Num <= 1) return;
		// 最根部的子堆开始堆化
		// 如果某个堆的两个子堆都是合法的堆，只需要对这个堆siftdown一次，它就也是堆了
		// 核心思想是把顶部元素下沉到合适的位置
		TSize Index = HeapGetParentIndex(Num - 1);
		while (true)
		{
			HeapSiftDown(First, Index, Num, std::forward<TPred>(Pred));
			if (Index == 0) return;
			--Index;
		}
	}
}