#pragma once
#include "CoreConfig.h"
#include "CoreType.h"

namespace Fuko::Algo::Impl
{
	template <typename T, typename IndexType, typename ProjectionType, typename PredType>
	bool IsSortedBy(const T* Range, IndexType RangeSize, ProjectionType Proj, PredType Pred)
	{
		if (RangeSize == 0)
		{
			return true;
		}

		--RangeSize;

		const T* Next = Range + 1;
		for (;;)
		{
			if (RangeSize == 0)
			{
				return true;
			}

			auto&& Ref1 = Invoke(Proj, *Next);
			auto&& Ref2 = Invoke(Proj, *Range);
			if (Invoke(Pred, Ref1, Ref2))
			{
				return false;
			}

			++Range;
			++Next;
			--RangeSize;
		}
	}
}

namespace Fuko::Alog
{
	/**
	 * @fn template <typename RangeType> FORCEINLINE bool IsSorted(const RangeType& Range)
	 *
	 * @brief 是否是从小大到大排序的数组
	 *
	 * @param  Range 数组
	 *
	 * @returns 是否是排序后的数组
	 */
	template <typename RangeType>
	FORCEINLINE bool IsSorted(const RangeType& Range)
	{
		return Impl::IsSortedBy(GetData(Range), GetNum(Range), FIdentityFunctor(), TLess<>());
	}

	/**
	 * @fn template <typename RangeType, typename PredType> FORCEINLINE bool IsSorted(const RangeType& Range, PredType Pred)
	 *
	 * @brief 是否是经过排序的数组
	 *
	 * @param  Range 数组
	 * @param  Pred  排序的顺序TLess<>()从小到,TGrater<>()从大到小
	 *
	 * @returns 是否是排序后的数组
	 */
	template <typename RangeType, typename PredType>
	FORCEINLINE bool IsSorted(const RangeType& Range, PredType Pred)
	{
		return Impl::IsSortedBy(GetData(Range), GetNum(Range), FIdentityFunctor(), std::move(Pred));
	}

	/**
	 * @fn template <typename RangeType, typename ProjectionType> FORCEINLINE bool IsSortedBy(const RangeType& Range, ProjectionType Projection)
	 *
	 * @brief 是否是从小大到大排序的数组
	 *
	 * @param  Range	  数组
	 * @param  Projection 映射函数
	 *
	 * @returns 是否是排序后的数组
	 */
	template <typename RangeType, typename ProjectionType>
	FORCEINLINE bool IsSortedBy(const RangeType& Range, ProjectionType Projection)
	{
		return Impl::IsSortedBy(GetData(Range), GetNum(Range), std::move(Projection), TLess<>());
	}

	/**
	 * @fn template <typename RangeType, typename ProjectionType, typename PredType> FORCEINLINE bool IsSortedBy(const RangeType& Range, ProjectionType Projection, PredType Pred)
	 *
	 * @brief 是否是经过排序的数组
	 *
	 * @param  Range	  数组
	 * @param  Projection 映射函数
	 * @param  Pred		  排序的顺序TLess<>()从小到,TGrater<>()从大到小
	 *
	 * @returns 是否是排序后的数组
	 */
	template <typename RangeType, typename ProjectionType, typename PredType>
	FORCEINLINE bool IsSortedBy(const RangeType& Range, ProjectionType Projection, PredType Pred)
	{
		return Impl::IsSortedBy(GetData(Range), GetNum(Range), std::move(Projection), std::move(Pred));
	}
}


