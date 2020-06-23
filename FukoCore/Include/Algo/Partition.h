#pragma once
#include "CoreConfig.h"
#include "CoreType.h"

namespace Fuko::Algo
{
	/**
	 * @fn template<class T, typename IndexType, class UnaryPredicate> IndexType Partition(T* Elements, const IndexType Num, const UnaryPredicate& Predicate)
	 *
	 * @brief 分割，使得谓词为true的元素都位于谓词为false的元素之前
	 *
	 * @param [in]     Elements  元素数组
	 * @param 		   Num		 元素数量
	 * @param 		   Predicate 谓词
	 *
	 * @returns 谓词开始为false的第一个位置
	 */
	template<class T, typename IndexType, class UnaryPredicate>
	IndexType Partition(T* Elements, const IndexType Num, const UnaryPredicate& Predicate)
	{
		T* First = Elements;
		T* Last = Elements + Num;

		while (First != Last)
		{
			while (Predicate(*First))	// 跳过谓词为true的前段 
			{
				++First;
				if (First == Last)
				{
					return (IndexType)(First - Elements);
				}
			}

			do			// 跳过谓词为false的后段
			{
				--Last;
				if (First == Last)
				{
					return (IndexType)(First - Elements);
				}
			} while (!Predicate(*Last));

			Swap(*First, *Last);	// 对产生差异的点进行交换
			++First;	// 此时First指向的元素已经是正确的，跳过这个元素
		}

		return (IndexType)(First - Elements);
	}
}
