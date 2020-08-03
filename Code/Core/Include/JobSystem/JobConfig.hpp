#pragma once
#include <mutex>
#include <atomic>
#include <stdint.h>
#include <assert.h>

// Job config policy 
namespace Fuko::Job
{
	// Executable alloc policy 
	void*	AllocExecutable(int32_t InSize, int32_t InAlign);
	void	FreeExecutable(void* Ptr);

	// Container alloc policy
	void*	AllocContainer(int32_t InSize, int32_t InAlign);
	void	FreeContainer(void* Ptr);

	// Memory operator
	void	Memcpy(void* Dest, void* Src, int32_t Size);
}

// Fuko core policy 
#include <Memory/MemoryOps.h>
namespace Fuko::Job
{
	FORCEINLINE void* AllocExecutable(int32_t InSize, int32_t InAlign)
	{ 
		return _aligned_malloc(InSize, InAlign); 
	}
	FORCEINLINE void FreeExecutable(void* Ptr)
	{
		_aligned_free(Ptr);
	}
	FORCEINLINE void* AllocContainer(int32_t InSize, int32_t InAlign)
	{
		return _aligned_malloc(InSize, InAlign);
	}
	FORCEINLINE void FreeContainer(void* Ptr)
	{
		_aligned_free(Ptr);
	}

	FORCEINLINE void Memcpy(void* Dest, void* Src, int32_t Size)
	{
		::Fuko::Memcpy(Dest, Src, Size);
	}
}