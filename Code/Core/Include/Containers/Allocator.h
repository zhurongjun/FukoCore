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

		FORCEINLINE __AllocTemplate() = default;
		FORCEINLINE __AllocTemplate(__AllocTemplate&&) = default;
		FORCEINLINE __AllocTemplate& operator=(__AllocTemplate&&) = default;

		FORCEINLINE T*			Data();
		FORCEINLINE SizeType	Free();

		FORCEINLINE SizeType	Grow(SizeType InNum, SizeType InMax);
		FORCEINLINE SizeType	Shrink(SizeType InNum, SizeType InMax);
		FORCEINLINE SizeType	Reserve(SizeType InMax);
		FORCEINLINE SizeType	GetCount(SizeType InNum) const;
	};
}

// pmr allocator
namespace Fuko
{
	template<typename T>
	class PmrAllocator
	{
		IAllocator*	m_Allocator;
		T*			m_Data;
	public:
		using SizeType = int32;

		FORCEINLINE PmrAllocator(IAllocator* InAllocator = DefaultAllocator())
			: m_Allocator(InAllocator)
			, m_Data(nullptr)
		{
			check(m_Allocator != nullptr);
		}
		FORCEINLINE PmrAllocator(PmrAllocator&& Other)
			: m_Allocator(Other.m_Allocator)
			, m_Data(Other.m_Data)
		{
			Other.m_Data = nullptr;
		}
		FORCEINLINE PmrAllocator& operator=(PmrAllocator&& Rhs)
		{
			Free();
			m_Allocator = Rhs.m_Allocator;
			m_Data = Rhs.m_Data;
			Rhs.m_Data = nullptr;
			return *this;
		}

		FORCEINLINE T*			Data() { return m_Data; }
		FORCEINLINE SizeType	Free() { if (m_Data) m_Allocator->TFree(m_Data); m_Data = nullptr; return 0; }
		
		FORCEINLINE SizeType	Grow(SizeType InNum, SizeType InMax)
		{
			SizeType GrowNum = (SizeType)m_Allocator->GetGrow(InNum, InMax, sizeof(T), alignof(T));
			if (GrowNum > InMax) m_Data = m_Allocator->TRealloc(m_Data, GrowNum);
			return GrowNum;
		}
		FORCEINLINE SizeType	Shrink(SizeType InNum, SizeType InMax)
		{
			SizeType ShrinkNum = (SizeType)m_Allocator->GetShrink(InNum, InMax, sizeof(T), alignof(T));
			if (ShrinkNum < InMax) m_Data = m_Allocator->TRealloc(m_Data, ShrinkNum);
			return ShrinkNum;
		}
		FORCEINLINE SizeType	Reserve(SizeType InMax)
		{
			m_Data = m_Allocator->TRealloc(m_Data, InMax);
			return InMax;
		}
		FORCEINLINE SizeType	GetCount(SizeType InNum) const 
		{
			return InNum;
		}
	};
}

// runtime inline allocator
namespace Fuko
{
	template<typename T>
	class RuntimeInlineAllocator
	{
		T*			m_Data;
		uint32		m_Size;
	public:
		using SizeType = int32;

		FORCEINLINE RuntimeInlineAllocator() :m_Data(nullptr), m_Size(0) {}
		FORCEINLINE RuntimeInlineAllocator(T* Data, SizeType Size) : m_Data(Data), m_Size(Size) {}
		FORCEINLINE RuntimeInlineAllocator(RuntimeInlineAllocator&& Other) 
			: m_Data(Other.m_Data)
			, m_Size(Other.m_Size)
		{
			Other.m_Data = nullptr;
			Other.m_Size = 0;
		}
		FORCEINLINE RuntimeInlineAllocator& operator=(RuntimeInlineAllocator&& Other)
		{
			m_Data = Other.m_Data;
			m_Size = Other.m_Size;
			Other.m_Data = nullptr;
			Other.m_Size = 0;
		}

		FORCEINLINE T*			Data() { return m_Data; }
		FORCEINLINE SizeType	Free() { return m_Size; }

		FORCEINLINE SizeType	Grow(SizeType InNum, SizeType InMax) { check(InNum <= m_Size); return m_Size; }
		FORCEINLINE SizeType	Shrink(SizeType InNum, SizeType InMax) { check(InMax) <= m_Size; return m_Size; }
		FORCEINLINE SizeType	Reserve(SizeType InMax) { check(InMax <= m_Size); return m_Size; }
		FORCEINLINE SizeType	GetCount(SizeType InNum) const { return m_Size; }

		FORCEINLINE void	SetData(T* Data, SizeType Size) { m_Data = Data; m_Size = Size; }
	};
}
