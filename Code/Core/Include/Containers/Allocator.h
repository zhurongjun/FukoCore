#pragma once
#include <CoreConfig.h>
#include <CoreType.h>
#include "../Memory/Allocators.h"
#include "../Memory/Memory.h"
#include <corecrt_malloc.h>

// alloc template 
namespace Fuko
{
	class __AllocTemplate
	{
	public:
		using SizeType = int32;
		using USizeType = uint32;

		template<typename T>
		FORCEINLINE SizeType	Free(T*& Data);
		template<typename T>
		FORCEINLINE SizeType	Reserve(T*& Data, SizeType InMax);

		FORCEINLINE SizeType	GetGrow(SizeType InNum, SizeType InMax);
		FORCEINLINE SizeType	GetShrink(SizeType InNum, SizeType InMax);
	};
}

// pmr allocator
namespace Fuko
{
	class PmrAllocator
	{
		IAllocator*		m_Allocator;
	public:
		using SizeType = int32;
		using USizeType = uint32;

		FORCEINLINE PmrAllocator(IAllocator* InAllocator = DefaultAllocator())
			: m_Allocator(InAllocator)
		{
			check(m_Allocator != nullptr);
		}
		FORCEINLINE PmrAllocator(const PmrAllocator&) = default;
		FORCEINLINE PmrAllocator(PmrAllocator&&) = default;
		FORCEINLINE PmrAllocator& operator=(const PmrAllocator&) = default;
		FORCEINLINE PmrAllocator& operator=(PmrAllocator&&) = default;

		template<typename T>
		FORCEINLINE SizeType	Free(T*& Data) { m_Allocator->TFree(Data); Data = nullptr; return 0; }
		template<typename T>
		FORCEINLINE SizeType	Reserve(T*& Data, SizeType InMax)
		{
			if (Data)
			{
				Data = (T*)m_Allocator->TRealloc(Data, InMax);
			}
			else
			{
				Data = (T*)m_Allocator->TAlloc<T>(InMax);
			}
			return InMax;
		}

		FORCEINLINE SizeType	GetGrow(SizeType InNum, SizeType InMax)
		{
			constexpr SizeType FirstGrow = 4;
			constexpr SizeType ConstantGrow = 16;

			SizeType Retval;
			check(InNum > InMax && InNum > 0);

			SizeType Grow = FirstGrow;	// 初次分配空间的内存增长
			if (InMax || InNum > Grow)
			{
				// 计算内存增长
				Grow = InNum + 3 * InNum / 8 + ConstantGrow;
			}
			Retval = Grow;

			// 处理溢出
			if (InNum > Retval) Retval = ULLONG_MAX;
			return Retval;
		}
		FORCEINLINE SizeType	GetShrink(SizeType InNum, SizeType InMax)
		{
			SizeType Retval;
			check(InNum < InMax);

			// 如果闲余空间过多，则刚好收缩到使用空间
			if ((3 * InNum < 2 * InMax) && (InMax - InNum > 64 || !InNum))
			{
				Retval = InNum;
			}
			else
			{
				Retval = InMax;
			}

			return Retval;
		}
	};
}

// base allocator
namespace Fuko
{
	class BaseAlloc
	{
	public:
		using SizeType = int32;
		using USizeType = uint32;

		template<typename T>
		FORCEINLINE SizeType	Free(T*& Data) 
		{ 
			_aligned_free(Data);
			Data = nullptr;
			return 0;
		}
		template<typename T>
		FORCEINLINE SizeType	Reserve(T*& Data, SizeType InMax)
		{
			if (Data)
				Data = _aligned_realloc(Data, InMax * sizeof(T), alignof(T));
			else
				Data = _aligned_malloc(InMax * sizeof(T), alignof(T));
			return InMax;
		}

		FORCEINLINE SizeType	GetGrow(SizeType InNum, SizeType InMax)
		{
			constexpr SizeType FirstGrow = 4;
			constexpr SizeType ConstantGrow = 16;

			SizeType Retval;
			check(InNum > InMax && InNum > 0);

			SizeType Grow = FirstGrow;	// 初次分配空间的内存增长
			if (InMax || InNum > Grow)
			{
				// 计算内存增长
				Grow = InNum + 3 * InNum / 8 + ConstantGrow;
			}
			Retval = Grow;

			// 处理溢出
			if (InNum > Retval) Retval = ULLONG_MAX;
			return Retval;
		}
		FORCEINLINE SizeType	GetShrink(SizeType InNum, SizeType InMax)
		{
			SizeType Retval;
			check(InNum < InMax);

			// 如果闲余空间过多，则刚好收缩到使用空间
			if ((3 * InNum < 2 * InMax) && (InMax - InNum > 64 || !InNum))
			{
				Retval = InNum;
			}
			else
			{
				Retval = InMax;
			}

			return Retval;
		}
	};
}