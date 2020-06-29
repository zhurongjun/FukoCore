#pragma once
#include "CoreConfig.h"
#include "CoreType.h"
#include "Templates/UtilityTemp.h"
#include "Templates/Functor.h"

namespace Fuko::Algo::Impl
{
	template <typename RangeType, typename ValueType, typename ProjectionType>
	typename TRangePointerType<std::remove_reference_t<RangeType>> FindBy(RangeType&& Range, const ValueType& Value, ProjectionType Proj)
	{
		for (auto&& Elem : std::forward<RangeType>(Range))
		{
			if (Invoke(Proj, Elem) == Value)
			{
				return &Elem;
			}
		}

		return nullptr;
	}

	template <typename RangeType, typename PredicateType>
	typename TRangePointerType<std::remove_reference_t<RangeType>>::Type FindByPredicate(RangeType&& Range, PredicateType Pred)
	{
		for (auto&& Elem : std::forward<RangeType>(Range))
		{
			if (Invoke(Pred, Elem))
			{
				return &Elem;
			}
		}

		return nullptr;
	}

	template <typename T, typename ValueType, typename ProjectionType>
	T* FindLastBy(T* First, size_t Num, const ValueType& Value, ProjectionType Proj)
	{
		for (T* Last = First + Num; First != Last;)
		{
			if (Invoke(Proj, *--Last) == Value)
			{
				return Last;
			}
		}

		return nullptr;
	}

	template <typename T, typename PredicateType>
	T* FindLastByPredicate(T* First, size_t Num, PredicateType Pred)
	{
		for (T* Last = First + Num; First != Last;)
		{
			if (Invoke(Pred, *--Last))
			{
				return Last;
			}
		}

		return nullptr;
	}

	template<typename WhereType, typename WhatType>
	constexpr WhereType* FindSequence(WhereType* First, WhereType* Last, WhatType* WhatFirst, WhatType* WhatLast)
	{
		for (; ; ++First)
		{
			WhereType* It = First;
			for (WhatType* WhatIt = WhatFirst; ; ++It, ++WhatIt)
			{
				if (WhatIt == WhatLast)	// 已经找到了
				{
					return First;
				}
				if (It == Last)	// 未找到
				{
					return nullptr;
				}
				if (!(*It == *WhatIt))	// 元素出现差异重新找
				{
					break;
				}
			}
		}
	}
}

namespace Fuko::Algo
{
	/**
	 * @fn template <typename RangeType, typename ValueType> FORCEINLINE auto Find(RangeType&& Range, const ValueType& Value) -> decltype(Impl::FindBy(std::forward<RangeType>(Range), Value, FIdentityFunctor()))
	 *
	 * @brief 遍历查找
	 *
	 * @param [in]	   查找的数组
	 * @param 		   查找的值
	 *
	 * @returns 找到的元素
	 */
	template <typename RangeType, typename ValueType>
	FORCEINLINE auto Find(RangeType&& Range, const ValueType& Value)
		-> decltype(Impl::FindBy(std::forward<RangeType>(Range), Value, FIdentityFunctor()))
	{
		return Impl::FindBy(std::forward<RangeType>(Range), Value, FIdentityFunctor());
	}

	/**
	 * @fn template <typename RangeType, typename ValueType, typename ProjectionType> FORCEINLINE auto FindBy(RangeType&& Range, const ValueType& Value, ProjectionType Proj) -> decltype(Impl::FindBy(std::forward<RangeType>(Range), Value, std::move(Proj)))
	 *
	 * @brief 查找的值
	 *
	 * @param [in]	   Range 数组
	 * @param 		   Value 目标值
	 * @param 		   Proj  映射函数
	 *
	 * @returns 查找到的元素
	 */
	template <typename RangeType, typename ValueType, typename ProjectionType>
	FORCEINLINE auto FindBy(RangeType&& Range, const ValueType& Value, ProjectionType Proj)
		-> decltype(Impl::FindBy(std::forward<RangeType>(Range), Value, std::move(Proj)))
	{
		return Impl::FindBy(std::forward<RangeType>(Range), Value, std::move(Proj));
	}

	/**
	 * @fn template <typename RangeType, typename PredicateType> FORCEINLINE auto FindByPredicate(RangeType&& Range, PredicateType Pred) -> decltype(Impl::FindByPredicate(std::forward<RangeType>(Range), std::move(Pred)))
	 *
	 * @brief 遍历查找
	 *
	 * @param [in]	   Range 数组
	 * @param 		   Pred  判断条件是否符合的谓语
	 *
	 * @returns 查找到的元素
	 */
	template <typename RangeType, typename PredicateType>
	FORCEINLINE auto FindByPredicate(RangeType&& Range, PredicateType Pred)
		-> decltype(Impl::FindByPredicate(std::forward<RangeType>(Range), std::move(Pred)))
	{
		return Impl::FindByPredicate(std::forward<RangeType>(Range), std::move(Pred));
	}

	/**
	 * @fn template <typename RangeType, typename ValueType> FORCEINLINE auto FindLast(RangeType&& Range, const ValueType& Value) -> decltype(Impl::FindLastBy(GetData(Range), GetNum(Range), Value, FIdentityFunctor()))
	 *
	 * @brief 从尾部查找
	 *
	 * @param [in]	   Range 数组
	 * @param 		   Value 目标元素
	 *
	 * @returns 查找到的元素
	 */
	template <typename RangeType, typename ValueType>
	FORCEINLINE auto FindLast(RangeType&& Range, const ValueType& Value)
		-> decltype(Impl::FindLastBy(GetData(Range), GetNum(Range), Value, FIdentityFunctor()))
	{
		return Impl::FindLastBy(GetData(Range), GetNum(Range), Value, FIdentityFunctor());
	}

	/**
	 * @fn template <typename RangeType, typename ValueType, typename ProjectionType> FORCEINLINE auto FindLastBy(RangeType&& Range, const ValueType& Value, ProjectionType Proj) -> decltype(Impl::FindLastBy(GetData(Range), GetNum(Range), Value, std::move(Proj)))
	 *
	 * @brief 从尾部查找
	 *
	 * @param [in]	   Range 数组
	 * @param 		   Value 目标元素
	 * @param 		   Proj  映射函数
	 *
	 * @returns 查找到的元素
	 */
	template <typename RangeType, typename ValueType, typename ProjectionType>
	FORCEINLINE auto FindLastBy(RangeType&& Range, const ValueType& Value, ProjectionType Proj)
		-> decltype(Impl::FindLastBy(GetData(Range), GetNum(Range), Value, std::move(Proj)))
	{
		return Impl::FindLastBy(GetData(Range), GetNum(Range), Value, std::move(Proj));
	}

	/**
	 * @fn template <typename RangeType, typename PredicateType> FORCEINLINE auto FindLastByPredicate(RangeType&& Range, PredicateType Pred) -> decltype(Impl::FindLastByPredicate(GetData(Range), GetNum(Range), std::move(Pred)))
	 *
	 * @brief 从尾部查找
	 *
	 * @param [in]	   Range 数组
	 * @param 		   Pred  判断元素是否符合条件的谓语
	 *
	 * @returns 查找到的元素
	 */
	template <typename RangeType, typename PredicateType>
	FORCEINLINE auto FindLastByPredicate(RangeType&& Range, PredicateType Pred)
		-> decltype(Impl::FindLastByPredicate(GetData(Range), GetNum(Range), std::move(Pred)))
	{
		return Impl::FindLastByPredicate(GetData(Range), GetNum(Range), std::move(Pred));
	}

	/**
	 * @fn template<typename RangeWhereType, typename RangeWhatType> FORCEINLINE auto FindSequence(const RangeWhereType& Where, const RangeWhatType& What) -> decltype(Impl::FindSequence(GetData(Where), GetData(Where) + GetNum(Where), GetData(What), GetData(What) + GetNum(What)))
	 *
	 * @brief 查找子序列在父序列中的位置
	 *
	 * @param  Where 查找的父数组
	 * @param  What  想要查找的子数组
	 *
	 * @returns 找到的子数组在父数组中的起始指针
	 */
	template<typename RangeWhereType, typename RangeWhatType>
	FORCEINLINE auto FindSequence(const RangeWhereType& Where, const RangeWhatType& What)
		-> decltype(Impl::FindSequence(GetData(Where), GetData(Where) + GetNum(Where), GetData(What), GetData(What) + GetNum(What)))
	{
		if (GetNum(What) > GetNum(Where))
		{
			return nullptr;
		}
		else
		{
			return Impl::FindSequence(
				GetData(Where), GetData(Where) + GetNum(Where),
				GetData(What), GetData(What) + GetNum(What));
		}
	}
}