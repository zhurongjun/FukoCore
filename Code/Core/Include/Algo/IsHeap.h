#pragma once
#include <CoreConfig.h>
#include <CoreType.h>

namespace Fuko::Algo::Impl
{
	template <typename RangeValueType, typename IndexType, typename ProjectionType, typename PredicateType>
	bool IsHeapInternal(RangeValueType* Heap, IndexType Num, ProjectionType Projection, PredicateType Predicate)
	{
		for (IndexType Index = 1; Index < Num; Index++)
		{
			IndexType ParentIndex = HeapGetParentIndex(Index);

			// 假设现在是小根堆，如果子节点小于父节点，则必然不是堆
			if (Predicate(std::invoke(Projection, Heap[Index]), std::invoke(Projection, Heap[ParentIndex])))
			{
				return false;
			}
		}

		return true;
	}
}

namespace Fuko::Algo
{
	/**
	 * @fn template <typename RangeType> FORCEINLINE bool IsHeap(RangeType& Range)
	 *
	 * @brief 判断是否是小根堆
	 *
	 * @param [in] Range 输入的数组
	 *
	 * @returns 是否是一个小根堆
	 */
	template <typename RangeType>
	FORCEINLINE bool IsHeap(RangeType& Range)
	{
		return Impl::IsHeapInternal(GetData(Range), GetNum(Range), FIdentityFunctor(), TLess<>());
	}

	/**
	 * @fn template <typename RangeType, typename PredicateType> FORCEINLINE bool IsHeap(RangeType& Range, PredicateType Predicate)
	 *
	 * @brief 判断是否是一个堆
	 *
	 * @param [in,out] Range	 输入的数组
	 * @param 		   Predicate 判断条件Less<>()小根堆，Greater<>()大根堆
	 *
	 * @returns 是否是一个堆
	 */
	template <typename RangeType, typename PredicateType>
	FORCEINLINE bool IsHeap(RangeType& Range, PredicateType Predicate)
	{
		return Impl::IsHeapInternal(GetData(Range), GetNum(Range), FIdentityFunctor(), std::move(Predicate));
	}

	/**
	 * @fn template <typename RangeType, typename ProjectionType> FORCEINLINE bool IsHeapBy(RangeType& Range, ProjectionType Projection)
	 *
	 * @brief 判断是否是小根堆
	 *
	 * @param [in]     Range	  输入的数组
	 * @param 		   Projection 映射函数
	 *
	 * @returns 是否是一个小根堆
	 */
	template <typename RangeType, typename ProjectionType>
	FORCEINLINE bool IsHeapBy(RangeType& Range, ProjectionType Projection)
	{
		return Impl::IsHeapInternal(GetData(Range), GetNum(Range), std::move(Projection), TLess<>());
	}

	/**
	 * @fn template <typename RangeType, typename ProjectionType, typename PredicateType> FORCEINLINE bool IsHeapBy(RangeType& Range, ProjectionType Projection, PredicateType Predicate)
	 *
	 * @brief 判断是否是一个堆
	 *
	 * @param [in]	   Range	  输入的数组
	 * @param 		   Projection 映射函数
	 * @param 		   Predicate  判断条件Less<>()小根堆，Greater<>()大根堆
	 *
	 * @returns 是否是一个堆
	 */
	template <typename RangeType, typename ProjectionType, typename PredicateType>
	FORCEINLINE bool IsHeapBy(RangeType& Range, ProjectionType Projection, PredicateType Predicate)
	{
		return Impl::IsHeapInternal(GetData(Range), GetNum(Range), std::move(Projection), std::move(Predicate));
	}
}