#pragma once
#include <stdint.h>
#include <limits>
#include "CoreConfig.h"
#include "Memory/Memory.h"
#include "Templates/Align.h"
#include "Math/MathUtility.h"

// HeapAllocator: 堆分配器
// InlineAllocator: 自身分配了一部分内存，只有在超出这个内存的时候才会把数据转移到备选分配器上
// NonRelocatableInlineAllocator: 没有备选分配器，直接从堆开内存
// FixedAllocator: 固定分配器，resize? 不存在的
// SetAlloctor: 封装给set使用的Alloctor
// SparseArrayAllocator: 用来给稀疏数组使用的Allocator

// 空间增长，收缩算法
namespace Fuko
{
	/**
	 * @fn FORCEINLINE int32_t DefaultCalculateSlackShrink(int32_t NumElements, int32_t NumAllocatedElements, size_t BytesPerElement, bool bAllowQuantize, int32_t Alignment = DEFAULT_ALIGNMENT)
	 *
	 * @brief 收缩元素个数的默认算法
	 *
	 * @date 2020/6/18
	 *
	 * @param  NumElements		    当前元素个数
	 * @param  NumAllocatedElements 当前空间容纳的元素个数
	 * @param  BytesPerElement	    每个元素的大小
	 * @param  bAllowQuantize	    是否使用对齐来校准元素数量
	 * @param  Alignment		    内存对齐
	 *
	 * @returns 元素个数
	 */
	template<typename SizeType>
	FORCEINLINE SizeType DefaultCalculateSlackShrink(SizeType NumElements, SizeType NumAllocatedElements, size_t BytesPerElement, bool bAllowQuantize, uint32 Alignment = DEFAULT_ALIGNMENT)
	{
		SizeType Retval;
		check(NumElements < NumAllocatedElements);

		// 如果闲余空间过多，则刚好收缩到使用空间
		const SizeType CurrentSlackElements = NumAllocatedElements - NumElements;
		const size_t CurrentSlackBytes = (NumAllocatedElements - NumElements)*BytesPerElement;
		const bool bTooManySlackBytes = CurrentSlackBytes >= 16384;
		const bool bTooManySlackElements = 3 * NumElements < 2 * NumAllocatedElements;
		if ((bTooManySlackBytes || bTooManySlackElements) && (CurrentSlackElements > 64 || !NumElements))
		{
			Retval = NumElements;
			if (Retval > 0)
			{
				if (bAllowQuantize)
				{
					Retval = (SizeType)QuantizeSize(Retval * BytesPerElement, Alignment) / BytesPerElement;
				}
			}
		}
		else
		{
			Retval = NumAllocatedElements;
		}

		return Retval;
	}

	/**
	 * @fn FORCEINLINE int32_t DefaultCalculateSlackGrow(int32_t NumElements, int32_t NumAllocatedElements, size_t BytesPerElement, bool bAllowQuantize, int32_t Alignment = DEFAULT_ALIGNMENT)
	 *
	 * @brief 增长元素个数的默认算法
	 *
	 * @date 2020/6/18
	 *
	 * @param  NumElements		    当前元素个数
	 * @param  NumAllocatedElements 当前空间容纳的元素个数
	 * @param  BytesPerElement	    每个元素的大小
	 * @param  bAllowQuantize	    是否使用对齐来校准元素数量
	 * @param  Alignment		    内存对齐
	 *
	 * @returns 元素个数
	 */
	template<typename SizeType>
	FORCEINLINE SizeType DefaultCalculateSlackGrow(SizeType NumElements, SizeType NumAllocatedElements, size_t BytesPerElement, bool bAllowQuantize, uint32 Alignment = DEFAULT_ALIGNMENT)
	{
		constexpr size_t FirstGrow = 4;
		constexpr size_t ConstantGrow = 16;
		
		SizeType Retval;
		check(NumElements > NumAllocatedElements && NumElements > 0);

		size_t Grow = FirstGrow;	// 初次分配空间的内存增长
		if (NumAllocatedElements || size_t(NumElements) > Grow)
		{
			// 计算内存增长
			Grow = size_t(NumElements) + 3 * size_t(NumElements) / 8 + ConstantGrow;
		}
		// 对齐内存
		if (bAllowQuantize)
		{
			Retval = (SizeType)QuantizeSize(Grow * BytesPerElement, Alignment) / BytesPerElement;
		}
		else
		{
			Retval = (SizeType)Grow;
		}
		// 处理溢出
		if (NumElements > Retval)
		{
			Retval = std::numeric_limits<SizeType>::max();
		}
		return Retval;
	}

	/**
	 * @fn FORCEINLINE int32_t DefaultCalculateSlackReserve(int32_t NumElements, size_t BytesPerElement, bool bAllowQuantize, int32_t Alignment = DEFAULT_ALIGNMENT)
	 *
	 * @brief 计算元素实际空间占用的默认算法
	 *
	 * @date 2020/6/18
	 *
	 * @param  NumElements	   当前元素个数
	 * @param  BytesPerElement 每个元素的大小
	 * @param  bAllowQuantize  是否使用对齐来校准元素数量
	 * @param  Alignment	   内存对齐
	 *
	 * @returns 元素个数
	 */
	template<typename SizeType>
	FORCEINLINE SizeType DefaultCalculateSlackReserve(SizeType NumElements, size_t BytesPerElement, bool bAllowQuantize, uint32 Alignment = DEFAULT_ALIGNMENT)
	{
		check(NumElements > 0);

		SizeType Retval = NumElements;
		if (bAllowQuantize)
		{
			Retval = (SizeType)QuantizeSize(size_t(Retval) * size_t(BytesPerElement), Alignment) / BytesPerElement;
			// 处理溢出
			if (NumElements > Retval)
			{
				Retval = std::numeric_limits<SizeType>::max();
			}
		}

		return Retval;
	}
}


// TSetAllocator,给Set用的Allocator
namespace Fuko
{
	#define DEFAULT_NUMBER_OF_ELEMENTS_PER_HASH_BUCKET	2
	#define DEFAULT_BASE_NUMBER_OF_HASH_BUCKETS			8
	#define DEFAULT_MIN_NUMBER_OF_HASHED_ELEMENTS		4

}

