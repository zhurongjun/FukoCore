#pragma once
#include <CoreConfig.h>
#include <CoreType.h>

namespace Fuko::Algo
{
	/**
	 * @fn template <typename InT, typename PredicateT, typename CallableT> FORCEINLINE void ForEachIf(InT& Input, PredicateT Predicate, CallableT Callable)
	 *
	 * @brief 有条件的遍历元素
	 *
	 * @param [in]	   Input	 有条件的遍历元素
	 * @param 		   Predicate 遍历成立的条件
	 * @param 		   Callable  调用的函数
	 */
	template <typename InT, typename PredicateT, typename CallableT>
	FORCEINLINE void ForEachIf(InT& Input, PredicateT Predicate, CallableT Callable)
	{
		for (auto& Value : Input)
		{
			if (std::invoke(Predicate, Value))
			{
				std::invoke(Callable, Value);
			}
		}
	}

	/**
	 * @fn template <typename InT, typename CallableT> FORCEINLINE void ForEach(InT& Input, CallableT Callable)
	 *
	 * @brief 遍历元素
	 *
	 * @param [in]     Input    数组
	 * @param 		   Callable 调用的函数
	 */
	template <typename InT, typename CallableT>
	FORCEINLINE void ForEach(InT& Input, CallableT Callable)
	{
		for (auto& Value : Input)
		{
			std::invoke(Callable, Value);
		}
	}
}