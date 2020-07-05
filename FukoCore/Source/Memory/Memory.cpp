#include "Memory/Memory.h"
#include "Templates/Align.h"
#include "Math/MathUtility.h"
#include "CoreMinimal/Assert.h"

void* Fuko::Malloc(size_t Size)
{
	return malloc(Size);
}

void* Fuko::Realloc(void * Ptr, size_t Size)
{
	return realloc(Ptr, Size);
}

void Fuko::Free(void * Ptr)
{
	free(Ptr);
}

void * Fuko::AlignedMalloc(size_t Size, uint32 Alignment)
{
	return _aligned_malloc(Size, Alignment);
}

void * Fuko::AlignedRealloc(void * Ptr, size_t Size, uint32 Alignment)
{
	return _aligned_realloc(Ptr, Size, Alignment);
}

void Fuko::AlignedFree(void * Ptr)
{
	_aligned_free(Ptr);
}

size_t Fuko::QuantizeSize(size_t Size, uint32 Alignment)
{
	check((Alignment & (Alignment - 1)) == 0);
	return Alignment == 0 ?
		Size :
		Size + Alignment - 1;
}

void Fuko::Trim()
{
}

namespace Fuko
{
	IAllocator* DefaultAllocator()
	{
		return nullptr;
	}
}
