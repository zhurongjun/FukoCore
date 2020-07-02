#pragma once
#include "CoreConfig.h"
#include "CoreType.h"
#include "Templates/Functor.h"
#include "Templates/UtilityTemp.h"

namespace Fuko::Algo::Impl
{
	/**
	 * @fn template <typename RangeValueType, typename SizeType, typename PredicateValueType, typename ProjectionType, typename SortPredicateType> FORCEINLINE SizeType LowerBoundInternal(RangeValueType* First, const SizeType Num, const PredicateValueType& Value, ProjectionType Projection, SortPredicateType SortPredicate)
	 *
	 * @brief 二分查找，查找上界
	 *
	 * @param [in]	   First		 起始点
	 * @param 		   Num			 二分查找的数量
	 * @param 		   Value		 查找值
	 * @param 		   Projection    映射函数
	 * @param 		   SortPredicate 比较谓语
	 *
	 * @returns 查找到的上界
	 */
	template <typename RangeValueType, typename SizeType, typename PredicateValueType, typename ProjectionType, typename SortPredicateType>
	FORCEINLINE SizeType LowerBoundInternal(RangeValueType* First, const SizeType Num, const PredicateValueType& Value, ProjectionType Projection, SortPredicateType SortPredicate)
	{
		// 查找范围起点 
		SizeType Start = 0;
		// 查找范围大小 
		SizeType Size = Num;

		while (Size > 0)
		{
			const SizeType LeftoverSize = Size % 2;
			Size = Size / 2;

			const SizeType CheckIndex = Start + Size;	// 中位 
			const SizeType StartIfLess = CheckIndex + LeftoverSize;	// 如果谓语成立的下一个起始点 
			
			auto&& CheckValue = std::invoke(Projection, First[CheckIndex]);	// 经过映射后的比较中值 
			Start = SortPredicate(CheckValue, Value) ? StartIfLess : Start;	// 更新起点 
		}
		return Start;
	}

	/**
	 * @fn template <typename RangeValueType, typename SizeType, typename PredicateValueType, typename ProjectionType, typename SortPredicateType> FORCEINLINE SizeType UpperBoundInternal(RangeValueType* First, const SizeType Num, const PredicateValueType& Value, ProjectionType Projection, SortPredicateType SortPredicate)
	 *
	 * @brief 二分查找，查找下界
	 *
	 * @param [in]     First		 起始点
	 * @param 		   Num			 二分查找的数量
	 * @param 		   Value		 查找值
	 * @param 		   Projection    映射函数
	 * @param 		   SortPredicate 比较谓语
	 *
	 * @returns 查找到的下界
	 */
	template <typename RangeValueType, typename SizeType, typename PredicateValueType, typename ProjectionType, typename SortPredicateType>
	FORCEINLINE SizeType UpperBoundInternal(RangeValueType* First, const SizeType Num, const PredicateValueType& Value, ProjectionType Projection, SortPredicateType SortPredicate)
	{
		SizeType Start = 0;
		SizeType Size = Num;

		while (Size > 0)
		{
			const SizeType LeftoverSize = Size % 2;
			Size = Size / 2;

			const SizeType CheckIndex = Start + Size;
			const SizeType StartIfLess = CheckIndex + LeftoverSize;

			auto&& CheckValue = std::invoke(Projection, First[CheckIndex]);
			Start = !SortPredicate(Value, CheckValue) ? StartIfLess : Start;
		}

		return Start;
	}
}

namespace Fuko::Algo
{
	/**
	 * @fn template <typename RangeType, typename ValueType, typename SortPredicateType> FORCEINLINE auto LowerBound(RangeType& Range, const ValueType& Value, SortPredicateType SortPredicate) -> decltype(GetNum(Range))
	 *
	 * @brief 二分查找,寻找下界
	 *
	 * @param [in]     Range		 查找的数组
	 * @param 		   Value		 查找的值
	 * @param 		   SortPredicate 查找谓语
	 *
	 * @returns 查找到的索引
	 */
	template <typename RangeType, typename ValueType, typename SortPredicateType>
	FORCEINLINE auto LowerBound(RangeType& Range, const ValueType& Value, SortPredicateType SortPredicate) -> decltype(GetNum(Range))
	{
		return ::Fuko::Algo::Impl::LowerBoundInternal(GetData(Range), GetNum(Range), Value, FIdentityFunctor(), SortPredicate);
	}

	/**
	 * @fn template <typename RangeType, typename ValueType> FORCEINLINE auto LowerBound(RangeType& Range, const ValueType& Value) -> decltype(GetNum(Range))
	 *
	 * @brief 二分查找,寻找下界
	 *
	 * @param [in]	Range   查找的数组
	 * @param 		Value   查找的值
	 * 				   
	 * @returns 查找到的索引
	 */
	template <typename RangeType, typename ValueType>
	FORCEINLINE auto LowerBound(RangeType& Range, const ValueType& Value) -> decltype(GetNum(Range))
	{
		return ::Fuko::Algo::Impl::LowerBoundInternal(GetData(Range), GetNum(Range), Value, FIdentityFunctor(), TLess<>());
	}

	/**
	 * @fn template <typename RangeType, typename ValueType, typename ProjectionType, typename SortPredicateType> FORCEINLINE auto LowerBoundBy(RangeType& Range, const ValueType& Value, ProjectionType Projection, SortPredicateType SortPredicate) -> decltype(GetNum(Range))
	 *
	 * @brief 二分查找,寻找下界
	 *
	 * @param [in]	   Range		 查找的数组
	 * @param 		   Value		 查找的值
	 * @param 		   Projection    映射方式
	 * @param 		   SortPredicate 排序谓语
	 *
	 * @returns 查找到的索引
	 */
	template <typename RangeType, typename ValueType, typename ProjectionType, typename SortPredicateType>
	FORCEINLINE auto LowerBoundBy(RangeType& Range, const ValueType& Value, ProjectionType Projection, SortPredicateType SortPredicate) -> decltype(GetNum(Range))
	{
		return ::Fuko::Algo::Impl::LowerBoundInternal(GetData(Range), GetNum(Range), Value, Projection, SortPredicate);
	}

	/**
	 * @fn template <typename RangeType, typename ValueType, typename ProjectionType> FORCEINLINE auto LowerBoundBy(RangeType& Range, const ValueType& Value, ProjectionType Projection) -> decltype(GetNum(Range))
	 *
	 * @brief 二分查找,寻找下界
	 *
	 * @param [in]	   Range	  查找的数组
	 * @param 		   Value	  查找的值
	 * @param 		   Projection 映射方式
	 *
	 * @returns 查找到的索引
	 */
	template <typename RangeType, typename ValueType, typename ProjectionType>
	FORCEINLINE auto LowerBoundBy(RangeType& Range, const ValueType& Value, ProjectionType Projection) -> decltype(GetNum(Range))
	{
		return ::Fuko::Algo::Impl::LowerBoundInternal(GetData(Range), GetNum(Range), Value, Projection, TLess<>());
	}

	/**
	 * @fn template <typename RangeType, typename ValueType, typename SortPredicateType> FORCEINLINE auto UpperBound(RangeType& Range, const ValueType& Value, SortPredicateType SortPredicate) -> decltype(GetNum(Range))
	 *
	 * @brief 二分查找，查找上界
	 *
	 * @param [in]	   Range		 需要查找的数组
	 * @param 		   Value		 查找的值
	 * @param 		   SortPredicate 查找谓语
	 *
	 * @returns 查找到的索引
	 */
	template <typename RangeType, typename ValueType, typename SortPredicateType>
	FORCEINLINE auto UpperBound(RangeType& Range, const ValueType& Value, SortPredicateType SortPredicate) -> decltype(GetNum(Range))
	{
		return ::Fuko::Algo::Impl::UpperBoundInternal(GetData(Range), GetNum(Range), Value, FIdentityFunctor(), SortPredicate);
	}

	/**
	 * @fn template <typename RangeType, typename ValueType> FORCEINLINE auto UpperBound(RangeType& Range, const ValueType& Value) -> decltype(GetNum(Range))
	 *
	 * @brief 二分查找，查找上界
	 *
	 * @param [in]	 Range  需要查找的数组
	 * @param 		 Value  查找的值
	 *
	 * @returns 查找到的索引
	 */
	template <typename RangeType, typename ValueType>
	FORCEINLINE auto UpperBound(RangeType& Range, const ValueType& Value) -> decltype(GetNum(Range))
	{
		return ::Fuko::Algo::Impl::UpperBoundInternal(GetData(Range), GetNum(Range), Value, FIdentityFunctor(), TLess<>());
	}

	/**
	 * @fn template <typename RangeType, typename ValueType, typename ProjectionType, typename SortPredicateType> FORCEINLINE auto UpperBoundBy(RangeType& Range, const ValueType& Value, ProjectionType Projection, SortPredicateType SortPredicate) -> decltype(GetNum(Range))
	 *
	 * @brief 二分查找，查找上界
	 *
	 * @param [in]	   Range		 需要查找的数组
	 * @param 		   Value		 查找的值
	 * @param 		   Projection    映射函数
	 * @param 		   SortPredicate 查找谓语
	 *
	 * @returns 查找到的索引
	 */
	template <typename RangeType, typename ValueType, typename ProjectionType, typename SortPredicateType>
	FORCEINLINE auto UpperBoundBy(RangeType& Range, const ValueType& Value, ProjectionType Projection, SortPredicateType SortPredicate) -> decltype(GetNum(Range))
	{
		return ::Fuko::Algo::Impl::UpperBoundInternal(GetData(Range), GetNum(Range), Value, Projection, SortPredicate);
	}

	/**
	 * @fn template <typename RangeType, typename ValueType, typename ProjectionType> FORCEINLINE auto UpperBoundBy(RangeType& Range, const ValueType& Value, ProjectionType Projection) -> decltype(GetNum(Range))
	 *
	 * @brief 二分查找，查找上界
	 *
	 * @param [in]	   Range	  需要查找的数组
	 * @param 		   Value	  查找的值
	 * @param 		   Projection 映射函数
	 *
	 * @returns 查找到的索引
	 */
	template <typename RangeType, typename ValueType, typename ProjectionType>
	FORCEINLINE auto UpperBoundBy(RangeType& Range, const ValueType& Value, ProjectionType Projection) -> decltype(GetNum(Range))
	{
		return ::Fuko::Algo::Impl::UpperBoundInternal(GetData(Range), GetNum(Range), Value, Projection, TLess<>());
	}

	/**
	 * @fn template <typename RangeType, typename ValueType, typename SortPredicateType> FORCEINLINE auto BinarySearch(RangeType& Range, const ValueType& Value, SortPredicateType SortPredicate) -> decltype(GetNum(Range))
	 *
	 * @brief 二分查找
	 *
	 * @param [in]	   Range		 查找的数组
	 * @param 		   Value		 查找的值
	 * @param 		   SortPredicate 查找谓语
	 *
	 * @returns 查找到的索引
	 */
	template <typename RangeType, typename ValueType, typename SortPredicateType>
	FORCEINLINE auto BinarySearch(RangeType& Range, const ValueType& Value, SortPredicateType SortPredicate) -> decltype(GetNum(Range))
	{
		auto CheckIndex = LowerBound(Range, Value, SortPredicate);
		if (CheckIndex < GetNum(Range))
		{
			auto&& CheckValue = GetData(Range)[CheckIndex];
			// 使用相反的谓语进行一下测试，假设使用的Functor是Less<>() 
			// 未找到的情况有三种，小于序列中所有值，大于序列中所有值，值在其中但是序列中没有 
			// 对于大于序列中所有值的情况，Index == Num，在上一层就以及筛掉了 
			// 对于小于序列中所有值的情况，Index == 0， 且此次谓语测试为true 
			// 对于在序列范围中的情况，Index上的值大于Value，则此次谓语测试为true 
			// 对于已经找到的情况，此次谓语测试为false 
			if (!SortPredicate(Value, CheckValue))
			{
				return CheckIndex;
			}
		}
		return INDEX_NONE;
	}

	/**
	 * @fn template <typename RangeType, typename ValueType> FORCEINLINE auto BinarySearch(RangeType& Range, const ValueType& Value)
	 *
	 * @brief 二分查找
	 *
	 * @param [in]	Range   查找的数组
	 * @param 		Value   查找的值
	 *
	 * @returns 查找到的索引
	 */
	template <typename RangeType, typename ValueType>
	FORCEINLINE auto BinarySearch(RangeType& Range, const ValueType& Value)
	{
		return BinarySearch(Range, Value, TLess<>());
	}

	/**
	 * @fn template <typename RangeType, typename ValueType, typename ProjectionType, typename SortPredicateType> FORCEINLINE auto BinarySearchBy(RangeType& Range, const ValueType& Value, ProjectionType Projection, SortPredicateType SortPredicate) -> decltype(GetNum(Range))
	 *
	 * @brief 二分查找
	 *
	 * @param [in]	   Range		 查找的数组
	 * @param 		   Value		 查找的值
	 * @param 		   Projection    映射函数
	 * @param 		   SortPredicate 查找谓语
	 *
	 * @returns 查找到的索引
	 */
	template <typename RangeType, typename ValueType, typename ProjectionType, typename SortPredicateType>
	FORCEINLINE auto BinarySearchBy(RangeType& Range, const ValueType& Value, ProjectionType Projection, SortPredicateType SortPredicate) -> decltype(GetNum(Range))
	{
		auto CheckIndex = LowerBoundBy(Range, Value, Projection, SortPredicate);
		if (CheckIndex < GetNum(Range))
		{
			auto&& CheckValue = std::invoke(Projection, GetData(Range)[CheckIndex]);
			if (!SortPredicate(Value, CheckValue))
			{
				return CheckIndex;
			}
		}
		return INDEX_NONE;
	}

	/**
	 * @fn template <typename RangeType, typename ValueType, typename ProjectionType> FORCEINLINE auto BinarySearchBy(RangeType& Range, const ValueType& Value, ProjectionType Projection)
	 *
	 * @brief 二分查找
	 *
	 * @param [in,out] Range	  查找的数组
	 * @param 		   Value	  查找的值
	 * @param 		   Projection 映射函数
	 *
	 * @returns An auto.
	 */
	template <typename RangeType, typename ValueType, typename ProjectionType>
	FORCEINLINE auto BinarySearchBy(RangeType& Range, const ValueType& Value, ProjectionType Projection)
	{
		return BinarySearchBy(Range, Value, Projection, TLess<>());
	}
}