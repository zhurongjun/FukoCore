#pragma once
#include <CoreConfig.h>
#include <CoreType.h>
#include <Templates/TypeTraits.h>
#include <CoreMinimal/Assert.h>
#include <Math/MathUtility.h>

#include <corecrt_malloc.h>
#include <vcruntime_string.h>

// Move,Copy... 
namespace Fuko
{
	template <typename T>
	FORCEINLINE void Valswap(T& A, T& B)
	{
		T Tmp = A;
		A = B;
		B = Tmp;
	}
	void MemswapGreaterThan8(void* Ptr1, void* Ptr2, size_t Size)
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
	
	FORCEINLINE void* Memmove(void* Dest, const void* Src, size_t Count) { return memmove(Dest, Src, Count); } 
	FORCEINLINE int32_t Memcmp(const void* Buf1, const void* Buf2, size_t Count) { return memcmp(Buf1, Buf2, Count); }
	FORCEINLINE void* Memset(void* Dest, uint8 Char, size_t Count) { return memset(Dest, Char, Count); }
	FORCEINLINE void* Memzero(void* Dest, size_t Count) { return memset(Dest, 0, Count); }
	FORCEINLINE void* Memcpy(void* Dest, const void* Src, size_t Count) { return memcpy(Dest, Src, Count); }
	FORCEINLINE void Memswap(void* Ptr1, void* Ptr2, size_t Size)
	{
		switch (Size)
		{
		case 0:
			break;

		case 1:
			Valswap(*(uint8_t*)Ptr1, *(uint8_t*)Ptr2);
			break;

		case 2:
			Valswap(*(uint16_t*)Ptr1, *(uint16_t*)Ptr2);
			break;

		case 3:
			Valswap(*((uint16_t*&)Ptr1)++, *((uint16_t*&)Ptr2)++);
			Valswap(*(uint8_t*)Ptr1, *(uint8_t*)Ptr2);
			break;

		case 4:
			Valswap(*(uint32_t*)Ptr1, *(uint32_t*)Ptr2);
			break;

		case 5:
			Valswap(*((uint32_t*&)Ptr1)++, *((uint32_t*&)Ptr2)++);
			Valswap(*(uint8_t*)Ptr1, *(uint8_t*)Ptr2);
			break;

		case 6:
			Valswap(*((uint32_t*&)Ptr1)++, *((uint32_t*&)Ptr2)++);
			Valswap(*(uint16_t*)Ptr1, *(uint16_t*)Ptr2);
			break;

		case 7:
			Valswap(*((uint32_t*&)Ptr1)++, *((uint32_t*&)Ptr2)++);
			Valswap(*((uint16_t*&)Ptr1)++, *((uint16_t*&)Ptr2)++);
			Valswap(*(uint8_t*)Ptr1, *(uint8_t*)Ptr2);
			break;

		case 8:
			Valswap(*(uint64_t*)Ptr1, *(uint64_t*)Ptr2);
			break;

		case 16:
			Valswap(((uint64_t*)Ptr1)[0], ((uint64_t*)Ptr2)[0]);
			Valswap(((uint64_t*)Ptr1)[1], ((uint64_t*)Ptr2)[1]);
			break;

		default:
			MemswapGreaterThan8(Ptr1, Ptr2, Size);
			break;
		}
	}

	//提供便利函数
	template<class T>
	FORCEINLINE void Memset(T& Src, uint8_t ValueToSet)
	{
		static_assert(!std::is_pointer_v<T>, "For pointers use the three parameters function");
		Memset(&Src, ValueToSet, sizeof(T));
	}
	template<class T>
	FORCEINLINE void Memzero(T& Src)
	{
		static_assert(!std::is_pointer_v<T>, "For pointers use the two parameters function");
		Memzero(&Src, sizeof(T));
	}
	template<class T>
	FORCEINLINE void Memcpy(T& Dest, const T& Src)
	{
		static_assert(!std::is_pointer_v<T>, "For pointers use the three parameters function");
		Memcpy(&Dest, &Src, sizeof(T));
	}
}

// Construct,Copy,Destruct object manually
namespace Fuko
{
	// 在指定内存上构造对象
	template <typename ElementType>
	void DefaultConstructItems(ElementType* Address, size_t Count)
	{
		if constexpr (TIsZeroConstructType_v<ElementType>)
		{
			Memset(Address, 0, sizeof(ElementType) * Count);
		}
		else
		{
			ElementType* Element = (ElementType*)Address;
			while (Count)
			{
				new (Element) ElementType;
				++Element;
				--Count;
			}
		}
	}

	// 在指定内存上析构对象
	template <typename ElementType>
	void DestructItems(ElementType* Element, size_t Count = 1)
	{
		if constexpr (!std::is_trivially_destructible_v<ElementType>)
		{
			while (Count)
			{
				// 防止类型拥有成员ElementType的typedef
				typedef ElementType DestructItemsElementTypeTypedef;

				Element->DestructItemsElementTypeTypedef::~DestructItemsElementTypeTypedef();
				++Element;
				--Count;
			}
		}
	}

	// 拷贝对象调用拷贝函数
	template <typename DestinationElementType, typename SourceElementType>
	void ConstructItems(DestinationElementType* Dest, const SourceElementType* Source, size_t Count)
	{
		if constexpr (TIsBitwiseConstructible_v<DestinationElementType, SourceElementType>)
		{
			Memcpy(Dest, Source, sizeof(SourceElementType) * Count);
		}
		else
		{
			while (Count)
			{
				new (Dest) DestinationElementType(*Source);
				++Dest;
				++Source;
				--Count;
			}
		}
	}

	// 拷贝对象，调用=重载
	template <typename ElementType>
	void CopyAssignItems(ElementType* Dest, const ElementType* Source, size_t Count)
	{
		if constexpr (std::is_trivially_copy_assignable_v<ElementType>)
		{
			Memcpy(Dest, Source, sizeof(ElementType) * Count);
		}
		else
		{
			while (Count)
			{
				*Dest = *Source;
				++Dest;
				++Source;
				--Count;
			}
		}
	}

	// 重定向对象
	template <typename DestElementType, typename SrcElementType>
	void RelocateConstructItems(DestElementType* Dest, const SrcElementType* Source, size_t Count)
	{
		if constexpr (TCanBitwiseRelocate_v<DestElementType, SrcElementType>)
		{
			Memmove(Dest, Source, sizeof(SrcElementType) * Count);
		}
		else
		{
			while (Count)
			{
				// 防止类型拥有成员SrcElementType的typedef
				typedef SrcElementType RelocateConstructItemsElementTypeTypedef;

				new (Dest) DestElementType(*Source);
				++(DestElementType*&)Dest;
				(Source++)->RelocateConstructItemsElementTypeTypedef::~RelocateConstructItemsElementTypeTypedef();
				--Count;
			}
		}
	}

	// 调用移动构造
	template <typename ElementType>
	void MoveConstructItems(ElementType* Dest, const ElementType* Source, size_t Count)
	{
		if constexpr (std::is_trivially_copy_constructible_v<ElementType>)
		{
			Memmove(Dest, Source, sizeof(ElementType) * Count);
		}
		else
		{
			while (Count)
			{
				new (Dest) ElementType((ElementType&&)*Source);
				++Dest;
				++Source;
				--Count;
			}
		}
	}

	// 调用移动拷贝
	template <typename ElementType>
	void MoveAssignItems(ElementType* Dest, const ElementType* Source, size_t Count)
	{
		if constexpr (std::is_trivially_move_assignable<ElementType>)
		{
			Memmove(Dest, Source, sizeof(ElementType) * Count);
		}
		else
		{
			while (Count)
			{
				*Dest = (ElementType&&)*Source;
				++Dest;
				++Source;
				--Count;
			}
		}
	}

	// 比较对象
	template <typename ElementType>
	bool CompareItems(const ElementType* A, const ElementType* B, size_t Count)
	{
		if constexpr (TCanBitwiseCompare_v<ElementType>)
		{
			return !Memcmp(A, B, sizeof(ElementType) * Count);
		}
		{
			while (Count)
			{
				if (!(*A == *B))
				{
					return false;
				}

				++A;
				++B;
				--Count;
			}
			return true;
		}
	}
}
