#pragma once
#include <CoreConfig.h>
#include <CoreType.h>
#include <Templates/UtilityTemp.h>
#include "Utility.h"

namespace Fuko::Algo
{
	template<typename T,typename TSize>
	void Rotate(T* First, TSize Num, TSize Amount)
	{
		if (Amount == 0) return;

		TSize LocGCD = GCD(Num, Amount);
		// 一个完全剩余系的大小 
		TSize CycleSize = Num / LocGCD;
		
		for (TSize i = 0; i < LocGCD; ++i)
		{
			T BufferObject = std::move(First[i]);
			TSize IndexToFill = i;
			// 完全剩余系内各值向后Amount个元素，循环过后，这个完全剩余系的元素即完成移动
			for (TSize j = 0; j < CycleSize; ++j)
			{
				IndexToFill = (IndexToFill + Amount) % Num;
				Swap(First[IndexToFill], BufferObject);
			}
		}
	}

	template<typename T,typename TSize>
	FORCEINLINE void Rotate(T* First, TSize From, TSize To, TSize Amount)
	{
		return Rotate(First + From, To - From, Amount);
	}
}
