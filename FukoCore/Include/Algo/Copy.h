#pragma once
#include "CoreConfig.h"
#include "CoreType.h"

namespace Fuko::Algo
{
	/**
	 * @fn template <typename InT, typename OutT, typename PredicateT> FORCEINLINE void CopyIf(const InT& Input, OutT& Output, PredicateT Predicate)
	 *
	 * @brief 具有谓语条件的拷贝
	 *
	 * @param 		   Input	 拷贝源
	 * @param [in,out] Output    拷贝目标
	 * @param 		   Predicate 拷贝谓语
	 */
	template <typename InT, typename OutT, typename PredicateT>
	FORCEINLINE void CopyIf(const InT& Input, OutT& Output, PredicateT Predicate)
	{
		for (const auto& Value : Input)
		{
			if (std::invoke(Predicate, Value))
			{
				Output.Add(Value);
			}
		}
	}

	/**
	 * @fn template <typename InT, typename OutT> FORCEINLINE void Copy(const InT& Input, OutT& Output)
	 *
	 * @brief 拷贝操作
	 *
	 * @param 		   Input  拷贝源
	 * @param [out]    Output 拷贝目标
	 */
	template <typename InT, typename OutT>
	FORCEINLINE void Copy(const InT& Input, OutT& Output)
	{
		for (const auto& Value : Input)
		{
			Output.Add(Value);
		}
	}
}