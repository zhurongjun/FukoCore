#pragma once
#include <CoreConfig.h>
#include <CoreType.h>
#include <Memory/MemoryPolicy.h>
#include <Memory/MemoryOps.h>
#include <corecrt_malloc.h>

// alloc template 
namespace Fuko
{
	class __AllocTemplate
	{
	public:
		using SizeType = int32;
		using USizeType = uint32;

		template<typename T>
		FORCEINLINE SizeType	Free(T*& Data);
		template<typename T>
		FORCEINLINE SizeType	Reserve(T*& Data, SizeType InMax);
		FORCEINLINE SizeType	FreeRaw(void*& Data, SizeType InAlign);
		FORCEINLINE SizeType	ReserveRaw(void*& Data, SizeType InSize, SizeType InAlign);

		FORCEINLINE SizeType	GetGrow(SizeType InNum, SizeType InMax);
		FORCEINLINE SizeType	GetShrink(SizeType InNum, SizeType InMax);
	};
}

// pmr allocator
namespace Fuko
{
	class PmrAlloc
	{
		IAllocator*		m_Allocator;
	public:
		using SizeType = int32;
		using USizeType = uint32;

		FORCEINLINE PmrAlloc(IAllocator* InAllocator = DefaultAllocator())
			: m_Allocator(InAllocator)
		{
			check(m_Allocator != nullptr);
		}
		FORCEINLINE PmrAlloc(const PmrAlloc&) = default;
		FORCEINLINE PmrAlloc(PmrAlloc&&) = default;
		FORCEINLINE PmrAlloc& operator=(const PmrAlloc&) = default;
		FORCEINLINE PmrAlloc& operator=(PmrAlloc&&) = default;

		template<typename T>
		FORCEINLINE SizeType	Free(T*& Data) { m_Allocator->TFree(Data); Data = nullptr; return 0; }
		template<typename T>
		FORCEINLINE SizeType	Reserve(T*& Data, SizeType InMax)
		{
			if (Data)
			{
				Data = (T*)m_Allocator->TRealloc(Data, InMax);
			}
			else
			{
				Data = (T*)m_Allocator->TAlloc<T>(InMax);
			}
			return InMax;
		}
		FORCEINLINE SizeType	FreeRaw(void*& Data, SizeType InAlign)
		{
			m_Allocator->Free(Data);
			return 0;
		}
		FORCEINLINE SizeType	ReserveRaw(void*& Data, SizeType InSize, SizeType InAlign)
		{
			if (Data)
			{
				Data = m_Allocator->Realloc(Data, InSize, InAlign);
			}
			else
			{
				Data = m_Allocator->Alloc(InSize, InAlign);
			}
			return InSize;
		}

		FORCEINLINE SizeType	GetGrow(SizeType InNum, SizeType InMax)
		{
			constexpr SizeType FirstGrow = 4;
			constexpr SizeType ConstantGrow = 16;

			SizeType Retval;
			check(InNum > InMax && InNum > 0);

			SizeType Grow = FirstGrow;	// 初次分配空间的内存增长
			if (InMax || InNum > Grow)
			{
				// 计算内存增长
				Grow = InNum + 3 * InNum / 8 + ConstantGrow;
			}
			Retval = Grow;

			// 处理溢出
			if (InNum > Retval) Retval = ULLONG_MAX;
			return Retval;
		}
		FORCEINLINE SizeType	GetShrink(SizeType InNum, SizeType InMax)
		{
			SizeType Retval;
			check(InNum < InMax);

			// 如果闲余空间过多，则刚好收缩到使用空间
			if ((3 * InNum < 2 * InMax) && (InMax - InNum > 64 || !InNum))
			{
				Retval = InNum;
			}
			else
			{
				Retval = InMax;
			}

			return Retval;
		}
	};
}

// base allocator
namespace Fuko
{
	class BaseAlloc
	{
	public:
		using SizeType = int32;
		using USizeType = uint32;

		template<typename T>
		FORCEINLINE SizeType	Free(T*& Data) 
		{ 
			_aligned_free(Data);
			Data = nullptr;
			return 0;
		}
		template<typename T>
		FORCEINLINE SizeType	Reserve(T*& Data, SizeType InMax)
		{
			if (Data)
				Data = (T*)_aligned_realloc(Data, InMax * sizeof(T), alignof(T));
			else
				Data = (T*)_aligned_malloc(InMax * sizeof(T), alignof(T));
			return InMax;
		}
		FORCEINLINE SizeType	FreeRaw(void*& Data, SizeType InAlign)
		{
			_aligned_free(Data);
			Data = nullptr;
			return 0;
		}
		FORCEINLINE SizeType	ReserveRaw(void*& Data, SizeType InSize, SizeType InAlign)
		{
			if (Data)
				Data = _aligned_realloc(Data, InSize, InAlign);
			else
				Data = _aligned_malloc(InSize, InAlign);
			return InSize;
		}

		FORCEINLINE SizeType	GetGrow(SizeType InNum, SizeType InMax)
		{
			constexpr SizeType FirstGrow = 4;
			constexpr SizeType ConstantGrow = 16;

			SizeType Retval;
			check(InNum > InMax && InNum > 0);

			SizeType Grow = FirstGrow;	// 初次分配空间的内存增长
			if (InMax || InNum > Grow)
			{
				// 计算内存增长
				Grow = InNum + 3 * InNum / 8 + ConstantGrow;
			}
			Retval = Grow;

			// 处理溢出
			if (InNum > Retval) Retval = ULLONG_MAX;
			return Retval;
		}
		FORCEINLINE SizeType	GetShrink(SizeType InNum, SizeType InMax)
		{
			SizeType Retval;
			check(InNum < InMax);

			// 如果闲余空间过多，则刚好收缩到使用空间
			if ((3 * InNum < 2 * InMax) && (InMax - InNum > 64 || !InNum))
			{
				Retval = InNum;
			}
			else
			{
				Retval = InMax;
			}

			return Retval;
		}
	};
}

// block allocator
namespace Fuko
{
	template<typename FallBackAlloc = PmrAlloc>
	class TBlockAlloc
	{
		FallBackAlloc	m_FallBack;
		void* _UnpackData(void* RawData) { return((int32*)(RawData) - 4); }
	public:
		using SizeType = int32;
		using USizeType = uint32;

		FORCEINLINE SizeType	FreeRaw(void*& Data, SizeType InAlign)
		{
			check(InAlign <= 16);
			if (ReleaseBlock(Data))
			{
				Data = nullptr;
			}
			else
			{
				void* RawPtr = _UnpackData(Data);
				m_FallBack.FreeRaw(RawPtr, InAlign);
				Data = nullptr;
			}
			return 0;
		}
		FORCEINLINE SizeType	ReserveRaw(void*& Data, SizeType InSize, SizeType InAlign)
		{
			check(InAlign <= 16);
			USizeType BlockSize = Math::RoundUpToPowerOfTwo((USizeType)InSize);
			if (Data)
			{
				void* OldData = Data;
				SizeType LastSize = GetBlockSize(OldData);
				if (ReleaseBlock(Data))
				{
					// old memory is block 
					Data = nullptr;
					if (BlockSize <= 256)
					{
						// new memory use block 
						Data = RequirBlock(BlockSize);
						Memcpy(Data, OldData, LastSize);
					}
					else
					{
						// new memory use Fallback 
						m_FallBack.ReserveRaw(Data, InSize + 16, InAlign);
						int32* SizePtr = (int32*)Data;
						*SizePtr = InSize;
						Data = SizePtr + 4;
						Memcpy(Data, OldData, LastSize);
					}
				}
				else
				{
					// old memory isn't block 
					if (BlockSize <= 256)
					{
						// new memory use block 
						void* NewData = RequirBlock(BlockSize);
						Memcpy(NewData, OldData, LastSize);
						void* RawData = _UnpackData(Data);
						m_FallBack.FreeRaw(RawData, InAlign);
						Data = NewData;
					}
					else
					{
						// new memory use Fallback 
						void* RawData = _UnpackData(Data);
						m_FallBack.ReserveRaw(RawData, InSize + 16, InAlign);
						int32* SizePtr = (int32*)RawData;
						*SizePtr = InSize;
						Data = SizePtr + 4;
					}
				}
			}
			else
			{
				// old memory not exist 
				if (BlockSize <= 256)
				{
					// use block 
					Data = RequirBlock(BlockSize);
				}
				else
				{
					// use Fallback 
					m_FallBack.ReserveRaw(Data, InSize + 16, InAlign);
					int32* SizePtr = (int32*)Data;
					*SizePtr = InSize;
					Data = SizePtr + 4;
				}
			}
			return BlockSize > 256 ? InSize : BlockSize < 32 ? 32 : BlockSize;
		}

		template<typename T>
		FORCEINLINE SizeType	Free(T*& Data) { return FreeRaw((void*&)Data, alignof(T)); }		
		template<typename T>
		FORCEINLINE SizeType	Reserve(T*& Data, SizeType InMax) { return ReserveRaw((void*&)Data, InMax * sizeof(T), alignof(T)) / sizeof(T); }
	
		FORCEINLINE SizeType	GetGrow(SizeType InNum, SizeType InMax)
		{
			constexpr SizeType FirstGrow = 4;
			constexpr SizeType ConstantGrow = 16;

			SizeType Retval;
			check(InNum > InMax && InNum > 0);

			SizeType Grow = FirstGrow;	// 初次分配空间的内存增长
			if (InMax || InNum > Grow)
			{
				// 计算内存增长
				Grow = InNum + 3 * InNum / 8 + ConstantGrow;
			}
			Retval = Grow;

			// 处理溢出
			if (InNum > Retval) Retval = ULLONG_MAX;
			return Retval;
		}
		FORCEINLINE SizeType	GetShrink(SizeType InNum, SizeType InMax)
		{
			SizeType Retval;
			check(InNum < InMax);

			// 如果闲余空间过多，则刚好收缩到使用空间
			if ((3 * InNum < 2 * InMax) && (InMax - InNum > 64 || !InNum))
			{
				Retval = InNum;
			}
			else
			{
				Retval = InMax;
			}

			return Retval;
		}
	};
}