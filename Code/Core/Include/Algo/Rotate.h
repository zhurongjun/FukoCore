#pragma once
#include <CoreConfig.h>
#include <CoreType.h>
#include <Templates/UtilityTemp.h>
#include "Utility.h"

namespace Fuko::Algo
{
	template<typename T,typename TSize>
	void Rotate(T* First, TSize From, TSize To, TSize Amount)
	{
		if (Amount == 0) return;

		TSize Num = To - From;
		TSize LocGCD = GCD(Num, Amount);
		TSize CycleSize = Num / LocGCD;

		for (TSize i = 0; i < LocGCD; ++i)
		{
			T BufferObject = std::move(First[From + i]);
			TSize IndexToFill = i;

			for (TSize j = 0; j < CycleSize; ++j)
			{
				IndexToFill = (IndexToFill + Amount) % Num;
				Swap(First[From + IndexToFill], BufferObject);
			}
		}
	}
}
