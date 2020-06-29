#pragma once
#include "CoreConfig.h"
#include "CoreType.h"
#include "Templates/UtilityTemp.h"
#include "Templates/Functor.h"

namespace Fuko::Algo::Impl
{
	template <typename RangeType, typename ProjectionType, typename PredicateType>
	typename TRangePointerType<RangeType>::Type MinElementBy(RangeType& Range, ProjectionType Proj, PredicateType Pred)
	{
		typename TRangePointerType<RangeType>::Type Result = nullptr;

		for (auto& Elem : Range)
		{
			if (!Result || Invoke(Pred, Invoke(Proj, Elem), Invoke(Proj, *Result)))
			{
				Result = &Elem;
			}
		}

		return Result;
	}

	template <typename RangeType, typename ProjectionType, typename PredicateType>
	typename TRangePointerType<RangeType>::Type MaxElementBy(RangeType& Range, ProjectionType Proj, PredicateType Pred)
	{
		typename TRangePointerType<RangeType>::Type Result = nullptr;

		for (auto& Elem : Range)
		{
			if (!Result || Invoke(Pred, Invoke(Proj, *Result), Invoke(Proj, Elem)))
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
	FORCEINLINE auto MaxElement(RangeType& Range)
		-> decltype(Impl::MaxElementBy(Range, FIdentityFunctor(), TLess<>()))
	{
		return Impl::MaxElementBy(Range, FIdentityFunctor(), TLess<>());
	}

	template <typename RangeType, typename ComparatorType>
	FORCEINLINE auto MaxElement(RangeType& Range, ComparatorType Comp)
		-> decltype(Impl::MaxElementBy(Range, FIdentityFunctor(), std::move(Comp)))
	{
		return Impl::MaxElementBy(Range, FIdentityFunctor(), std::move(Comp));
	}

	template <typename RangeType, typename ProjectionType>
	FORCEINLINE auto MaxElementBy(RangeType& Range, ProjectionType Proj)
		-> decltype(Impl::MaxElementBy(Range, std::move(Proj), TLess<>()))
	{
		return Impl::MaxElementBy(Range, std::move(Proj), TLess<>());
	}

	
	template <typename RangeType, typename ProjectionType, typename ComparatorType>
	FORCEINLINE auto MaxElementBy(RangeType& Range, ProjectionType Proj, ComparatorType Comp)
		-> decltype(Impl::MaxElementBy(Range, std::move(Proj), std::move(Comp)))
	{
		return Impl::MaxElementBy(Range, std::move(Proj), std::move(Comp));
	}

	template <typename RangeType>
	FORCEINLINE auto MinElement(RangeType& Range)
		-> decltype(Impl::MinElementBy(Range, FIdentityFunctor(), TLess<>()))
	{
		return Impl::MinElementBy(Range, FIdentityFunctor(), TLess<>());
	}

	template <typename RangeType, typename ComparatorType>
	FORCEINLINE auto MinElement(RangeType& Range, ComparatorType Comp)
		-> decltype(Impl::MinElementBy(Range, FIdentityFunctor(), std::move(Comp)))
	{
		return Impl::MinElementBy(Range, FIdentityFunctor(), std::move(Comp));
	}

	template <typename RangeType, typename ProjectionType>
	FORCEINLINE auto MinElementBy(RangeType& Range, ProjectionType Proj)
		-> decltype(Impl::MinElementBy(Range, std::move(Proj), TLess<>()))
	{
		return Impl::MinElementBy(Range, std::move(Proj), TLess<>());
	}

	template <typename RangeType, typename ProjectionType, typename ComparatorType>
	FORCEINLINE auto MinElementBy(RangeType& Range, ProjectionType Proj, ComparatorType Comp)
		-> decltype(Impl::MinElementBy(Range, std::move(Proj), std::move(Comp)))
	{
		return Impl::MinElementBy(Range, std::move(Proj), std::move(Comp));
	}
}
