#pragma once
#include <mutex>
#include <atomic>
#include <stdint.h>
#include <assert.h>
#include <future>
#include <vector>
#include <string>

#define JobAssert(Pred) assert(Pred)

// Job config  
namespace Fuko::Job
{
	// Normal alloc policy
	void*	Alloc(int32_t InSize, int32_t InAlign);
	void	Free(void* Ptr);

	// Executable alloc policy 
	void*	AllocExecutable(int32_t InSize, int32_t InAlign);
	void	FreeExecutable(void* Ptr);

	// Container alloc policy
	void*	AllocContainer(int32_t InSize, int32_t InAlign);
	void	FreeContainer(void* Ptr);

	// Memory operator
	void	Memcpy(void* Dest, void* Src, int32_t Size);

	//================================Helper================================
	// JobAllocator 
	template<class T>
	class JobAllocator : public std::allocator<T>
	{
	public:
		using base_type = std::allocator<T>;

		template<class Other>
		struct rebind { using other = JobAllocator<Other>; };

		JobAllocator() {}

		JobAllocator(JobAllocator<T> const&) {}

		JobAllocator<T>& operator=(JobAllocator<T> const&) { return (*this); }

		template<class Other> JobAllocator(JobAllocator<Other> const&) {}

		template<class Other> JobAllocator<T>& operator=(JobAllocator<Other> const&) { return (*this); }

		pointer allocate(size_type count) { return (pointer)AllocContainer(count * sizeof(T), alignof(T)); }
		void deallocate(pointer ptr, size_type count) { FreeContainer(ptr); }
	};

	// container
	template<typename T>
	using JobVector = std::vector<T, JobAllocator<T>>;
	using JobString = std::basic_string<wchar_t, std::char_traits<wchar_t>, JobAllocator<wchar_t>>;
	
	// helpers 
	template<typename T,typename...Ts>
	T*		JobNew(Ts&&...Args)
	{
		void* Memory = Alloc(sizeof(T), alignof(T));
		return new(Memory)T(std::forward<Ts>(Args)...);
	}
	template<typename T>
	void    JobDelete(T* Ptr)
	{
		Ptr->~T();
		Free(Ptr);
	}
}

//================================Fuko core policy================================
#include <Memory/MemoryOps.h>
#include <Memory/MemoryPolicy.h>
namespace Fuko::Job
{
	void* Alloc(int32_t InSize, int32_t InAlign)
	{
		return PoolMAlloc(InSize, InAlign);
	}
	void Free(void* Ptr)
	{
		PoolFree(Ptr);
	}
	FORCEINLINE void* AllocExecutable(int32_t InSize, int32_t InAlign)
	{ 
		return PoolMAlloc(InSize, InAlign);
	}
	FORCEINLINE void FreeExecutable(void* Ptr)
	{
		PoolFree(Ptr);
	}
	FORCEINLINE void* AllocContainer(int32_t InSize, int32_t InAlign)
	{
		return PoolMAlloc(InSize, InAlign);
	}
	FORCEINLINE void FreeContainer(void* Ptr)
	{
		PoolFree(Ptr);
	}

	FORCEINLINE void Memcpy(void* Dest, void* Src, int32_t Size)
	{
		::Fuko::Memcpy(Dest, Src, Size);
	}
}