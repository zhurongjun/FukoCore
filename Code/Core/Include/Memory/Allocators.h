#pragma once
#include <limits>

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
		// alloc 
		virtual void*	Alloc(size_t InSize, size_t Alignment = DEFAULT_ALIGNMENT) = 0;
		virtual void*	Realloc(void* InPtr, size_t InSize, size_t Alignment = DEFAULT_ALIGNMENT) = 0;
		virtual void	Free(void* InPtr) = 0;
		virtual size_t	Size(void* Ptr, size_t Alignment = DEFAULT_ALIGNMENT) = 0;
		virtual void	Trim() = 0;

		// help 
		template<typename T>
		FORCEINLINE T* TAlloc(size_t Count = 1)
		{
			return (T*)Alloc(Count * sizeof(T), alignof(T));
		}
		template<typename T>
		FORCEINLINE T* TRealloc(T* Ptr, size_t Count = 1)
		{
			return (T*)Realloc(Ptr, Count * sizeof(T), alignof(T));
		}
		template<typename T>
		FORCEINLINE void TFree(T* Ptr)
		{
			Free(Ptr);
		}


		// alloc policy 
		virtual size_t GetGrow(size_t InNumElement, size_t InExistElementNum, size_t InElementByte, size_t Alignment = 0) = 0;
		virtual size_t GetShrink(size_t InNumElement, size_t InExistElementNum, size_t InElementByte, size_t Alignment = 0) = 0;
	};
}

// Heap allocator
namespace Fuko
{
	class HeapAllocator : public IAllocator
	{
	public:
		void* Alloc(size_t InSize, size_t Alignment) override
		{
			return _aligned_malloc(InSize, Alignment);
		}
		void* Realloc(void* InPtr, size_t InSize, size_t Alignment) override
		{
			return _aligned_realloc(InPtr, InSize, Alignment);
		}
		void Free(void* InPtr) override
		{
			return _aligned_free(InPtr);
		}
		size_t Size(void* Ptr, size_t Alignment) override
		{
			return _aligned_msize(Ptr, Alignment, 0);
		}
		void Trim() override {}


		size_t GetGrow(size_t InNumElement, size_t InExistElementNum, size_t InElementByte, size_t Alignment) override
		{
			constexpr size_t FirstGrow = 4;
			constexpr size_t ConstantGrow = 16;

			size_t Retval;
			check(InNumElement > InExistElementNum && InNumElement > 0);

			size_t Grow = FirstGrow;	// 初次分配空间的内存增长
			if (InExistElementNum || size_t(InNumElement) > Grow)
			{
				// 计算内存增长
				Grow = InNumElement + 3 * InNumElement / 8 + ConstantGrow;
			}
			Retval = (size_t)Grow;

			// 处理溢出
			if (InNumElement > Retval)
			{
				Retval = ULLONG_MAX;
			}
			return Retval;
		}
		size_t GetShrink(size_t InNumElement, size_t InExistElementNum, size_t InElementByte, size_t Alignment) override
		{
			size_t Retval;
			check(InNumElement < InExistElementNum);

			// 如果闲余空间过多，则刚好收缩到使用空间
			const size_t CurrentSlackElements = InExistElementNum - InNumElement;
			const size_t CurrentSlackBytes = (InExistElementNum - InNumElement)*InElementByte;
			const bool bTooManySlackBytes = CurrentSlackBytes >= 16384;
			const bool bTooManySlackElements = 3 * InNumElement < 2 * InExistElementNum;
			if ((bTooManySlackBytes || bTooManySlackElements) && (CurrentSlackElements > 64 || !InNumElement))
			{
				Retval = InNumElement;
			}
			else
			{
				Retval = InExistElementNum;
			}

			return Retval;
		}
	};
}