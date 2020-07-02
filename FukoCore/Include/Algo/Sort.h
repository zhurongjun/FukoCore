#pragma once
#include "CoreConfig.h"
#include "CoreType.h"
#include "Templates/Functor.h"

// Binary heap
namespace Fuko::Algo::Impl
{
	/**
	 * @fn FORCEINLINE int32 HeapGetLeftChildIndex(int32 Index)
	 *
	 * @brief 得到堆节点的左Child所在的Index
	 * 		  
	 * @param  父节点的Index
	 *
	 * @returns 左子节点的Index
	 */
	FORCEINLINE constexpr int32 HeapGetLeftChildIndex(int32 Index)
	{
		return Index * 2 + 1;
	}

	/**
	 * @fn FORCEINLINE constexpr int32 HeapGetRightChildIndex(int32 Index)
	 *
	 * @brief 得到堆节点的右Child所在的Index
	 *
	 * @param  父节点的Index
	 *
	 * @returns 堆节点的右Child所在的Index
	 */
	FORCEINLINE constexpr int32 HeapGetRightChildIndex(int32 Index)
	{
		return HeapGetLeftChildIndex(Index) + 1;
	}

	/**
	 * @fn FORCEINLINE bool HeapIsLeaf(int32 Index, int32 Count)
	 *
	 * @brief 判断节点是否是一个叶子节点
	 *
	 * @param  节点位置
	 * @param  堆的大小
	 *
	 * @returns 是否是叶子节点
	 */
	FORCEINLINE constexpr bool HeapIsLeaf(int32 Index, int32 Count)
	{
		return HeapGetLeftChildIndex(Index) >= Count;
	}

	/**
	 * @fn FORCEINLINE constexpr int32 HeapGetParentIndex(int32 Index)
	 *
	 * @brief 得到父亲节点的位置
	 *
	 * @param  节点所处的位置
	 *
	 * @returns 父节点的位置
	 */
	FORCEINLINE constexpr int32 HeapGetParentIndex(int32 Index)
	{
		return (Index - 1) / 2;
	}

	/**
	 * @fn template <typename RangeValueType, typename ProjectionType, typename PredicateType> FORCEINLINE void HeapSiftDown(RangeValueType* Heap, int32 Index, const int32 Count, const ProjectionType& Projection, const PredicateType& Predicate)
	 *
	 * @brief 修复节点与其子节点违反堆规则的情况，这里是向下检索
	 *
	 * @param [in]	   Heap		  堆顶指针
	 * @param 		   Index	  节点的位置
	 * @param 		   Count	  堆的大小
	 * @param 		   Projection 元素的投影法则
	 * @param 		   Predicate  判断谓语，指定A是否应当在B之前
	 */
	template <typename RangeValueType, typename ProjectionType, typename PredicateType>
	FORCEINLINE void HeapSiftDown(RangeValueType* Heap, int32 Index, const int32 Count, const ProjectionType& Projection, const PredicateType& Predicate)
	{
		while (!HeapIsLeaf(Index, Count))
		{
			const int32 LeftChildIndex = HeapGetLeftChildIndex(Index);
			const int32 RightChildIndex = LeftChildIndex + 1;

			int32 MinChildIndex = LeftChildIndex;
			if (RightChildIndex < Count)
			{
				MinChildIndex = Predicate(std::invoke(Projection, Heap[LeftChildIndex]), std::invoke(Projection, Heap[RightChildIndex])) ? LeftChildIndex : RightChildIndex;
			}

			if (!Predicate(std::invoke(Projection, Heap[MinChildIndex]), std::invoke(Projection, Heap[Index])))
			{
				break;
			}

			Swap(Heap[Index], Heap[MinChildIndex]);
			Index = MinChildIndex;
		}
	}

	/**
	 * @fn template <class RangeValueType, typename ProjectionType, class PredicateType> FORCEINLINE int32 HeapSiftUp(RangeValueType* Heap, int32 RootIndex, int32 NodeIndex, const ProjectionType& Projection, const PredicateType& Predicate)
	 *
	 * @brief 修复节点与其子节点违反堆规则的情况，这里是向上检索
	 *
	 * @param [in]	   Heap		  堆顶指针
	 * @param 		   RootIndex  最多向上到哪个节点
	 * @param 		   NodeIndex  起始节点的位置
	 * @param 		   Projection 元素的投影法则
	 * @param 		   Predicate  判断谓语，指定A是否应当在B之前
	 *
	 * @returns NodeIndex的新位置
	 */
	template <class RangeValueType, typename ProjectionType, class PredicateType>
	FORCEINLINE int32 HeapSiftUp(RangeValueType* Heap, int32 RootIndex, int32 NodeIndex, const ProjectionType& Projection, const PredicateType& Predicate)
	{
		while (NodeIndex > RootIndex)
		{
			int32 ParentIndex = HeapGetParentIndex(NodeIndex);
			if (!Predicate(std::invoke(Projection, Heap[NodeIndex]), std::invoke(Projection, Heap[ParentIndex])))
			{
				break;
			}

			Swap(Heap[NodeIndex], Heap[ParentIndex]);
			NodeIndex = ParentIndex;
		}

		return NodeIndex;
	}

	/**
	 * @fn template <typename RangeValueType, typename ProjectionType, typename PredicateType> FORCEINLINE void HeapifyInternal(RangeValueType* First, int32 Num, ProjectionType Projection, PredicateType Predicate)
	 *
	 * @brief 堆化
	 *
	 * @param [in]	   First	  堆化的第一个节点
	 * @param 		   Num		  堆化的Item数量
	 * @param 		   Projection 元素的投影法则
	 * @param 		   Predicate  判断谓语，指定A是否应当在B之前
	 */
	template <typename RangeValueType, typename ProjectionType, typename PredicateType>
	FORCEINLINE void HeapifyInternal(RangeValueType* First, int32 Num, ProjectionType Projection, PredicateType Predicate)
	{
		// 除了叶子节点之外，对所有节点校准一遍 
		for (int32 Index = HeapGetParentIndex(Num - 1); Index >= 0; Index--)
		{
			HeapSiftDown(First, Index, Num, Projection, Predicate);
		}
	}

	/**
	 * @fn template <typename RangeValueType, typename ProjectionType, class PredicateType> void HeapSortInternal(RangeValueType* First, int32 Num, ProjectionType Projection, PredicateType Predicate)
	 *
	 * @brief 堆排序
	 *
	 * @param [in]	   First	  堆顶
	 * @param 		   Num		  排序的数量
	 * @param 		   Projection 元素的投影法则
	 * @param 		   Predicate  判断谓语，指定A是否应当在B之前
	 */
	template <typename RangeValueType, typename ProjectionType, class PredicateType>
	void HeapSortInternal(RangeValueType* First, int32 Num, ProjectionType Projection, PredicateType Predicate)
	{
		// 使用相反的谓语堆化，比如原来是大根堆，现在构建的是小根堆
		TReversePredicate< PredicateType > ReversePredicateWrapper(Predicate);
		HeapifyInternal(First, Num, Projection, ReversePredicateWrapper);

		// e.g，现在是小根堆，堆顶部是最小的元素
		for (int32 Index = Num - 1; Index > 0; Index--)
		{
			// 抽出顶部，放在堆底，然后排除这个元素，继续堆化，直到元素耗尽
			Swap(First[0], First[Index]);
			HeapSiftDown(First, 0, Index, Projection, ReversePredicateWrapper);
		}
	}

}

// Introspective sort
namespace Fuko::Algo::Impl
{
	/**
	 * @fn template <typename T, typename IndexType, typename ProjectionType, typename PredicateType> void IntroSortInternal(T* First, IndexType Num, ProjectionType Projection, PredicateType Predicate)
	 *
	 * @brief 内省排序，深度过大的时候使用堆排序，数据段过小的时候使用冒泡排序
	 *
	 * @param [in,out] First	  排序开始元素
	 * @param 		   Num		  排序的数字总量
	 * @param 		   Projection 值映射
	 * @param 		   Predicate  用于排序的谓语
	 */
	template <typename T, typename IndexType, typename ProjectionType, typename PredicateType>
	void IntroSortInternal(T* First, IndexType Num, ProjectionType Projection, PredicateType Predicate)
	{
		// 模拟栈
		struct FStack
		{
			T* Min;
			T* Max;
			uint32 MaxDepth;
		};

		// 元素过少
		if (Num < 2)
		{
			return;
		}

		// 构建栈，最高深度为数量的自然对数 * 2
		FStack RecursionStack[32] = { {First, First + Num - 1, (uint32)(FMath::Loge((float)Num) * 2.f)} }, Current, Inner;
		
		// 开始内省排序
		for (FStack* StackTop = RecursionStack; StackTop >= RecursionStack; --StackTop)
		{
			Current = *StackTop;	// 保存栈顶

		Loop:
			IndexType Count = (IndexType)(Current.Max - Current.Min + 1);	// 当前栈中的元素数量

			// 递归过深，使用堆排序
			if (Current.MaxDepth == 0)
			{
				HeapSortInternal(Current.Min, Count, Projection, Predicate);
				continue;
			}

			if (Count <= 8)
			{
				// 数据量太少了，直接冒泡排序
				while (Current.Max > Current.Min)
				{
					T *Max, *Item;
					for (Max = Current.Min, Item = Current.Min + 1; Item <= Current.Max; Item++)
					{
						if (std::invoke(Predicate, std::invoke(Projection, *Max), std::invoke(Projection, *Item)))
						{
							Max = Item;
						}
					}
					Swap(*Max, *Current.Max--);
				}
			}
			else
			{
				// 将中心元素交换至首部，以首部作为基准的存储位置
				Swap(Current.Min[Count / 2], Current.Min[0]);

				// Max+1以使用更加高效的前--
				Inner.Min = Current.Min;
				Inner.Max = Current.Max + 1;
				for (; ; )
				{
					// 从左向右迭代寻找 大/小 于基准的元素
					while (++Inner.Min <= Current.Max && !std::invoke(Predicate, std::invoke(Projection, *Current.Min), std::invoke(Projection, *Inner.Min)));
					// 从右向左迭代寻找 小/大 于基准的元素
					while (--Inner.Max > Current.Min && !std::invoke(Predicate, std::invoke(Projection, *Inner.Max), std::invoke(Projection, *Current.Min)));
					// 此次迭代完毕，退出
					if (Inner.Min > Inner.Max)
					{
						break;
					}
					// 交换，使得基准左侧元素总 大/小 于基准的元素
					Swap(*Inner.Min, *Inner.Max);
				}
				// 将基准重新塞回中心 
				Swap(*Current.Min, *Inner.Max);

				// 此时，准备开始递归，减少MaxDepth
				--Current.MaxDepth;

				if (Inner.Max - 1 - Current.Min >= Current.Max - Inner.Min)
				{
					// 左侧元素较多，存储左侧，使用右侧递归
					if (Current.Min + 1 < Inner.Max)
					{
						StackTop->Min = Current.Min;
						StackTop->Max = Inner.Max - 1;
						StackTop->MaxDepth = Current.MaxDepth;
						StackTop++;
					}
					// 如果右侧还有元素，就使用右侧递归
					if (Current.Max > Inner.Min)
					{
						Current.Min = Inner.Min;
						goto Loop;
					}
				}
				else
				{
					// 右侧元素较多，存储右侧，使用左侧递归
					if (Current.Max > Inner.Min)
					{
						StackTop->Min = Inner.Min;
						StackTop->Max = Current.Max;
						StackTop->MaxDepth = Current.MaxDepth;
						StackTop++;
					}
					// 如果左侧还有元素，就使用左侧递归
					if (Current.Min + 1 < Inner.Max)
					{
						Current.Max = Inner.Max - 1;
						goto Loop;
					}
				}
			}
		}
	}
}

// Introspective Sort
namespace Fuko::Algo
{
	/**
	 * @fn template <typename RangeType> FORCEINLINE void IntroSort(RangeType& Range)
	 *
	 * @brief 内省排序
	 *
	 * @param [in,out] Range	 排序的对象
	 */
	template <typename RangeType>
	FORCEINLINE void IntroSort(RangeType& Range)
	{
		::Fuko::Algo::Impl::IntroSortInternal(GetData(Range), GetNum(Range), FIdentityFunctor(), TLess<>());
	}

	/**
	 * @fn template <typename RangeType, typename PredicateType> FORCEINLINE void IntroSort(RangeType& Range, PredicateType Predicate)
	 *
	 * @brief 内省排序
	 *
	 * @param [in,out] Range	 排序的对象
	 * @param 		   Predicate 用于指定排序顺序的谓语
	 */
	template <typename RangeType, typename PredicateType>
	FORCEINLINE void IntroSort(RangeType& Range, PredicateType Predicate)
	{
		::Fuko::Algo::Impl::IntroSortInternal(GetData(Range), GetNum(Range), FIdentityFunctor(), std::move(Predicate));
	}

	/**
	 * @fn template <typename RangeType, typename ProjectionType> FORCEINLINE void IntroSortBy(RangeType& Range, ProjectionType Projection)
	 *
	 * @brief 内省排序
	 *
	 * @param [in,out] Range	  排序的对象
	 * @param 		   Projection 用于指定映射方式
	 */
	template <typename RangeType, typename ProjectionType>
	FORCEINLINE void IntroSortBy(RangeType& Range, ProjectionType Projection)
	{
		::Fuko::Algo::Impl::IntroSortInternal(GetData(Range), GetNum(Range), std::move(Projection), TLess<>());
	}

	/**
	 * @fn template <typename RangeType, typename ProjectionType, typename PredicateType> FORCEINLINE void IntroSortBy(RangeType& Range, ProjectionType Projection, PredicateType Predicate)
	 *
	 * @brief 内省排序
	 *
	 * @param [in,out] Range	  排序的对象
	 * @param 		   Projection 映射方式
	 * @param 		   Predicate  排序方式
	 */
	template <typename RangeType, typename ProjectionType, typename PredicateType>
	FORCEINLINE void IntroSortBy(RangeType& Range, ProjectionType Projection, PredicateType Predicate)
	{
		::Fuko::Algo::Impl::IntroSortInternal(GetData(Range), GetNum(Range), std::move(Projection), std::move(Predicate));
	}
}

// HeapSort
namespace Fuko::Algo
{
	template <typename RangeType>
	FORCEINLINE void HeapSort(RangeType& Range)
	{
		::Fuko::Algo::Impl::HeapSortInternal(GetData(Range), GetNum(Range), FIdentityFunctor(), TLess<>());
	}

	template <typename RangeType, typename PredicateType>
	FORCEINLINE void HeapSort(RangeType& Range, PredicateType Predicate)
	{
		::Fuko::Algo::Impl::HeapSortInternal(GetData(Range), GetNum(Range), FIdentityFunctor(), std::move(Predicate));
	}

	template <typename RangeType, typename ProjectionType>
	FORCEINLINE void HeapSortBy(RangeType& Range, ProjectionType Projection)
	{
		::Fuko::Algo::Impl::HeapSortInternal(GetData(Range), GetNum(Range), Projection, TLess<>());
	}

	template <typename RangeType, typename ProjectionType, typename PredicateType>
	FORCEINLINE void HeapSortBy(RangeType& Range, ProjectionType Projection, PredicateType Predicate)
	{
		::Fuko::Algo::Impl::HeapSortInternal(GetData(Range), GetNum(Range), std::move(Projection), std::move(Predicate));
	}
}

// Heapify
namespace Fuko::Algo
{
	template <typename RangeType>
	FORCEINLINE void Heapify(RangeType& Range)
	{
		::Fuko::Algo::Impl::HeapifyInternal(GetData(Range), GetNum(Range), FIdentityFunctor(), TLess<>());
	}

	template <typename RangeType, typename PredicateType>
	FORCEINLINE void Heapify(RangeType& Range, PredicateType Predicate)
	{
		::Fuko::Algo::Impl::HeapifyInternal(GetData(Range), GetNum(Range), FIdentityFunctor(), Predicate);
	}

	template <typename RangeType, typename ProjectionType, typename PredicateType>
	FORCEINLINE void HeapifyBy(RangeType& Range, ProjectionType Projection)
	{
		::Fuko::Algo::Impl::HeapifyInternal(GetData(Range), GetNum(Range), Projection, TLess<>());
	}

	template <typename RangeType, typename ProjectionType, typename PredicateType>
	FORCEINLINE void HeapifyBy(RangeType& Range, ProjectionType Projection, PredicateType Predicate)
	{
		::Fuko::Algo::Impl::HeapifyInternal(GetData(Range), GetNum(Range), Projection, Predicate);
	}
}
