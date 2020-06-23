#pragma once
#include "CoreConfig.h"
#include "CoreType.h"

namespace Fuko::Algo::Impl
{
	// 置反
	template <typename T>
	FORCEINLINE void Reverse(T* Array, int32 ArraySize)
	{
		for (int32 i = 0, i2 = ArraySize - 1; i < ArraySize / 2; ++i, --i2)
		{
			Swap(Array[i], Array[i2]);
		}
	}
}

namespace Fuko::Algo
{
	/**
	 * @fn template <typename T, int32 ArraySize> FORCEINLINE void Reverse(T(&Array)[ArraySize])
	 *
	 * @brief 置反数组
	 *
	 * @param [in] Array 数组
	 */
	template <typename T, int32 ArraySize>
	FORCEINLINE void Reverse(T(&Array)[ArraySize])
	{
		return Impl::Reverse((T*)Array, ArraySize);
	}

	/**
	 * @fn template <typename T> FORCEINLINE void Reverse(T* Array, int32 ArraySize)
	 *
	 * @brief 置反数组
	 *
	 * @param [in]     Array	 数组
	 * @param 		   ArraySize 数组大小
	 */
	template <typename T>
	FORCEINLINE void Reverse(T* Array, int32 ArraySize)
	{
		return Impl::Reverse(Array, ArraySize);
	}

	/**
	 * @fn template <typename ContainerType> FORCEINLINE void Reverse(ContainerType& Container)
	 *
	 * @brief 置反容器
	 *
	 * @param [in] 容器
	 */
	template <typename ContainerType>
	FORCEINLINE void Reverse(ContainerType& Container)
	{
		return Impl::Reverse(Container.GetData(), Container.Num());
	}
}

