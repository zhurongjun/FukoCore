#pragma once

// Pmr Allocator
namespace Fuko
{
	template<typename T>
	class PmrAllocator
	{
	public:
		// construct 
		PmrAllocator(IAllocator* InAllocator) : m_Allocator(InAllocator) { check(m_Allocator); }
		// copy construct 
		PmrAllocator(const PmrAllocator&) = delete;
		// move construct 
		PmrAllocator(PmrAllocator&& Rhs) : m_Allocator(Rhs.m_Allocator) { Rhs.m_Allocator = nullptr; check(m_Allocator);  }

		// assign operator 
		PmrAllocator& operator=(const PmrAllocator&) = delete;
		// move assign operator
		PmrAllocator& operator=(PmrAllocator&& Rhs)
		{
			m_Allocator = Rhs.m_Allocator;
			Rhs.m_Allocator = nullptr;
			check(m_Allocator);
		}

		

	private:
		IAllocator*		m_Allocator;
		void*			m_Allocation;
	};

}

// Allocator interface
namespace Fuko
{
	class IAllocator
	{
	public:
		// alloc function 
		virtual void* Alloc(size_t InSize) = 0;
		virtual void* Realloc(void* InPtr, size_t InSize) = 0;
		virtual void  Free(void* InPtr) = 0;
		virtual void  Size(size_t InSize) = 0;

		// aligned alloc 
		virtual void* AlignedAlloc(size_t InSize, size_t Alignment) = 0;
		virtual void* AlignedRealloc(void* InPtr, size_t InSize, size_t Alignment) = 0;
		virtual void  AlignedFree(void* InPtr) = 0;
		virtual void  AlignedSize(size_t InSize, size_t Alignment) = 0;

		// alloc policy 
		virtual size_t GetReserve(size_t InNumElement, size_t InElementByte) = 0;
		virtual size_t GetGrow(size_t InNumElement, size_t InExistElementNum, size_t InElementByte) = 0;
		virtual size_t GetSlack(size_t InNumElement, size_t InExistElementNum, size_t InElementByte) = 0;
	};
}

// Heap allocator
namespace Fuko
{

}