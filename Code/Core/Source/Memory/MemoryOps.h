#pragma once
#include <Memory/MemoryOps.h>

namespace Fuko
{
	CORE_API void MemswapGreaterThan8(void* Ptr1, void* Ptr2, size_t Size)
	{
		union PtrUnion
		{
			void*		PtrVoid;
			uint8_t*	Ptr8;
			uint16_t*	Ptr16;
			uint32_t*	Ptr32;
			uint64_t*	Ptr64;
			size_t		PtrUint;
		};

		PtrUnion Union1 = { Ptr1 };
		PtrUnion Union2 = { Ptr2 };

		checkf(Union1.PtrVoid && Union2.PtrVoid, TEXT("Pointers must be non-null: %p, %p"), Union1.PtrVoid, Union2.PtrVoid);

		// 不对8字节以下进行交换
		check(Size > 8);

		// 先把底部交换方便进行对齐的高速交换
		if (Union1.PtrUint & 1)
		{
			Valswap(*Union1.Ptr8++, *Union2.Ptr8++);
			Size -= 1;
		}
		if (Union1.PtrUint & 2)
		{
			Valswap(*Union1.Ptr16++, *Union2.Ptr16++);
			Size -= 2;
		}
		if (Union1.PtrUint & 4)
		{
			Valswap(*Union1.Ptr32++, *Union2.Ptr32++);
			Size -= 4;
		}

		// 对齐的内存交换
		uint32_t CommonAlignment = Math::Min(Math::CountTrailingZeros((uint32)Union1.PtrUint - (uint32)Union2.PtrUint), 3u);
		switch (CommonAlignment)
		{
		default:
			for (; Size >= 8; Size -= 8)
			{
				Valswap(*Union1.Ptr64++, *Union2.Ptr64++);
			}

		case 2:
			for (; Size >= 4; Size -= 4)
			{
				Valswap(*Union1.Ptr32++, *Union2.Ptr32++);
			}

		case 1:
			for (; Size >= 2; Size -= 2)
			{
				Valswap(*Union1.Ptr16++, *Union2.Ptr16++);
			}

		case 0:
			for (; Size >= 1; Size -= 1)
			{
				Valswap(*Union1.Ptr8++, *Union2.Ptr8++);
			}
		}
	}
}