#pragma once
#include <CoreConfig.h>
#include <CoreType.h>

namespace Fuko::Algo
{
	/**
	 * @fn template <typename InT, typename ValueT> FORCEINLINE size_t Count(const InT& Input, const ValueT& InValue)
	 *
	 * @brief 计数
	 *
	 * @param  Input   源数组
	 * @param  InValue 对比的值
	 *
	 * @returns Value在Input中的数量
	 */
	template <typename InT, typename ValueT>
	FORCEINLINE size_t Count(const InT& Input, const ValueT& InValue)
	{
		size_t Result = 0;
		for (const auto& Value : Input)
		{
			if (Value == InValue)
			{
				++Result;
			}
		}
		return Result;
	}

	/**
	 * @fn template <typename InT, typename PredicateT> FORCEINLINE size_t CountIf(const InT& Input, PredicateT Predicate)
	 *
	 * @brief 带谓语的计数
	 *
	 * @param  Input	 输入的数组
	 * @param  Predicate 谓语
	 *
	 * @returns 符合谓语的元素数量
	 */
	template <typename InT, typename PredicateT>
	FORCEINLINE size_t CountIf(const InT& Input, PredicateT Predicate)
	{
		size_t Result = 0;
		for (const auto& Value : Input)
		{
			if (std::invoke(Predicate, Value))
			{
				++Result;
			}
		}
		return Result;
	}
}