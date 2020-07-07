#pragma once
#include <CoreConfig.h>
#include <CoreType.h>
#include <Algo/BinarySearch.h>
#include <Algo/Rotate.h>
#include <Templates/Functor.h>

namespace Fuko::Algo::Impl
{
	/**
	 * @fn template <typename T, typename ProjectionType, typename PredicateType> void Merge(T* First, int32 Mid, int32 Num, ProjectionType Projection, PredicateType Predicate)
	 *
	 * @brief 归并排序的归并算法
	 *
	 * @param [in]	   First	  排序范围的起点
	 * @param 		   Mid		  归并排序的中点(组长)
	 * @param 		   Num		  执行排序的元素数量
	 * @param 		   Projection 映射函数
	 * @param 		   Predicate  谓语
	 */
	template <typename T, typename ProjectionType, typename PredicateType>
	void Merge(T* First, int32 Mid, int32 Num, ProjectionType Projection, PredicateType Predicate)
	{
		// 归并的子序列都是已经有序的
		int32 AStart = 0;		// 左侧序列的节点
		int32 BStart = Mid;		// 右侧序列的起点

		while (AStart < BStart && BStart < Num)
		{
			// 在A中寻找适合B插入的位置
			int32 NewAOffset = UpperBoundInternal(First + AStart, BStart - AStart, std::invoke(Projection, First[BStart]), Projection, Predicate);
			
			// |xxxxxxxA---|-----------|
			// 更新A的起点，NewAOffset的左侧已经是有序序列 
			AStart += NewAOffset;

			if (AStart >= BStart)	// 无需排序，整个序列已经是有序的
			{
				return;
			}

			// |xxxxxxxA***|####B------|
			// 从B序列中寻找正好可以插入A的位置
			int32 NewBOffset = LowerBoundInternal(First + BStart, Num - BStart, std::invoke(Projection, First[AStart]), Projection, Predicate);
			
			// |xxxxxxxA###|#***B------|
			// 交换两个区域
			RotateInternal(First + AStart, NewBOffset + BStart - AStart, BStart - AStart);

			// |xxxxxxxxxxx|xA------B--|
			//  xxxxxxxxxxx x|------|--|
			//               |------|--|
			// 更新有序区
			BStart += NewBOffset;
			AStart += NewBOffset + 1;
		}
	}

	constexpr int32 MinMergeSubgroupSize = 2;

	/**
	 * @fn template <typename T, typename ProjectionType, typename PredicateType> void StableSortInternal(T* First, int32 Num, ProjectionType Projection, PredicateType Predicate)
	 *
	 * @brief 归并排序，是稳定的
	 *
	 * @param [in]     First	  数组起点
	 * @param 		   Num		  数组长度
	 * @param 		   Projection 映射函数
	 * @param 		   Predicate  谓语
	 */
	template <typename T, typename ProjectionType, typename PredicateType>
	void StableSortInternal(T* First, int32 Num, ProjectionType Projection, PredicateType Predicate)
	{
		int32 SubgroupStart = 0;	// 子序列起点

		if constexpr (MinMergeSubgroupSize > 1)
		{
			if constexpr (MinMergeSubgroupSize > 2)
			{
				// 对最小组进行bubble排序
				do
				{
					int32 GroupEnd = SubgroupStart + MinMergeSubgroupSize;
					if (Num < GroupEnd)		// 校正尾部
					{
						GroupEnd = Num;
					}
					// 对单组进行冒泡排序
					do
					{
						for (int32 It = SubgroupStart; It < GroupEnd - 1; ++It)	
						{
							if (std::invoke(Predicate, std::invoke(Projection, First[It + 1]), std::invoke(Projection, First[It])))
							{
								Swap(First[It], First[It + 1]);
							}
						}
						GroupEnd--;
					} while (GroupEnd - SubgroupStart > 1);

					// 更新组 
					SubgroupStart += MinMergeSubgroupSize;
				} while (SubgroupStart < Num);
			}
			else
			{
				for (int32 Subgroup = 0; Subgroup < Num; Subgroup += 2)
				{
					if (Subgroup + 1 < Num && std::invoke(Predicate, std::invoke(Projection, First[Subgroup + 1]), std::invoke(Projection, First[Subgroup])))
					{
						Swap(First[Subgroup], First[Subgroup + 1]);
					}
				}
			}
		}

		int32 SubgroupSize = MinMergeSubgroupSize;	// 小子序列的大小 
		while (SubgroupSize < Num)
		{
			SubgroupStart = 0;	// 重置起点 
			do
			{
				int32 MergeNum = SubgroupSize << 1;		// 合并的数量 = 组长 * 2
				if (Num - SubgroupStart < MergeNum)		// 校准合并的数量 
				{
					MergeNum = Num - SubgroupStart;
				}

				// 归并
				Merge(First + SubgroupStart, SubgroupSize, MergeNum, Projection, Predicate);
				
				// 更新起点
				SubgroupStart += SubgroupSize << 1;
			} while (SubgroupStart < Num);

			SubgroupSize <<= 1;	// 增大归并组的大小
		}
	}
}

namespace Fuko::Algo
{
	/**
	 * @fn template <typename RangeType> FORCEINLINE void StableSort(RangeType& Range)
	 *
	 * @brief 稳定排序
	 *
	 * @param [in] Range 数组
	 */
	template <typename RangeType>
	FORCEINLINE void StableSort(RangeType& Range)
	{
		Impl::StableSortInternal(GetData(Range), GetNum(Range), FIdentityFunctor(), TLess<>());
	}

	/**
	 * @fn template <typename RangeType, typename PredicateType> FORCEINLINE void StableSort(RangeType& Range, PredicateType Pred)
	 *
	 * @brief 稳定排序
	 *
	 * @param [in]     Range 数组
	 * @param 		   Pred  排序谓语
	 */
	template <typename RangeType, typename PredicateType>
	FORCEINLINE void StableSort(RangeType& Range, PredicateType Pred)
	{
		Impl::StableSortInternal(GetData(Range), GetNum(Range), FIdentityFunctor(), std::move(Pred));
	}

	/**
	 * @fn template <typename RangeType, typename ProjectionType> FORCEINLINE void StableSortBy(RangeType& Range, ProjectionType Proj)
	 *
	 * @brief 稳定排序
	 *
	 * @param [in,out] Range 数组
	 * @param 		   Proj  映射函数
	 */
	template <typename RangeType, typename ProjectionType>
	FORCEINLINE void StableSortBy(RangeType& Range, ProjectionType Proj)
	{
		Impl::StableSortInternal(GetData(Range), GetNum(Range), std::move(Proj), TLess<>());
	}

	/**
	 * @fn template <typename RangeType, typename ProjectionType, typename PredicateType> FORCEINLINE void StableSortBy(RangeType& Range, ProjectionType Proj, PredicateType Pred)
	 *
	 * @brief 稳定排序
	 *
	 * @param [in]	Range 数组
	 * @param 		Proj  映射函数
	 * @param 		Pred  排序谓语
	 */
	template <typename RangeType, typename ProjectionType, typename PredicateType>
	FORCEINLINE void StableSortBy(RangeType& Range, ProjectionType Proj, PredicateType Pred)
	{
		Impl::StableSortInternal(GetData(Range), GetNum(Range), std::move(Proj), std::move(Pred));
	}
}
