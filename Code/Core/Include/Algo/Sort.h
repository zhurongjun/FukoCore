#pragma once
#include <CoreConfig.h>
#include <CoreType.h>
#include <Templates/Functor.h>
#include "HeapSort.h"

// Introspective Sort
namespace Fuko::Algo
{
	template <typename T, typename TSize, typename TPred>
	void IntroSort(T* First, TSize Num, TPred&& Pred)
	{
		// 模拟栈
		struct FStack
		{
			T* Min;
			T* Max;
			uint32 MaxDepth;
		};

		// 元素过少
		if (Num < 2)
		{
			return;
		}

		// 构建栈，最高深度为数量的自然对数 * 2
		FStack RecursionStack[32] = { {First, First + Num - 1, (uint32)(FMath::Loge((float)Num) * 2.f)} }, Current, Inner;

		// 开始内省排序
		for (FStack* StackTop = RecursionStack; StackTop >= RecursionStack; --StackTop)
		{
			Current = *StackTop;	// 保存栈顶

		Loop:
			TSize Count = (TSize)(Current.Max - Current.Min + 1);	// 当前栈中的元素数量

			// 递归过深，使用堆排序
			if (Current.MaxDepth == 0)
			{
				HeapSort(Current.Min, Count, std::forward<TPred>(Pred));
				continue;
			}

			if (Count <= 8)
			{
				// 数据量太少了，直接冒泡排序
				while (Current.Max > Current.Min)
				{
					T *Max, *Item;
					for (Max = Current.Min, Item = Current.Min + 1; Item <= Current.Max; Item++)
					{
						if (Pred(*Max, *Item))
						{
							Max = Item;
						}
					}
					Swap(*Max, *Current.Max--);
				}
			}
			else
			{
				// 将中心元素交换至首部，以首部作为基准的存储位置
				Swap(Current.Min[Count / 2], Current.Min[0]);

				// Max+1以使用更加高效的前--
				Inner.Min = Current.Min;
				Inner.Max = Current.Max + 1;
				for (; ; )
				{
					// 从左向右迭代寻找 大/小 于基准的元素
					while (++Inner.Min <= Current.Max && !Pred(*Current.Min, *Inner.Min));
					// 从右向左迭代寻找 小/大 于基准的元素
					while (--Inner.Max > Current.Min && !Pred(*Inner.Max, *Current.Min));
					// 此次迭代完毕，退出
					if (Inner.Min > Inner.Max)
					{
						break;
					}
					// 交换，使得基准左侧元素总 大/小 于基准的元素
					Swap(*Inner.Min, *Inner.Max);
				}
				// 将基准重新塞回中心 
				Swap(*Current.Min, *Inner.Max);

				// 此时，准备开始递归，减少MaxDepth
				--Current.MaxDepth;

				if (Inner.Max - 1 - Current.Min >= Current.Max - Inner.Min)
				{
					// 左侧元素较多，存储左侧，使用右侧递归
					if (Current.Min + 1 < Inner.Max)
					{
						StackTop->Min = Current.Min;
						StackTop->Max = Inner.Max - 1;
						StackTop->MaxDepth = Current.MaxDepth;
						StackTop++;
					}
					// 如果右侧还有元素，就使用右侧递归
					if (Current.Max > Inner.Min)
					{
						Current.Min = Inner.Min;
						goto Loop;
					}
				}
				else
				{
					// 右侧元素较多，存储右侧，使用左侧递归
					if (Current.Max > Inner.Min)
					{
						StackTop->Min = Inner.Min;
						StackTop->Max = Current.Max;
						StackTop->MaxDepth = Current.MaxDepth;
						StackTop++;
					}
					// 如果左侧还有元素，就使用左侧递归
					if (Current.Min + 1 < Inner.Max)
					{
						Current.Max = Inner.Max - 1;
						goto Loop;
					}
				}
			}
		}
	}
}
