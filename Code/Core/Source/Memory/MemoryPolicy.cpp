#include <Templates/Align.h>
#include <Math/MathUtility.h>
#include <CoreMinimal/Assert.h>
#include <Memory/MemoryOps.h>
#include <Memory/MemoryPolicy.h>
#include <Containers/Pool.h>

namespace Fuko
{
	IAllocator* DefaultAllocator()
	{
		static HeapAllocator s_HeapAllocator;
		return &s_HeapAllocator;
	}

	template<int InSize>
	struct TBlock
	{
		int32		Size;
		int32		Unused1;
		int64		Unused2;
		TAlignedBytes<InSize, 16>	Memory;
		TBlock()
			: Size(InSize)
			, Unused1(0)
			, Unused2(0)
		{}
	};

	static_assert(sizeof(TBlock<32>) == 48);

	// global pool 
	TPool<TBlock<32>, MutexLock, BaseAlloc>		g_Block32(64, 4);
	TPool<TBlock<64>, MutexLock, BaseAlloc>		g_Block64(32, 4);
	TPool<TBlock<128>, MutexLock, BaseAlloc>	g_Block128(16, 4);
	TPool<TBlock<256>, MutexLock, BaseAlloc>	g_Block256(16, 2);
	TPool<TBlock<512>, MutexLock, BaseAlloc>	g_Block512(16, 2);


	CORE_API void*	PoolMAlloc(size_t InSize, size_t InAlign)
	{
		check(InAlign <= 16);
		size_t BlockSize = Math::RoundUpToPowerOfTwo(InSize);
		switch (BlockSize)
		{
		case 0: 
			return nullptr;
		case 2:
		case 4:
		case 8:
		case 16:
		case 32:
			return &g_Block32.New()->Memory;
		case 64:
			return &g_Block64.New()->Memory;
		case 128:
			return &g_Block128.New()->Memory;
		case 256:
			return &g_Block256.New()->Memory;
		case 512:
			return &g_Block512.New()->Memory;
		default:
		{
			int32* RawMemory = (int32*)MAlloc(InSize + 16, 16);
			*RawMemory = (int32)InSize;
			return RawMemory + 4;
		}
		}
	}
	CORE_API void*	PoolRealloc(void* Ptr, size_t InSize, size_t InAlign)
	{
		size_t LastSize = PoolMSize(Ptr);
		size_t BlockSize = Math::RoundUpToPowerOfTwo(InSize);
		if (InSize == 0)
		{
			PoolFree(Ptr);
			return nullptr;
		}
		if (LastSize == BlockSize) return Ptr;
		if (LastSize > 512 && InSize > 512) return Realloc(Ptr, InSize, InAlign);
		void* NewPtr = PoolMAlloc(InSize, InAlign);
		Memcpy(NewPtr, Ptr, Math::Min(LastSize, InSize));
		PoolFree(Ptr);
		return NewPtr;
	}
	CORE_API void	PoolFree(void* Ptr)
	{
		int32* RawPtr = (((int32*)Ptr) - 4);
		int32 BlockSize = *RawPtr;
		switch (BlockSize)
		{
		case 32:
			g_Block32.Free((TBlock<32>*)RawPtr);
			break;
		case 64:
			g_Block64.Free((TBlock<64>*)RawPtr);
			break;
		case 128:
			g_Block128.Free((TBlock<128>*)RawPtr);
			break;
		case 256:
			g_Block256.Free((TBlock<256>*)RawPtr);
			break;
		case 512:
			g_Block512.Free((TBlock<512>*)RawPtr);
		default:
		{
			check(BlockSize > 512);
			Free(RawPtr);
		}
		}
	}
	CORE_API size_t PoolMSize(void* Ptr)
	{
		if (Ptr == 0) return 0;
		int32* RawPtr = (((int32*)Ptr) - 4);
		return *RawPtr;
	}
}
