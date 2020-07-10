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

		template<typename OtherT>
		FORCEINLINE __AllocTemplate<OtherT>& Rebind();

		FORCEINLINE SizeType	Free(T*& Data);
		FORCEINLINE SizeType	Reserve(T*& Data, SizeType InMax);

		FORCEINLINE SizeType	GetGrow(SizeType InNum, SizeType InMax);
		FORCEINLINE SizeType	GetShrink(SizeType InNum, SizeType InMax);
	};
}

// pmr allocator
namespace Fuko
{
	template<typename T>
	class TPmrAllocator
	{
		IAllocator*		m_Allocator;
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

		template<typename OtherT>
		FORCEINLINE TPmrAllocator<OtherT>& Rebind() { return (TPmrAllocator<OtherT>&)*this; }

		FORCEINLINE SizeType	Free(T*& Data) { m_Allocator->TFree(Data); Data = nullptr; return 0; }
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

		FORCEINLINE SizeType	GetGrow(SizeType InNum, SizeType InMax) { return (SizeType)m_Allocator->GetGrow(InNum, InMax, sizeof(T), alignof(T)); }
		FORCEINLINE SizeType	GetShrink(SizeType InNum, SizeType InMax) { return (SizeType)m_Allocator->GetShrink(InNum, InMax, sizeof(T), alignof(T)); }
	};
}