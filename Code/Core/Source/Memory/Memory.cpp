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

	TPool<TAlignedBytes<32, 16>, MutexLock>		g_Block32(64, 4);
	TPool<TAlignedBytes<64, 16>, MutexLock>		g_Block64(32, 4);
	TPool<TAlignedBytes<128, 16>, MutexLock>	g_Block128(16, 4);
	TPool<TAlignedBytes<256, 16>, MutexLock>	g_Block256(16, 2);

	CORE_API void*	RequirBlock(size_t BlockSize)
	{
		check(Math::IsPowerOfTwo(BlockSize) || BlockSize > 256);
		switch (BlockSize)
		{
		case 2:
		case 4:
		case 8:
		case 16:
		case 32: 
			return g_Block32.Alloc();
		case 64: 
			return g_Block64.Alloc();
		case 128:
			return g_Block128.Alloc();
		case 256:
			return g_Block256.Alloc();
		default:
			checkNoEntry();
		}
		return  nullptr;
	}
	CORE_API bool	ReleaseBlock(void* Block)
	{
		if (g_Block32.IsInPool(Block))
		{
			g_Block32.Free((TAlignedBytes<32, 16>*)Block);
		}
		else if (g_Block64.IsInPool(Block))
		{
			g_Block64.Free((TAlignedBytes<64, 16>*)Block);
		}
		else if (g_Block128.IsInPool(Block))
		{
			g_Block128.Free((TAlignedBytes<128, 16>*)Block);
		}
		else if (g_Block256.IsInPool(Block))
		{
			g_Block256.Free((TAlignedBytes<256, 16>*)Block);
		}
		else
		{
			return false;
		}
		return true;
	}

	
}
