#pragma once
#include "CoreConfig.h"
#include "CoreType.h"
#include "Templates/Functor.h"

namespace Fuko::Algo::Impl
{
	template <typename T, typename SizeType, typename BinaryPredicate>
	SizeType Unique(T* Array, SizeType ArraySize, BinaryPredicate Predicate)
	{
		if (ArraySize <= 1)
		{
			return ArraySize;
		}

		T* Result = Array;
		for (T* Iter = Array + 1; Iter != Array + ArraySize; ++Iter)
		{
			if (!Invoke(Predicate, *Result, *Iter))
			{
				++Result;
				if (Result != Iter)
				{
					*Result = std::move(*Iter);
				}
			}
		}
		
		return static_cast<SizeType>(Result + 1 - Array);
	}
}

namespace Fuko::Algo
{
	/**
	 * @fn template<typename RangeType> auto Unique(RangeType&& Range) -> decltype(Impl::Unique(GetData(Range), GetNum(Range), TEqualTo<>{}))
	 *
	 * @brief 保证数组内所有元素都是唯一的
	 *
	 * @param [in,out] 输入的数组，必须是唯一的
	 *
	 * @returns 唯一序列的末尾 + 1
	 */
	template<typename RangeType>
	auto Unique(RangeType&& Range) -> decltype(Impl::Unique(GetData(Range), GetNum(Range), TEqualTo<>{}))
	{
		return Impl::Unique(GetData(Range), GetNum(Range), TEqualTo<>{});
	}

	/**
	 * @fn template<typename RangeType, typename BinaryPredicate> auto Unique(RangeType&& Range, BinaryPredicate Predicate) -> decltype(Impl::Unique(GetData(Range), GetNum(Range), std::move(Predicate)))
	 *
	 * @brief 保证数组内所有元素都是唯一的
	 *
	 * @param [in,out] Range	 输入的数组，必须是唯一的
	 * @param 		   Predicate 比较谓语
	 *
	 * @returns 唯一序列的末尾 + 1
	 */
	template<typename RangeType, typename BinaryPredicate>
	auto Unique(RangeType&& Range, BinaryPredicate Predicate) -> decltype(Impl::Unique(GetData(Range), GetNum(Range), std::move(Predicate)))
	{
		return Impl::Unique(GetData(Range), GetNum(Range), std::move(Predicate));
	}
}

