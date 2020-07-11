#pragma once
#include <CoreConfig.h>
#include <CoreType.h>
#include "../Memory/Allocators.h"
#include "../Memory/Memory.h"

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

		FORCEINLINE SizeType	GetGrow(SizeType InNum, SizeType InMax) { return (SizeType)m_Allocator->GetGrow(InNum, InMax, 1, 1); }
		FORCEINLINE SizeType	GetShrink(SizeType InNum, SizeType InMax) { return (SizeType)m_Allocator->GetShrink(InNum, InMax, 1, 1); }
	};
}