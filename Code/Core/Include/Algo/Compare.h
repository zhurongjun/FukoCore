#pragma once
#include <CoreConfig.h>
#include <CoreType.h>

namespace Fuko::Algo
{
	/**
	 * @fn template <typename InAT, typename InBT, typename PredicateT> constexpr bool CompareByPredicate(InAT&& InputA, InBT&& InputB, PredicateT Predicate)
	 *
	 * @brief 比较
	 * 			 
	 * @param [in,out] InputA    输入比较数组A
	 * @param [in,out] InputB    输入比较数组B
	 * @param 		   Predicate 比较谓语
	 *
	 * @returns true 所有元素通过了谓语检测，false 有至少一个元素没有通过谓语检测
	 */
	template <typename InAT, typename InBT, typename PredicateT>
	constexpr bool CompareByPredicate(InAT&& InputA, InBT&& InputB, PredicateT Predicate)
	{
		const size_t SizeA = GetNum(InputA);
		const size_t SizeB = GetNum(InputB);

		if (SizeA != SizeB)
		{
			return false;
		}

		auto* A = GetData(InputA);
		auto* B = GetData(InputB);

		for (size_t Count = SizeA; Count; --Count)
		{
			if (!std::invoke(Predicate, *A++, *B++))
			{
				return false;
			}
		}

		return true;
	}
}