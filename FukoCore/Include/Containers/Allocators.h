#pragma once
#include <stdint.h>
#include <limits>
#include "CoreConfig.h"
#include "Memory/Memory.h"
#include "Templates/Align.h"
#include "Math/MathUtility.h"

// HeapAllocator: 堆分配器
// InlineAllocator: 自身分配了一部分内存，只有在超出这个内存的时候才会把数据转移到备选分配器上
// NonRelocatableInlineAllocator: 没有备选分配器，直接从堆开内存
// FixedAllocator: 固定分配器，resize? 不存在的
// SetAlloctor: 封装给set使用的Alloctor
// SparseArrayAllocator: 用来给稀疏数组使用的Allocator

// 空间增长，收缩算法
namespace Fuko
{
	/**
	 * @fn FORCEINLINE int32_t DefaultCalculateSlackShrink(int32_t NumElements, int32_t NumAllocatedElements, size_t BytesPerElement, bool bAllowQuantize, int32_t Alignment = DEFAULT_ALIGNMENT)
	 *
	 * @brief 收缩元素个数的默认算法
	 *
	 * @date 2020/6/18
	 *
	 * @param  NumElements		    当前元素个数
	 * @param  NumAllocatedElements 当前空间容纳的元素个数
	 * @param  BytesPerElement	    每个元素的大小
	 * @param  bAllowQuantize	    是否使用对齐来校准元素数量
	 * @param  Alignment		    内存对齐
	 *
	 * @returns 元素个数
	 */
	template<typename SizeType>
	FORCEINLINE SizeType DefaultCalculateSlackShrink(SizeType NumElements, SizeType NumAllocatedElements, size_t BytesPerElement, bool bAllowQuantize, uint32 Alignment = DEFAULT_ALIGNMENT)
	{
		SizeType Retval;
		check(NumElements < NumAllocatedElements);

		// 如果闲余空间过多，则刚好收缩到使用空间
		const SizeType CurrentSlackElements = NumAllocatedElements - NumElements;
		const size_t CurrentSlackBytes = (NumAllocatedElements - NumElements)*BytesPerElement;
		const bool bTooManySlackBytes = CurrentSlackBytes >= 16384;
		const bool bTooManySlackElements = 3 * NumElements < 2 * NumAllocatedElements;
		if ((bTooManySlackBytes || bTooManySlackElements) && (CurrentSlackElements > 64 || !NumElements))
		{
			Retval = NumElements;
			if (Retval > 0)
			{
				if (bAllowQuantize)
				{
					Retval = (SizeType)QuantizeSize(Retval * BytesPerElement, Alignment) / BytesPerElement;
				}
			}
		}
		else
		{
			Retval = NumAllocatedElements;
		}

		return Retval;
	}

	/**
	 * @fn FORCEINLINE int32_t DefaultCalculateSlackGrow(int32_t NumElements, int32_t NumAllocatedElements, size_t BytesPerElement, bool bAllowQuantize, int32_t Alignment = DEFAULT_ALIGNMENT)
	 *
	 * @brief 增长元素个数的默认算法
	 *
	 * @date 2020/6/18
	 *
	 * @param  NumElements		    当前元素个数
	 * @param  NumAllocatedElements 当前空间容纳的元素个数
	 * @param  BytesPerElement	    每个元素的大小
	 * @param  bAllowQuantize	    是否使用对齐来校准元素数量
	 * @param  Alignment		    内存对齐
	 *
	 * @returns 元素个数
	 */
	template<typename SizeType>
	FORCEINLINE SizeType DefaultCalculateSlackGrow(SizeType NumElements, SizeType NumAllocatedElements, size_t BytesPerElement, bool bAllowQuantize, uint32 Alignment = DEFAULT_ALIGNMENT)
	{
		constexpr size_t FirstGrow = 4;
		constexpr size_t ConstantGrow = 16;
		
		SizeType Retval;
		check(NumElements > NumAllocatedElements && NumElements > 0);

		size_t Grow = FirstGrow;	// 初次分配空间的内存增长
		if (NumAllocatedElements || size_t(NumElements) > Grow)
		{
			// 计算内存增长
			Grow = size_t(NumElements) + 3 * size_t(NumElements) / 8 + ConstantGrow;
		}
		// 对齐内存
		if (bAllowQuantize)
		{
			Retval = (SizeType)QuantizeSize(Grow * BytesPerElement, Alignment) / BytesPerElement;
		}
		else
		{
			Retval = (SizeType)Grow;
		}
		// 处理溢出
		if (NumElements > Retval)
		{
			Retval = std::numeric_limits<SizeType>::max();
		}
		return Retval;
	}

	/**
	 * @fn FORCEINLINE int32_t DefaultCalculateSlackReserve(int32_t NumElements, size_t BytesPerElement, bool bAllowQuantize, int32_t Alignment = DEFAULT_ALIGNMENT)
	 *
	 * @brief 计算元素实际空间占用的默认算法
	 *
	 * @date 2020/6/18
	 *
	 * @param  NumElements	   当前元素个数
	 * @param  BytesPerElement 每个元素的大小
	 * @param  bAllowQuantize  是否使用对齐来校准元素数量
	 * @param  Alignment	   内存对齐
	 *
	 * @returns 元素个数
	 */
	template<typename SizeType>
	FORCEINLINE SizeType DefaultCalculateSlackReserve(SizeType NumElements, size_t BytesPerElement, bool bAllowQuantize, uint32 Alignment = DEFAULT_ALIGNMENT)
	{
		check(NumElements > 0);

		SizeType Retval = NumElements;
		if (bAllowQuantize)
		{
			Retval = (SizeType)QuantizeSize(size_t(Retval) * size_t(BytesPerElement), Alignment) / BytesPerElement;
			// 处理溢出
			if (NumElements > Retval)
			{
				Retval = std::numeric_limits<SizeType>::max();
			}
		}

		return Retval;
	}
}

// Allocator的TypeTraits和基本样式
namespace Fuko
{
	// 一个假的类型，作为编译期的占位符
	struct FScriptContainerElement
	{
	};

	// Allocator类型萃取 
	template <typename AllocatorType>
	struct TAllocatorTraitsBase
	{
		enum { IsZeroConstruct = true };	// 支持置0初始化
	};
	template <typename AllocatorType>
	struct TAllocatorTraits : TAllocatorTraitsBase<AllocatorType>
	{
	};

	// 是否支持在两个Allocator之间移动 
	template <typename FromAllocatorType, typename ToAllocatorType>
	struct TCanMoveBetweenAllocators
	{
		enum { Value = false };
	};
	
	// 从位数转到对应的类型
	template <int IndexSize>
	struct TBitsToSizeType
	{
		static_assert(IndexSize, "Unsupported allocator index size.");
	};
	template <> struct TBitsToSizeType<8> { using Type = int8; };
	template <> struct TBitsToSizeType<16> { using Type = int16; };
	template <> struct TBitsToSizeType<32> { using Type = int32; };
	template <> struct TBitsToSizeType<64> { using Type = int64; };
	
	// 基本样式 
	class FContainerAllocatorInterface
	{
	public:
		// Size和Index的类型，必须是有符号的 
		using SizeType = int32;

		// 是否需要Element类型，如果为true则ForAnyElementType被禁用 
		enum { NeedsElementType = true };

		// 有类型的Allocator，会使用类型信息
		template<typename ElementType>
		class ForElementType
		{
			/**
			 * 相当于右值拷贝
			 */
			void MoveToEmpty(ForElementType& Other);

			/**
			 * 跨Allocator的右值拷贝
			 */
			template <typename OtherAllocatorType>
			void MoveToEmptyFromOtherAllocator(typename OtherAllocatorType::template ForElementType<ElementType>& Other);

			/** 得到分配的内存 */
			ElementType* GetAllocation() const;

			/**
			 * 重置大小
			 * @param PreviousNumElements - 当前存储的元素数量
			 * @param NumElements - 想要存储的元素数量
			 * @param NumBytesPerElement - 每个元素的大小
			 */
			void ResizeAllocation(
				SizeType PreviousNumElements,
				SizeType NumElements,
				size_t NumBytesPerElement
			);

			/**
			 * 计算松弛存储的空间占用
			 * @param NumElements - 预期的元素数量
			 * @param NumBytesPerElement - 单个元素的大小
			 */
			SizeType CalculateSlackReserve(
				SizeType NumElements,
				size_t NumBytesPerElement
			) const;

			/**
			 * 计算内存收缩
			 * @param NumElements - 预期的元素数量
			 * @param NumAllocatedElements - 当前已分配的元素数量
			 * @param NumBytesPerElement - 单个元素的大小
			 */
			SizeType CalculateSlackShrink(
				SizeType NumElements,
				SizeType NumAllocatedElements,
				size_t NumBytesPerElement
			) const;

			/**
			 * 计算内存增长
			 * @param NumElements - 预期的元素数量
			 * @param CurrentNumSlackElements - 当前分配的元素数量
			 * @param NumBytesPerElement - 单个元素的大小
			 */
			SizeType CalculateSlackGrow(
				SizeType NumElements,
				SizeType NumAllocatedElements,
				size_t NumBytesPerElement
			) const;

			/**
			 * 返回分配的内存大小
			 * @param NumAllocatedElements - 分配的元素数量
			 * @param NumBytesPerElement - 单个元素的大小
			 */
			size_t GetAllocatedSize(SizeType NumAllocatedElements, size_t NumBytesPerElement) const;

			/** 返回是否以及有分配的内存 */
			bool HasAllocation() const;

			/** 得到初始大小 */
			SizeType GetInitialCapacity() const;
		};

		// 无类型的Allocator，只有在NeedsElementType为False的时候可以使用 
		typedef ForElementType<FScriptContainerElement> ForAnyElementType;
	};

	class FDefaultBitArrayAllocator;

	template<int IndexSize> class TSizedDefaultAllocator;
	using FDefaultAllocator = TSizedDefaultAllocator<32>;
}

// TAlignedHeapAlloctor,带有内存对齐的堆分配器
namespace Fuko
{
	template<typename T, uint32 Alignment = DEFAULT_ALIGNMENT>
	class TAlignedHeapAllocator
	{
	public:
		using SizeType = int32;
		using ElementType = T;

		TAlignedHeapAllocator() = default;
		TAlignedHeapAllocator(const TAlignedHeapAllocator&) = delete;
		TAlignedHeapAllocator(const TAlignedHeapAllocator&&) = delete;
		TAlignedHeapAllocator& operator=(const TAlignedHeapAllocator&) = delete;

		FORCEINLINE void MoveToEmpty(ForAnyElementType& Other)
		{
			check(this != &Other);

			if (Data)
			{
				_aligned_free(Data);
			}

			Data = Other.Data;
			Other.Data = nullptr;
		}

		FORCEINLINE ~ForAnyElementType()
		{
			if (Data) _aligned_free(Data);
		}

		FORCEINLINE ElementType* GetAllocation() const { return (ElementType*)Data; }

		void ResizeAllocation(
			SizeType PreviousNumElements,
			SizeType NumElements,
			size_t NumBytesPerElement
		)
		{
			if (Data || NumElements)
			{
				Data = _aligned_realloc(Data, NumElements*NumBytesPerElement, Alignment);
			}
		}
		FORCEINLINE SizeType CalculateSlackReserve(SizeType NumElements, size_t NumBytesPerElement) const
		{
			return DefaultCalculateSlackReserve(NumElements, NumBytesPerElement, true, Alignment);
		}
		FORCEINLINE SizeType CalculateSlackShrink(SizeType NumElements, SizeType NumAllocatedElements, size_t NumBytesPerElement) const
		{
			return DefaultCalculateSlackShrink(NumElements, NumAllocatedElements, NumBytesPerElement, true, Alignment);
		}
		FORCEINLINE SizeType CalculateSlackGrow(SizeType NumElements, SizeType NumAllocatedElements, size_t NumBytesPerElement) const
		{
			return DefaultCalculateSlackGrow(NumElements, NumAllocatedElements, NumBytesPerElement, true, Alignment);
		}

		FORCEINLINE size_t GetAllocatedSize(SizeType NumAllocatedElements, size_t NumBytesPerElement) const { return NumAllocatedElements * NumBytesPerElement; }

		FORCEINLINE bool HasAllocation() const { return !!Data; }

		FORCEINLINE SizeType GetInitialCapacity() const { return 0; }
	private:
		void* Data;
	};
}

// TSizedHeapAllocator,堆分配器 
namespace Fuko
{
	template <typename T, int IndexSize>
	class TSizedHeapAllocator
	{
	public:
		using SizeType = typename TBitsToSizeType<IndexSize>::Type;
		using ElementType = T;

		template <int>
		friend class TSizedHeapAllocator;

	public:
		TSizedHeapAllocator() = default;
		TSizedHeapAllocator(const TSizedHeapAllocator&) = delete;
		TSizedHeapAllocator(const TSizedHeapAllocator&&) = delete;
		TSizedHeapAllocator& operator=(const TSizedHeapAllocator&) = delete;

		template <typename OtherAllocator>
		FORCEINLINE void MoveToEmptyFromOtherAllocator(typename OtherAllocator& Other)
		{
			check((void*)this != (void*)&Other);

			if (Data)
			{
				Free(Data);
			}

			Data = Other.Data;
			Other.Data = nullptr;
		}

		FORCEINLINE void MoveToEmpty(TSizedHeapAllocator& Other)
		{
			this->MoveToEmptyFromOtherAllocator<TSizedHeapAllocator>(Other);
		}

		FORCEINLINE ~TSizedHeapAllocator() { if (Data) free(Data); }

		FORCEINLINE ElementType* GetAllocation() const { return (ElementType*)Data; }

		FORCEINLINE void ResizeAllocation(SizeType PreviousNumElements, SizeType NumElements, size_t NumBytesPerElement)
		{
			if (Data || NumElements)
			{
				Data = (ElementType*)realloc(Data, NumElements*NumBytesPerElement);
			}
		}

		FORCEINLINE SizeType CalculateSlackReserve(SizeType NumElements, size_t NumBytesPerElement) const
		{
			return DefaultCalculateSlackReserve(NumElements, NumBytesPerElement, true);
		}
		FORCEINLINE SizeType CalculateSlackShrink(SizeType NumElements, SizeType NumAllocatedElements, size_t NumBytesPerElement) const
		{
			return DefaultCalculateSlackShrink(NumElements, NumAllocatedElements, NumBytesPerElement, true);
		}
		FORCEINLINE SizeType CalculateSlackGrow(SizeType NumElements, SizeType NumAllocatedElements, size_t NumBytesPerElement) const
		{
			return DefaultCalculateSlackGrow(NumElements, NumAllocatedElements, NumBytesPerElement, true);
		}

		FORCEINLINE size_t GetAllocatedSize(SizeType NumAllocatedElements, size_t NumBytesPerElement) const
		{
			return NumAllocatedElements * NumBytesPerElement;
		}

		FORCEINLINE bool HasAllocation() const { return !!Data; }

		FORCEINLINE SizeType GetInitialCapacity() const { return 0; }

	private:
		void* Data;
	};

	template <typename FromT,typename ToT, uint8 FromIndexSize, uint8 ToIndexSize>
	struct TCanMoveBetweenAllocators<TSizedHeapAllocator<FromT, FromIndexSize>, TSizedHeapAllocator<ToT, ToIndexSize>>
	{
		enum { Value = true };
	};
}

// TInlineAllocator,自身具有固定的空间，当空间不够的时候，会使用Fallback的Allocator 
namespace Fuko
{
	template <typename T, uint32 NumInlineElements, typename SecondaryAllocator = TSizedHeapAllocator<T, 32>>
	class TInlineAllocator
	{
	public:
		using SizeType = int32;
		using ElementType = T;

	public:
		TInlineAllocator() = default;
		TInlineAllocator(const TInlineAllocator&) = delete;
		TInlineAllocator(const TInlineAllocator&&) = delete;
		TInlineAllocator& operator=(const TInlineAllocator&) = delete;

		FORCEINLINE void MoveToEmpty(ForElementType& Other)
		{
			check(this != &Other);

			if (!Other.SecondaryData.GetAllocation())
			{
				RelocateConstructItems<ElementType>((void*)InlineData, Other.GetInlineElements(), NumInlineElements);
			}

			SecondaryData.MoveToEmpty(Other.SecondaryData);
		}

		FORCEINLINE ElementType* GetAllocation() const
		{
			auto A = SecondaryData.GetAllocation();
			auto B = GetInlineElements();
			return A ? A : B;
		}

		void ResizeAllocation(SizeType PreviousNumElements, SizeType NumElements, size_t NumBytesPerElement)
		{
			if (NumElements <= NumInlineElements)
			{
				if (SecondaryData.GetAllocation())
				{
					RelocateConstructItems<ElementType>((void*)InlineData, (ElementType*)SecondaryData.GetAllocation(), PreviousNumElements);
					SecondaryData.ResizeAllocation(0, 0, NumBytesPerElement);
				}
			}
			else
			{
				if (!SecondaryData.GetAllocation())
				{
					SecondaryData.ResizeAllocation(0, NumElements, NumBytesPerElement);
					RelocateConstructItems<ElementType>((void*)SecondaryData.GetAllocation(), GetInlineElements(), PreviousNumElements);
				}
				else
				{
					SecondaryData.ResizeAllocation(PreviousNumElements, NumElements, NumBytesPerElement);
				}
			}
		}

		FORCEINLINE SizeType CalculateSlackReserve(SizeType NumElements, size_t NumBytesPerElement) const
		{
			return NumElements <= NumInlineElements ?
				NumInlineElements :
				SecondaryData.CalculateSlackReserve(NumElements, NumBytesPerElement);
		}
		FORCEINLINE SizeType CalculateSlackShrink(SizeType NumElements, SizeType NumAllocatedElements, size_t NumBytesPerElement) const
		{
			return NumElements <= NumInlineElements ?
				NumInlineElements :
				SecondaryData.CalculateSlackShrink(NumElements, NumAllocatedElements, NumBytesPerElement);
		}
		FORCEINLINE SizeType CalculateSlackGrow(SizeType NumElements, SizeType NumAllocatedElements, size_t NumBytesPerElement) const
		{
			return NumElements <= NumInlineElements ?
				NumInlineElements :
				SecondaryData.CalculateSlackGrow(NumElements, NumAllocatedElements, NumBytesPerElement);
		}

		size_t GetAllocatedSize(SizeType NumAllocatedElements, size_t NumBytesPerElement) const
		{
			if (NumAllocatedElements > NumInlineElements)
			{
				return SecondaryData.GetAllocatedSize(NumAllocatedElements, NumBytesPerElement);
			}
			return 0;
		}

		bool HasAllocation() const { return SecondaryData.HasAllocation(); }

		SizeType GetInitialCapacity() const { return NumInlineElements; }
	private:
		TTypeCompatibleBytes<ElementType> InlineData[NumInlineElements];

		SecondaryAllocator SecondaryData;

		ElementType* GetInlineElements() const
		{
			return (ElementType*)InlineData;
		}
	};
}

// TFixedAllocator,固定大小的Allocator，空间不够也不会开堆
namespace Fuko
{
	template <typename T, uint32 NumInlineElements>
	class TFixedAllocator
	{
	public:
		using SizeType = int32;
		using ElementType = T;

	public:
		TFixedAllocator() = default;
		TFixedAllocator(const TFixedAllocator&) = delete;
		TFixedAllocator(const TFixedAllocator&&) = delete;
		TFixedAllocator& operator=(const TFixedAllocator&) = delete;

		FORCEINLINE void MoveToEmpty(ForElementType& Other)
		{
			check(this != &Other);
			RelocateConstructItems<ElementType>((void*)InlineData, Other.GetInlineElements(), NumInlineElements);
		}

		FORCEINLINE ElementType* GetAllocation() const
		{
			return GetInlineElements();
		}

		void ResizeAllocation(SizeType PreviousNumElements, SizeType NumElements, size_t NumBytesPerElement)
		{
			check(NumElements <= NumInlineElements);
		}

		FORCEINLINE SizeType CalculateSlackReserve(SizeType NumElements, size_t NumBytesPerElement) const
		{
			check(NumElements <= NumInlineElements);
			return NumInlineElements;
		}
		FORCEINLINE SizeType CalculateSlackShrink(SizeType NumElements, SizeType NumAllocatedElements, size_t NumBytesPerElement) const
		{
			check(NumAllocatedElements <= NumInlineElements);
			return NumInlineElements;
		}
		FORCEINLINE SizeType CalculateSlackGrow(SizeType NumElements, SizeType NumAllocatedElements, size_t NumBytesPerElement) const
		{
			check(NumElements <= NumInlineElements);
			return NumInlineElements;
		}

		FORCEINLINE size_t GetAllocatedSize(SizeType NumAllocatedElements, size_t NumBytesPerElement) const { return 0; }
		FORCEINLINE bool HasAllocation() const { return false; }
		FORCEINLINE SizeType GetInitialCapacity() const { return NumInlineElements; }
	
	private:
		TTypeCompatibleBytes<ElementType> InlineData[NumInlineElements];
		ElementType* GetInlineElements() const
		{
			return (ElementType*)InlineData;
		}
	};
}

// TSparseArrayAllocator,给稀疏数组使用的Allocator
namespace Fuko
{
	template<typename InElementAllocator = TSizedHeapAllocator, typename InBitArrayAllocator = FDefaultBitArrayAllocator>
	class TSparseArrayAllocator
	{
	public:

		typedef InElementAllocator ElementAllocator;
		typedef InBitArrayAllocator BitArrayAllocator;
	};
}

// TInlineSparseArrayAllocator,上者的Inline版本
namespace Fuko
{
	inline constexpr int32 NumBitsPerDWORD = 32;
	inline constexpr int32 NumBitsPerDWORDLogTwo = 5;

	template<
		uint32 NumInlineElements,
		typename SecondaryAllocator = TSparseArrayAllocator<FDefaultAllocator, FDefaultAllocator>
	>
		class TInlineSparseArrayAllocator
	{
	private:
		enum { InlineBitArrayDWORDs = (NumInlineElements + NumBitsPerDWORD - 1) / NumBitsPerDWORD };

	public:
		typedef TInlineAllocator<NumInlineElements, typename SecondaryAllocator::ElementAllocator>		ElementAllocator;
		typedef TInlineAllocator<InlineBitArrayDWORDs, typename SecondaryAllocator::BitArrayAllocator>	BitArrayAllocator;
	};
}

// TFixedSparseArrayAllocator,上者的Fixed版本
namespace Fuko
{
	template <uint32 NumInlineElements>
	class TFixedSparseArrayAllocator
	{
	private:

		/** The size to allocate inline for the bit array. */
		enum { InlineBitArrayDWORDs = (NumInlineElements + NumBitsPerDWORD - 1) / NumBitsPerDWORD };

	public:

		typedef TFixedAllocator<NumInlineElements>    ElementAllocator;
		typedef TFixedAllocator<InlineBitArrayDWORDs> BitArrayAllocator;
	};
}

// TSetAllocator,给Set用的Allocator
namespace Fuko
{
	#define DEFAULT_NUMBER_OF_ELEMENTS_PER_HASH_BUCKET	2
	#define DEFAULT_BASE_NUMBER_OF_HASH_BUCKETS			8
	#define DEFAULT_MIN_NUMBER_OF_HASHED_ELEMENTS		4

	template<
		typename InSparseArrayAllocator = TSparseArrayAllocator<>,
		typename InHashAllocator = TInlineAllocator<1, FDefaultAllocator>,
		uint32   AverageNumberOfElementsPerHashBucket = DEFAULT_NUMBER_OF_ELEMENTS_PER_HASH_BUCKET,
		uint32   BaseNumberOfHashBuckets = DEFAULT_BASE_NUMBER_OF_HASH_BUCKETS,
		uint32   MinNumberOfHashedElements = DEFAULT_MIN_NUMBER_OF_HASHED_ELEMENTS
	>
	class TSetAllocator
	{
	public:
		static FORCEINLINE uint32 GetNumberOfHashBuckets(uint32 NumHashedElements)
		{
			if (NumHashedElements >= MinNumberOfHashedElements)
			{
				return FMath::RoundUpToPowerOfTwo(NumHashedElements / AverageNumberOfElementsPerHashBucket + BaseNumberOfHashBuckets);
			}

			return 1;
		}

		typedef InSparseArrayAllocator SparseArrayAllocator;
		typedef InHashAllocator        HashAllocator;
	};
}

// TInlineSetAllocator,上者的Inline版本
namespace Fuko
{
	template<
		uint32   NumInlineElements,
		typename SecondaryAllocator = TSetAllocator<TSparseArrayAllocator<FDefaultAllocator, FDefaultAllocator>, FDefaultAllocator>,
		uint32   AverageNumberOfElementsPerHashBucket = DEFAULT_NUMBER_OF_ELEMENTS_PER_HASH_BUCKET,
		uint32   MinNumberOfHashedElements = DEFAULT_MIN_NUMBER_OF_HASHED_ELEMENTS
	>
		class TInlineSetAllocator
	{
	private:
		enum { NumInlineHashBuckets = (NumInlineElements + AverageNumberOfElementsPerHashBucket - 1) / AverageNumberOfElementsPerHashBucket };
		static_assert(NumInlineHashBuckets > 0 && !(NumInlineHashBuckets & (NumInlineHashBuckets - 1)), "Number of inline buckets must be a power of two");

	public:
		static FORCEINLINE uint32 GetNumberOfHashBuckets(uint32 NumHashedElements)
		{
			const uint32 NumDesiredHashBuckets = FMath::RoundUpToPowerOfTwo(NumHashedElements / AverageNumberOfElementsPerHashBucket);
			if (NumDesiredHashBuckets < NumInlineHashBuckets)
			{
				return NumInlineHashBuckets;
			}

			if (NumHashedElements < MinNumberOfHashedElements)
			{
				return NumInlineHashBuckets;
			}

			return NumDesiredHashBuckets;
		}

		typedef TInlineSparseArrayAllocator<NumInlineElements, typename SecondaryAllocator::SparseArrayAllocator> SparseArrayAllocator;
		typedef TInlineAllocator<NumInlineHashBuckets, typename SecondaryAllocator::HashAllocator>                HashAllocator;
	};

}

// TFixedSetAllocator,上者的Fixed版本
namespace Fuko
{
	template<
		uint32 NumInlineElements,
		uint32 AverageNumberOfElementsPerHashBucket = DEFAULT_NUMBER_OF_ELEMENTS_PER_HASH_BUCKET,
		uint32 MinNumberOfHashedElements = DEFAULT_MIN_NUMBER_OF_HASHED_ELEMENTS
	>
	class TFixedSetAllocator
	{
	private:
		enum { NumInlineHashBuckets = (NumInlineElements + AverageNumberOfElementsPerHashBucket - 1) / AverageNumberOfElementsPerHashBucket };
		static_assert(NumInlineHashBuckets > 0 && !(NumInlineHashBuckets & (NumInlineHashBuckets - 1)), "Number of inline buckets must be a power of two");

	public:
		static FORCEINLINE uint32 GetNumberOfHashBuckets(uint32 NumHashedElements)
		{
			const uint32 NumDesiredHashBuckets = FMath::RoundUpToPowerOfTwo(NumHashedElements / AverageNumberOfElementsPerHashBucket);
			if (NumDesiredHashBuckets < NumInlineHashBuckets)
			{
				return NumInlineHashBuckets;
			}

			if (NumHashedElements < MinNumberOfHashedElements)
			{
				return NumInlineHashBuckets;
			}

			return NumDesiredHashBuckets;
		}

		typedef TFixedSparseArrayAllocator<NumInlineElements> SparseArrayAllocator;
		typedef TFixedAllocator<NumInlineHashBuckets>         HashAllocator;
	};
}

// 各种Typedef
namespace Fuko
{
	template <int IndexSize> class TSizedDefaultAllocator : public TSizedHeapAllocator<IndexSize> { public: typedef TSizedHeapAllocator<IndexSize> Typedef; };

	class FDefaultSetAllocator         : public TSetAllocator<>         { public: typedef TSetAllocator<>         Typedef; };
	class FDefaultBitArrayAllocator    : public TInlineAllocator<4>     { public: typedef TInlineAllocator<4>     Typedef; };
	class FDefaultSparseArrayAllocator : public TSparseArrayAllocator<> { public: typedef TSparseArrayAllocator<> Typedef; };

	template <int IndexSize> struct TAllocatorTraits<TSizedDefaultAllocator<IndexSize>> : TAllocatorTraits<typename TSizedDefaultAllocator<IndexSize>::Typedef> {};
	
	template <> struct TAllocatorTraits<FDefaultAllocator>            : TAllocatorTraits<typename FDefaultAllocator           ::Typedef> {};
	template <> struct TAllocatorTraits<FDefaultSetAllocator>         : TAllocatorTraits<typename FDefaultSetAllocator        ::Typedef> {};
	template <> struct TAllocatorTraits<FDefaultBitArrayAllocator>    : TAllocatorTraits<typename FDefaultBitArrayAllocator   ::Typedef> {};
	template <> struct TAllocatorTraits<FDefaultSparseArrayAllocator> : TAllocatorTraits<typename FDefaultSparseArrayAllocator::Typedef> {};

	template <uint8 FromIndexSize, uint8 ToIndexSize> struct TCanMoveBetweenAllocators<TSizedDefaultAllocator<FromIndexSize>, TSizedDefaultAllocator<ToIndexSize>> : TCanMoveBetweenAllocators<typename TSizedDefaultAllocator<FromIndexSize>::Typedef, typename TSizedDefaultAllocator<ToIndexSize>::Typedef> {};
}
