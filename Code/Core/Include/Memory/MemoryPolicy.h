#pragma once
#include <CoreConfig.h>
#include <CoreType.h>
#include <Misc/Assert.h>
#include <Memory/MemoryOps.h>
#include <Memory/MemoryPolicy.h>
#include <Misc/Assert.h>

// Allocator interface
namespace Fuko
{
	enum
	{
		DEFAULT_ALIGNMENT = 4,
		MIN_ALIGNMENT = 8
	};
	class IAllocator
	{
	public:
		virtual ~IAllocator() {}

		// alloc 
		virtual void*	Alloc(size_t InSize, size_t Alignment = DEFAULT_ALIGNMENT) = 0;
		virtual void*	Realloc(void* InPtr, size_t InSize, size_t Alignment = DEFAULT_ALIGNMENT) = 0;
		virtual void	Free(void* InPtr) = 0;
		virtual size_t	Size(void* Ptr, size_t Alignment = DEFAULT_ALIGNMENT) = 0;
		virtual void	Trim() = 0;

		// help 
		template<typename T>
		FORCEINLINE T* TAlloc(size_t Count = 1) { return (T*)Alloc(Count * sizeof(T), alignof(T)); }
		template<typename T>
		FORCEINLINE T* TRealloc(T* Ptr, size_t Count = 1) { return (T*)Realloc(Ptr, Count * sizeof(T), alignof(T)); }
		template<typename T>
		FORCEINLINE void TFree(T* Ptr) { Free(Ptr); }
	};
}

// Heap allocator
namespace Fuko
{
	class CORE_API HeapAllocator : public IAllocator
	{
		int	m_Count = 0;
	public:
		~HeapAllocator()
		{
			always_check(m_Count == 0);
		}
		void* Alloc(size_t InSize, size_t Alignment) override
		{
			if (InSize != 0) ++m_Count;
			return _aligned_malloc(InSize, Alignment);
		}
		void* Realloc(void* InPtr, size_t InSize, size_t Alignment) override
		{
			if (InSize > 0 && InPtr == nullptr)
				++m_Count;
			else if (InPtr != nullptr && InSize == 0)
				--m_Count;
			return _aligned_realloc(InPtr, InSize, Alignment);
		}
		void Free(void* InPtr) override
		{
			if (InPtr != nullptr)
				--m_Count;
			return _aligned_free(InPtr);
		}
		size_t Size(void* Ptr, size_t Alignment) override
		{
			return _aligned_msize(Ptr, Alignment, 0);
		}
		void Trim() override {}
	};
}

// Alloc 
namespace Fuko
{
	CORE_API IAllocator* DefaultAllocator();

	// Default malloc 
	FORCEINLINE void*	MAlloc(size_t InSize, size_t InAlign) { return DefaultAllocator()->Alloc(InSize, InAlign); }
	FORCEINLINE void*	Realloc(void* Ptr, size_t InSize, size_t InAlign) { return DefaultAllocator()->Realloc(Ptr, InSize, InAlign); }
	FORCEINLINE void	Free(void* Ptr) { DefaultAllocator()->Free(Ptr); }
	FORCEINLINE size_t	MSize(void* Ptr) { return DefaultAllocator()->Size(Ptr); }

	// Pool malloc 
	CORE_API void*	PoolMAlloc(size_t InSize, size_t InAlign);
	CORE_API void*	PoolRealloc(void* Ptr, size_t InSize, size_t InAlign);
	CORE_API void	PoolFree(void* Ptr);
	CORE_API size_t PoolMSize(void* Ptr); 
}
