#pragma once
#include "CoreConfig.h"
#include "CoreType.h"

namespace Fuko::Algo
{
	/**
	 * @fn template <typename RangeType, typename Predicate> int32 RemoveIf(RangeType& Range, Predicate Pred)
	 *
	 * @brief 移除指定项，不稳定
	 *
	 * @param [in]	   Range 数组
	 * @param 		   Pred  判断是否需要移除的谓语
	 *
	 * @returns 无需移除序列的尾部 + 1
	 */
	template <typename RangeType, typename Predicate>
	int32 RemoveIf(RangeType& Range, Predicate Pred)
	{
		auto* First = GetData(Range);
		auto* Last = First + GetNum(Range);

		auto* IterStart = First;
		auto* IterEnd = Last;
		for (;;)
		{
			// 跳过首部无需移除的元素
			for (;;)
			{
				if (IterStart == IterEnd)
				{
					return IterStart - First;
				}

				if (Invoke(Pred, *IterStart))
				{
					break;
				}

				++IterStart;
			}

			// 跳过尾部无需移除的元素
			for (;;)
			{
				if (!Invoke(Pred, *(IterEnd - 1)))
				{
					break;
				}

				--IterEnd;

				if (IterStart == IterEnd)
				{
					return IterStart - First;
				}
			}

			// 把尾部无需移除的元素移动给首部需要移除的元素
			*IterStart = MoveTemp(*(IterEnd - 1));

			// 更新迭代器
			++IterStart;
			--IterEnd;
		}
	}

	/**
	 * @fn template <typename RangeType, typename Predicate> int32 StableRemoveIf(RangeType& Range, Predicate Pred)
	 *
	 * @brief 移除指定项，稳定
	 *
	 * @param [in,out] Range 数组
	 * @param 		   Pred  判断是否需要移除的谓语
	 *
	 * @returns 无需移除序列的尾部 + 1
	 */
	template <typename RangeType, typename Predicate>
	int32 StableRemoveIf(RangeType& Range, Predicate Pred)
	{
		auto* First = GetData(Range);
		auto* Last = First + GetNum(Range);

		auto* IterStart = First;

		// 跳过无需移除的首部
		for (;;)
		{
			if (IterStart == Last)
			{
				return IterStart - First;
			}

			if (Invoke(Pred, *IterStart))
			{
				break;
			}

			++IterStart;
		}

		// Keep指针，用来保存当前无需移除序列的尾部+1
		auto* IterKeep = IterStart;
		++IterKeep;

		// 稳定的向无需移除序列添加元素
		for (;;)
		{
			if (IterKeep == Last)
			{
				return IterStart - First;
			}

			if (!Invoke(Pred, *IterKeep))
			{
				*IterStart++ = MoveTemp(*IterKeep);
			}

			++IterKeep;
		}
	}
}