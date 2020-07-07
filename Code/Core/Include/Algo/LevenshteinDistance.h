#pragma once
#include <CoreConfig.h>
#include <CoreType.h>

namespace Fuko::Algo
{
	/**
	 * @fn template <typename RangeAType, typename RangeBType> int32 LevenshteinDistance(const RangeAType& RangeA, const RangeBType& RangeB)
	 *
	 * @brief 编辑距离算法，得出从A到B或者B到A所需要改动的元素个数
	 *
	 * @param  RangeA 数组A
	 * @param  RangeB 数组B
	 *
	 * @returns 改动的个数
	 */
	template <typename RangeAType, typename RangeBType>
	int32 LevenshteinDistance(const RangeAType& RangeA, const RangeBType& RangeB)
	{
		const int32 LenA = GetNum(RangeA);
		const int32 LenB = GetNum(RangeB);
		
		if (LenA == 0)
		{
			return LenB;
		}
		else if (LenB == 0)
		{
			return LenA;
		}

		auto DataA = GetData(RangeA);
		auto DataB = GetData(RangeB);

		TArray<int32> OperationCount;
		
		// 初始化一个数组作为列，这个列从最左侧开始
		OperationCount.AddUninitialized(LenB + 1);
		for (int32 IndexB = 0; IndexB <= LenB; ++IndexB)
		{
			OperationCount[IndexB] = IndexB;
		}
		// 遍历行(A)   
		for (int32 IndexA = 0; IndexA < LenA; ++IndexA)
		{
			int32 LastCount = IndexA + 1;	// 列顶端的初始行值(A) 
			// 开始遍历列 
			for (int32 IndexB = 0; IndexB < LenB; ++IndexB)
			{
				int32 NewCount = OperationCount[IndexB];	// 左上角列值 
				if (DataA[IndexA] != DataB[IndexB])	// 如果两者的值不同，则取左上角值+1 
				{
					// NewCount: 左上角值
					// LastCount: 上方值
					// OperationCount[IndexB + 1]): 左侧值
					// 这种情况下，三个值都要 + 1，等价于先取得最小值在 + 1
					NewCount = FMath::Min3(NewCount, LastCount, OperationCount[IndexB + 1]) + 1;
				}
				// 此时，NewCount已经成为下一个值的上方值(也就是结果) 
				// 如果两者值相同，则通常左上方值是最小的 
				OperationCount[IndexB] = LastCount;	// 此时，左上方值已经用不到了，可以更新为下一次迭代做准备 
				LastCount = NewCount;	// 更新上方值
			}
			OperationCount[LenB] = LastCount;	// 更新列的最后一个值，收尾
		}
		return OperationCount[LenB];
	}
}
