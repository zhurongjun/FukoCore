#pragma once
#include "CoreConfig.h"
#include "CoreType.h"

namespace Fuko::Algo
{
	/**
	 * @fn template <typename RangeType> FORCEINLINE bool AllOf(const RangeType& Range)
	 *
	 * @brief 判断是否所有的数组元素都是True
	 *
	 * @tparam 数组类型
	 * @param  数组
	 *
	 * @returns 是否所有元素都是True
	 */
	template <typename RangeType>
	FORCEINLINE bool AllOf(const RangeType& Range)
	{
		for (const auto& Element : Range)
		{
			if (!Element)
			{
				return false;
			}
		}
		return true;
	}

	/**
	 * @fn template <typename RangeType> FORCEINLINE bool NoneOf(const RangeType& Range)
	 *
	 * @brief 判断是否所有数组元素都是false
	 *
	 * @tparam 数组类型
	 * @param  数组
	 *
	 * @returns 是否所有元素都是false
	 */
	template <typename RangeType>
	FORCEINLINE bool NoneOf(const RangeType& Range)
	{
		for (const auto& Element : Range)
		{
			if (Element)
			{
				return false;
			}
		}

		return true;
	}

	/**
	 * @fn template <typename RangeType> FORCEINLINE bool AnyOf(const RangeType& Range)
	 *
	 * @brief 判断数组中是否有任意一个True
	 *
	 * @tparam 数组类型
	 * @param  数组
	 *
	 * @returns 是否有任意一个元素是true
	 */
	template <typename RangeType>
	FORCEINLINE bool AnyOf(const RangeType& Range)
	{
		return !Algo::NoneOf(Range);
	}
}