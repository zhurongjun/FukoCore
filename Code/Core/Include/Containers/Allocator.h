#pragma once
#include <CoreConfig.h>
#include <CoreType.h>
#include <Memory/MemoryPolicy.h>
#include <Memory/MemoryOps.h>
#include <corecrt_malloc.h>
#include <Misc/Assert.h>

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
		FORCEINLINE SizeType	FreeRaw(void*& Data, SizeType InAlign);
		FORCEINLINE SizeType	ReserveRaw(void*& Data, SizeType InSize, SizeType InAlign);

		FORCEINLINE SizeType	GetGrow(SizeType InNum, SizeType InMax);
		FORCEINLINE SizeType	GetShrink(SizeType InNum, SizeType InMax);
	};

	template<typename SizeType>
	class __DefaultChangePolicy
	{
	public:
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

// pmr allocator
namespace Fuko
{
	class PmrAlloc : public __DefaultChangePolicy<int32>
	{
		IAllocator*		m_Allocator;
	public:
		using SizeType = int32;
		using USizeType = uint32;

		FORCEINLINE PmrAlloc(IAllocator* InAllocator = DefaultAllocator())
			: m_Allocator(InAllocator)
		{
			check(m_Allocator != nullptr);
		}
		FORCEINLINE PmrAlloc(const PmrAlloc&) = default;
		FORCEINLINE PmrAlloc(PmrAlloc&&) = default;
		FORCEINLINE PmrAlloc& operator=(const PmrAlloc&) = default;
		FORCEINLINE PmrAlloc& operator=(PmrAlloc&&) = default;

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
		FORCEINLINE SizeType	FreeRaw(void*& Data, SizeType InAlign)
		{
			m_Allocator->Free(Data);
			return 0;
		}
		FORCEINLINE SizeType	ReserveRaw(void*& Data, SizeType InSize, SizeType InAlign)
		{
			if (Data)
			{
				Data = m_Allocator->Realloc(Data, InSize, InAlign);
			}
			else
			{
				Data = m_Allocator->Alloc(InSize, InAlign);
			}
			return InSize;
		}
	};
}

// base allocator
namespace Fuko
{
	class BaseAlloc : public __DefaultChangePolicy<int32>
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
				Data = (T*)_aligned_realloc(Data, InMax * sizeof(T), alignof(T));
			else
				Data = (T*)_aligned_malloc(InMax * sizeof(T), alignof(T));
			return InMax;
		}
		FORCEINLINE SizeType	FreeRaw(void*& Data, SizeType InAlign)
		{
			_aligned_free(Data);
			Data = nullptr;
			return 0;
		}
		FORCEINLINE SizeType	ReserveRaw(void*& Data, SizeType InSize, SizeType InAlign)
		{
			if (Data)
				Data = _aligned_realloc(Data, InSize, InAlign);
			else
				Data = _aligned_malloc(InSize, InAlign);
			return InSize;
		}
	};
}

// block allocator
namespace Fuko
{
	class BlockAlloc : public __DefaultChangePolicy<int32>
	{
	public:
		using SizeType = int32;
		using USizeType = uint32;

		FORCEINLINE SizeType	FreeRaw(void*& Data, SizeType InAlign)
		{
			PoolFree(Data);
			return 0;
		}
		FORCEINLINE SizeType	ReserveRaw(void*& Data, SizeType InSize, SizeType InAlign)
		{
			if (Data)
			{
				Data = PoolRealloc(Data, InSize, InAlign);
			}
			else
			{
				Data = PoolMAlloc(InSize, InAlign);
			}
			return PoolMSize(Data);
		}

		template<typename T>
		FORCEINLINE SizeType	Free(T*& Data) { return FreeRaw((void*&)Data, alignof(T)) / sizeof(T); }
		template<typename T>
		FORCEINLINE SizeType	Reserve(T*& Data, SizeType InMax) { return ReserveRaw((void*&)Data, InMax * sizeof(T), alignof(T)) / sizeof(T); }
	};
}