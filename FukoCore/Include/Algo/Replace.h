#pragma once
#include "CoreConfig.h"
#include "CoreType.h"

namespace Fuko::Algo
{
	/**
	 * @fn template <typename RangeType, typename ValueType> FORCEINLINE void Replace(RangeType&& Range, const ValueType& InOld, const ValueType& InNew)
	 *
	 * @brief 替换元素
	 *
	 * @param [in]     Range 数组
	 * @param 		   InOld 旧元素
	 * @param 		   InNew 新元素
	 */
	template <typename RangeType, typename ValueType>
	FORCEINLINE void Replace(RangeType&& Range, const ValueType& InOld, const ValueType& InNew)
	{
		for (auto& Elem : std::forward<RangeType>(Range))
		{
			if (Elem == InOld)
			{
				Elem = InNew;
			}
		}
	}

	/**
	 * @fn template <typename RangeType, typename ValueType, typename PredicateType> FORCEINLINE void ReplaceIf(RangeType&& Range, PredicateType InPred, const ValueType& InNew)
	 *
	 * @brief 替换元素
	 *
	 * @param [in]     Range  数组
	 * @param 		   InPred 谓语
	 * @param 		   InNew  新元素
	 */
	template <typename RangeType, typename ValueType, typename PredicateType>
	FORCEINLINE void ReplaceIf(RangeType&& Range, PredicateType InPred, const ValueType& InNew)
	{
		for (auto& Elem : std::forward<RangeType>(Range))
		{
			if (std::invoke(InPred, Elem))
			{
				Elem = InNew;
			}
		}
	}
}