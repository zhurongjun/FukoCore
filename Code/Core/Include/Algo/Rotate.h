#pragma once
#include <CoreConfig.h>
#include <CoreType.h>
#include <Templates/UtilityTemp.h>

namespace Fuko::Algo::Impl
{
	/**
	 * @fn template <typename T> int32 RotateInternal(T* First, int32 Num, int32 Count)
	 *
	 * @brief 把指定的元素旋向末尾，这个算法的要点在于Count分割的区域前段和后段的长度，
	 * 		  任意一段先到达Count，首部的Count的元素都会被划入有序区，此时，只需要处理
	 * 		  先到达那一端，保证下一个Head会被正确划入有序区即可，算法以Iter和Mid相碰
	 * 		  结束。
	 *
	 * @param [in]     First 数组头
	 * @param 		   Num   数组长度
	 * @param 		   Count 想要旋转的元素个数
	 *
	 * @returns An int32.
	 */
	template <typename T>
	int32 RotateInternal(T* First, int32 Num, int32 Count)
	{
		if (Count == 0)
		{
			return Num;
		}

		if (Count >= Num)
		{
			return 0;
		}

		T* Iter = First;		// 迭代位置
		T* Mid = First + Count;	// 旋转中心
		T* End = First + Num;	// 数组尾

		T* OldMid = Mid;	// 上一个中心
		for (;;)
		{
			Swap(*Iter++, *Mid++);
			if (Mid == End)
			{
				if (Iter == OldMid)		// 迭代器与中心相碰的时候代表交换完毕
				{
					return Num - Count;
				}

				Mid = OldMid;	// Mid先到达末尾的话，持续把后段元素抛向Iter - OldMid的首部以保证顺序
			}
			else if (Iter == OldMid)
			{
				OldMid = Mid;	// Iter先到达OldMid，持续把OldMid - Mid的元素抛向末尾以保证顺序
			}
		}
	}
}

namespace Fuko::Algo
{

}
