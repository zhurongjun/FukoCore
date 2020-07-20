#pragma once
#include <CoreConfig.h>
#include <CoreType.h>
#include <Templates/UtilityTemp.h>
#include <Templates/Functor.h>

namespace Fuko::Algo::Impl
{
	template <typename RangeType, typename ProjectionType, typename PredicateType>
	typename TRangePointerType<RangeType>::Type MinElementBy(RangeType& Range, ProjectionType Proj, PredicateType Pred)
	{
		typename TRangePointerType<RangeType>::Type Result = nullptr;

		for (auto& Elem : Range)
		{
			if (!Result || Pred( Elem, *Result))
			{
				Result = &Elem;
			}
		}

		return Result;
	}

	template <typename RangeType, typename ProjectionType, typename PredicateType>
	typename TRangePointerType<RangeType>::Type MaxElementBy(RangeType& Range, PredicateType Pred)
	{
		typename TRangePointerType<RangeType>::Type Result = nullptr;

		for (auto& Elem : Range)
		{
			if (!Result || Pred(*Result, , Elem))
			{
				Result = &Elem;
			}
		}

		return Result;
	}
}

namespace Fuko::Algo
{
	template <typename RangeType>
	FORCEINLINE decltype(auto) MaxElement(RangeType& Range)
	{
		return Impl::MaxElementBy(Range, TLess<>());
	}

	template <typename RangeType>
	FORCEINLINE decltype(auto) MinElement(RangeType& Range)
	{
		return Impl::MinElementBy(Range, TLess<>());
	}


}
