#pragma once

// Allocator interface
namespace Fuko
{
	class IAllocator
	{
	public:
		// alloc 
		virtual void* Alloc(size_t InSize, size_t Alignment = 0) = 0;
		virtual void* Realloc(void* InPtr, size_t InSize, size_t Alignment = 0) = 0;
		virtual void  Free(void* InPtr) = 0;
		virtual void  Size(size_t InSize, size_t Alignment = 0) = 0;

		// alloc policy 
		virtual size_t GetGrow(size_t InNumElement, size_t InExistElementNum, size_t InElementByte, size_t Alignment = 0) = 0;
		virtual size_t GetShrink(size_t InNumElement, size_t InExistElementNum, size_t InElementByte, size_t Alignment = 0) = 0;
	};
}

// Heap allocator
namespace Fuko
{

}