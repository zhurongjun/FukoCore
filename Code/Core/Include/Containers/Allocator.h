#pragma once
#include <CoreConfig.h>
#include <CoreType.h>
#include "../Memory/Allocators.h"
#include "../Memory/Memory.h"

// alloc template 
namespace Fuko
{
	template<typename T>
	class __AllocTemplate
	{
	public:
		using SizeType = int32;

		template<int Flag>
		FORCEINLINE auto		Rebind();

		FORCEINLINE SizeType	Free(T*& Data);
		FORCEINLINE SizeType	Reserve(T*& Data, SizeType InMax);

		FORCEINLINE SizeType	GetGrow(SizeType InNum, SizeType InMax);
		FORCEINLINE SizeType	GetShrink(SizeType InNum, SizeType InMax);
		FORCEINLINE SizeType	GetCount(SizeType InNum) const;
	};
}

// pmr allocator
namespace Fuko
{
	template<typename T>
	class TPmrAllocator
	{
		IAllocator*	m_Allocator;
	public:
		using SizeType = int32;

		FORCEINLINE TPmrAllocator(IAllocator* InAllocator = DefaultAllocator())
			: m_Allocator(InAllocator)
		{
			check(m_Allocator != nullptr);
		}
		FORCEINLINE TPmrAllocator(TPmrAllocator&& Other)
			: m_Allocator(Other.m_Allocator)
		{}
		FORCEINLINE TPmrAllocator& operator=(TPmrAllocator&& Rhs)
		{
			m_Allocator = Rhs.m_Allocator;
			return *this;
		}

		FORCEINLINE SizeType	Free(T*& Data) { m_Allocator->TFree(Data); Data = nullptr; return 0; }
		FORCEINLINE SizeType	Reserve(T*& Data,SizeType InMax)
		{
			if (Data) Data = m_Allocator->TRealloc(Data, InMax);
			else Data = m_Allocator->TAlloc<T>(InMax);
			return InMax;
		}
		
		FORCEINLINE SizeType	GetGrow(SizeType InNum, SizeType InMax)
		{
			return (SizeType)m_Allocator->GetGrow(InNum, InMax, sizeof(T), alignof(T));
		}
		FORCEINLINE SizeType	GetShrink(SizeType InNum, SizeType InMax)
		{
			return (SizeType)m_Allocator->GetShrink(InNum, InMax, sizeof(T), alignof(T));
		}
		FORCEINLINE SizeType	GetCount(SizeType InNum) const
		{
			return InNum;
		}
	};
}
