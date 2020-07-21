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

	TPool<TBlock<32>, MutexLock>		g_Block32(64, 4);
	TPool<TBlock<64>, MutexLock>		g_Block64(32, 4);
	TPool<TBlock<128>, MutexLock>		g_Block128(16, 4);
	TPool<TBlock<256>, MutexLock>		g_Block256(16, 2);

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
			return &g_Block32.New()->Memory;
		case 64: 
			return &g_Block64.New()->Memory;
		case 128:
			return &g_Block128.New()->Memory;
		case 256:
			return &g_Block256.New()->Memory;
		default:
			checkNoEntry();
		}
		return  nullptr;
	}
	CORE_API bool	ReleaseBlock(void* Block)
	{
		int32* RawPtr = (((int32*)Block) - 4);
		int32 BlockSize = *RawPtr;
		switch (BlockSize)
		{
		case 2:
		case 4:
		case 8:
		case 16:
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
		default:
			return false;
		}
		return true;
	}

	
}
