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
	inline void*	Alloc(int32_t InSize, int32_t InAlign);
	inline void		Free(void* Ptr);

	// Executable alloc policy 
	inline void*	AllocExecutable(int32_t InSize, int32_t InAlign);
	inline void		FreeExecutable(void* Ptr);

	// Container alloc policy
	inline void*	AllocContainer(int32_t InSize, int32_t InAlign);
	inline void		FreeContainer(void* Ptr);

	// Memory operator
	inline void		Memcpy(void* Dest, void* Src, int32_t Size);

	//================================Helper================================
	// JobAllocator 
	template<class T>
	class JobAllocator : public std::allocator<T>
	{
	public:
		using base_type = std::allocator<T>;

		template<class Other>
		struct rebind { using other = JobAllocator<Other>; };

		inline JobAllocator() {}

		inline JobAllocator(JobAllocator<T> const&) {}

		inline JobAllocator<T>& operator=(JobAllocator<T> const&) { return (*this); }

		template<class Other> inline JobAllocator(JobAllocator<Other> const&) {}

		template<class Other> inline JobAllocator<T>& operator=(JobAllocator<Other> const&) { return (*this); }

		inline pointer allocate(size_type count) { return (pointer)AllocContainer(count * sizeof(T), alignof(T)); }
		inline void deallocate(pointer ptr, size_type count) { FreeContainer(ptr); }
	};

	// container
	template<typename T>
	using JobVector = std::vector<T, JobAllocator<T>>;
	using JobString = std::basic_string<wchar_t, std::char_traits<wchar_t>, JobAllocator<wchar_t>>;
	
	// helpers 
	template<typename T,typename...Ts>
	inline T*		JobNew(Ts&&...Args)
	{
		void* Memory = Alloc(sizeof(T), alignof(T));
		return new(Memory)T(std::forward<Ts>(Args)...);
	}
	template<typename T>
	inline void    JobDelete(T* Ptr)
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
	inline void* Alloc(int32_t InSize, int32_t InAlign)
	{
		return PoolMAlloc(InSize, InAlign);
	}
	inline void Free(void* Ptr)
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