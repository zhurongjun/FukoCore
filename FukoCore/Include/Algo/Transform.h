#pragma once
#include "CoreConfig.h"
#include "CoreType.h"

namespace Fuko::Algo
{
	/**
	 * @fn template <typename InT, typename OutT, typename PredicateT, typename TransformT> FORCEINLINE void TransformIf(const InT& Input, OutT& Output, PredicateT Predicate, TransformT Trans)
	 *
	 * @brief 根据条件执行转换
	 *
	 * @param 		Input	  源数组
	 * @param [out]	Output    输出数组
	 * @param 		Predicate 谓语函数
	 * @param 		Trans	  转换函数
	 */
	template <typename InT, typename OutT, typename PredicateT, typename TransformT>
	FORCEINLINE void TransformIf(const InT& Input, OutT& Output, PredicateT Predicate, TransformT Trans)
	{
		for (const auto& Value : Input)
		{
			if (std::invoke(Predicate, Value))
			{
				Output.Add(std::invoke(Trans, Value));
			}
		}
	}

	/**
	 * @fn template <typename InT, typename OutT, typename TransformT> FORCEINLINE void Transform(const InT& Input, OutT& Output, TransformT Trans)
	 *
	 * @brief 执行转换
	 *
	 * @param 		   Input  源数组
	 * @param [out]    Output 输出数组
	 * @param 		   Trans  转换函数
	 */
	template <typename InT, typename OutT, typename TransformT>
	FORCEINLINE void Transform(const InT& Input, OutT& Output, TransformT Trans)
	{
		for (const auto& Value : Input)
		{
			Output.Add(std::invoke(Trans, Value));
		}
	}
}