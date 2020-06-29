#pragma once
#include "CoreType.h"
#include "CoreConfig.h"
#include "Allocators.h"
#include "Array.h"
#include "CoreMinimal/Assert.h"
#include "BitArray.h"

#if FUKO_DEBUG
#define TSPARSEARRAY_RANGED_FOR_CHECKS 1
#else
#define TSPARSEARRAY_RANGED_FOR_CHECKS 0 
#endif // DEBUG

// std::forward
namespace Fuko
{
	template<typename ElementType, typename Allocator = FDefaultSparseArrayAllocator >
	class TSparseArray;
}

// Struct
namespace Fuko
{
	/**
	 * @struct FSparseArrayAllocationInfo
	 *
	 * @brief 用于输出稀疏数组的元素
	 */
	struct FSparseArrayAllocationInfo
	{
		int32 Index;
		void* Pointer;
	};

	/**
	 * @union TSparseArrayElementOrFreeListLink
	 *
	 * @brief 两用类型，存储已经分配的元素或是空闲元素链表节点
	 */
	template<typename ElementType>
	union TSparseArrayElementOrFreeListLink
	{
		ElementType ElementData;

		struct
		{
			// 前一个空闲元素或是分配的元素
			int32 PrevFreeIndex;

			// 前一个已经分配的元素或是空闲元素 
			int32 NextFreeIndex;
		};
	};
}

// TSparseArray
namespace Fuko
{
	template<typename InElementType, typename Allocator /*= FDefaultSparseArrayAllocator */>
	class TSparseArray
	{
		using ElementType = InElementType;

		friend struct TContainerTraits<TSparseArray>;

		template <typename, typename>
		friend class TScriptSparseArray;

	public:
		static const bool SupportsFreezeMemoryImage = false;

		/** Destructor. */
		~TSparseArray()
		{
			// Destruct the elements in the array.
			Empty();
		}

		/** Marks an index as allocated, and returns information about the allocation. */
		FSparseArrayAllocationInfo AllocateIndex(int32 Index)
		{
			check(Index >= 0);
			check(Index < GetMaxIndex());
			check(!AllocationFlags[Index]);

			// Flag the element as allocated.
			AllocationFlags[Index] = true;

			// Set the allocation info.
			FSparseArrayAllocationInfo Result;
			Result.Index = Index;
			Result.Pointer = &GetData(Result.Index).ElementData;

			return Result;
		}

		/**
		 * Allocates space for an element in the array.  The element is not initialized, and you must use the corresponding placement new operator
		 * to construct the element in the allocated memory.
		 */
		FSparseArrayAllocationInfo AddUninitialized()
		{
			int32 Index;
			if (NumFreeIndices)
			{
				// Remove and use the first index from the list of free elements.
				Index = FirstFreeIndex;
				FirstFreeIndex = GetData(FirstFreeIndex).NextFreeIndex;
				--NumFreeIndices;
				if (NumFreeIndices)
				{
					GetData(FirstFreeIndex).PrevFreeIndex = -1;
				}
			}
			else
			{
				// Add a new element.
				Index = Data.AddUninitialized(1);
				AllocationFlags.Add(false);
			}

			return AllocateIndex(Index);
		}

		/** Adds an element to the array. */
		int32 Add(const ElementType& Element)
		{
			FSparseArrayAllocationInfo Allocation = AddUninitialized();
			new(Allocation) ElementType(Element);
			return Allocation.Index;
		}

		/** Adds an element to the array. */
		int32 Add(ElementType&& Element)
		{
			FSparseArrayAllocationInfo Allocation = AddUninitialized();
			new(Allocation) ElementType(std::move(Element));
			return Allocation.Index;
		}

		FSparseArrayAllocationInfo AddUninitializedAtLowestFreeIndex(int32& LowestFreeIndexSearchStart)
		{
			int32 Index;
			if (NumFreeIndices)
			{
				Index = AllocationFlags.FindAndSetFirstZeroBit(LowestFreeIndexSearchStart);
				LowestFreeIndexSearchStart = Index + 1;

				auto& IndexData = GetData(Index);

				// Update FirstFreeIndex
				if (FirstFreeIndex == Index)
				{
					FirstFreeIndex = IndexData.NextFreeIndex;
				}

				// Link our next and prev free nodes together
				if (IndexData.NextFreeIndex >= 0)
				{
					GetData(IndexData.NextFreeIndex).PrevFreeIndex = IndexData.PrevFreeIndex;
				}

				if (IndexData.PrevFreeIndex >= 0)
				{
					GetData(IndexData.PrevFreeIndex).NextFreeIndex = IndexData.NextFreeIndex;
				}

				--NumFreeIndices;
			}
			else
			{
				// Add a new element.
				Index = Data.AddUninitialized(1);
				AllocationFlags.Add(true);
			}

			FSparseArrayAllocationInfo Result;
			Result.Index = Index;
			Result.Pointer = &GetData(Result.Index).ElementData;
			return Result;
		}

		/**
		 * Add an element at the lowest free index, instead of the last freed index.
		 * This requires a search which can be accelerated with LowestFreeIndexSearchStart.
		 */
		int32 AddAtLowestFreeIndex(const ElementType& Element, int32& LowestFreeIndexSearchStart)
		{
			FSparseArrayAllocationInfo Allocation = AddUninitializedAtLowestFreeIndex(LowestFreeIndexSearchStart);
			new(Allocation) ElementType(Element);
			return Allocation.Index;
		}

		/**
		 * Allocates space for an element in the array at a given index.  The element is not initialized, and you must use the corresponding placement new operator
		 * to construct the element in the allocated memory.
		 */
		FSparseArrayAllocationInfo InsertUninitialized(int32 Index)
		{
			// Enlarge the array to include the given index.
			if (Index >= Data.Num())
			{
				Data.AddUninitialized(Index + 1 - Data.Num());
				while (AllocationFlags.Num() < Data.Num())
				{
					const int32 FreeIndex = AllocationFlags.Num();
					GetData(FreeIndex).PrevFreeIndex = -1;
					GetData(FreeIndex).NextFreeIndex = FirstFreeIndex;
					if (NumFreeIndices)
					{
						GetData(FirstFreeIndex).PrevFreeIndex = FreeIndex;
					}
					FirstFreeIndex = FreeIndex;
					always_check(AllocationFlags.Add(false) == FreeIndex);
					++NumFreeIndices;
				};
			}

			// Verify that the specified index is free.
			check(!AllocationFlags[Index]);

			// Remove the index from the list of free elements.
			--NumFreeIndices;
			const int32 PrevFreeIndex = GetData(Index).PrevFreeIndex;
			const int32 NextFreeIndex = GetData(Index).NextFreeIndex;
			if (PrevFreeIndex != -1)
			{
				GetData(PrevFreeIndex).NextFreeIndex = NextFreeIndex;
			}
			else
			{
				FirstFreeIndex = NextFreeIndex;
			}
			if (NextFreeIndex != -1)
			{
				GetData(NextFreeIndex).PrevFreeIndex = PrevFreeIndex;
			}

			return AllocateIndex(Index);
		}

		/**
		 * Inserts an element to the array.
		 */
		void Insert(int32 Index, typename TTypeTraits<ElementType>::ConstInitType Element)
		{
			new(InsertUninitialized(Index)) ElementType(Element);
		}

		/** Removes Count elements from the array, starting from Index. */
		void RemoveAt(int32 Index, int32 Count = 1)
		{
			if (!std::is_trivially_destructible_v<ElementType>::Value)
			{
				for (int32 It = Index, ItCount = Count; ItCount; ++It, --ItCount)
				{
					((ElementType&)GetData(It).ElementData).~ElementType();
				}
			}

			RemoveAtUninitialized(Index, Count);
		}

		/** Removes Count elements from the array, starting from Index, without destructing them. */
		void RemoveAtUninitialized(int32 Index, int32 Count = 1)
		{
			for (; Count; --Count)
			{
				check(AllocationFlags[Index]);

				// Mark the element as free and add it to the free element list.
				if (NumFreeIndices)
				{
					GetData(FirstFreeIndex).PrevFreeIndex = Index;
				}
				auto& IndexData = GetData(Index);
				IndexData.PrevFreeIndex = -1;
				IndexData.NextFreeIndex = NumFreeIndices > 0 ? FirstFreeIndex : INDEX_NONE;
				FirstFreeIndex = Index;
				++NumFreeIndices;
				AllocationFlags[Index] = false;

				++Index;
			}
		}

		/**
		 * Removes all elements from the array, potentially leaving space allocated for an expected number of elements about to be added.
		 * @param ExpectedNumElements - The expected number of elements about to be added.
		 */
		void Empty(int32 ExpectedNumElements = 0)
		{
			// Destruct the allocated elements.
			if constexpr (!std::is_trivially_destructible_v<ElementType>)
			{
				for (TIterator It(*this); It; ++It)
				{
					ElementType& Element = *It;
					Element.~ElementType();
				}
			}

			// Free the allocated elements.
			Data.Empty(ExpectedNumElements);
			FirstFreeIndex = -1;
			NumFreeIndices = 0;
			AllocationFlags.Empty(ExpectedNumElements);
		}

		/** Empties the array, but keep its allocated memory as slack. */
		void Reset()
		{
			// Destruct the allocated elements.
			if (!std::is_trivially_destructible_v<ElementType>::Value)
			{
				for (TIterator It(*this); It; ++It)
				{
					ElementType& Element = *It;
					Element.~ElementType();
				}
			}

			// Free the allocated elements.
			Data.Reset();
			FirstFreeIndex = -1;
			NumFreeIndices = 0;
			AllocationFlags.Reset();
		}

		/**
		 * Preallocates enough memory to contain the specified number of elements.
		 *
		 * @param	ExpectedNumElements		the total number of elements that the array will have
		 */
		void Reserve(int32 ExpectedNumElements)
		{
			if (ExpectedNumElements > Data.Num())
			{
				const int32 ElementsToAdd = ExpectedNumElements - Data.Num();

				// allocate memory in the array itself
				int32 ElementIndex = Data.AddUninitialized(ElementsToAdd);

				// now mark the new elements as free
				for (int32 FreeIndex = ExpectedNumElements - 1; FreeIndex >= ElementIndex; --FreeIndex)
				{
					if (NumFreeIndices)
					{
						GetData(FirstFreeIndex).PrevFreeIndex = FreeIndex;
					}
					GetData(FreeIndex).PrevFreeIndex = -1;
					GetData(FreeIndex).NextFreeIndex = NumFreeIndices > 0 ? FirstFreeIndex : INDEX_NONE;
					FirstFreeIndex = FreeIndex;
					++NumFreeIndices;
				}

				if (ElementsToAdd == ExpectedNumElements)
				{
					AllocationFlags.Init(false, ElementsToAdd);
				}
				else
				{
					AllocationFlags.Add(false, ElementsToAdd);
				}
			}
		}

		/** Shrinks the array's storage to avoid slack. */
		void Shrink()
		{
			// Determine the highest allocated index in the data array.
			int32 MaxAllocatedIndex = AllocationFlags.FindLast(true);

			const int32 FirstIndexToRemove = MaxAllocatedIndex + 1;
			if (FirstIndexToRemove < Data.Num())
			{
				if (NumFreeIndices > 0)
				{
					// Look for elements in the free list that are in the memory to be freed.
					int32 FreeIndex = FirstFreeIndex;
					while (FreeIndex != INDEX_NONE)
					{
						if (FreeIndex >= FirstIndexToRemove)
						{
							const int32 PrevFreeIndex = GetData(FreeIndex).PrevFreeIndex;
							const int32 NextFreeIndex = GetData(FreeIndex).NextFreeIndex;
							if (NextFreeIndex != -1)
							{
								GetData(NextFreeIndex).PrevFreeIndex = PrevFreeIndex;
							}
							if (PrevFreeIndex != -1)
							{
								GetData(PrevFreeIndex).NextFreeIndex = NextFreeIndex;
							}
							else
							{
								FirstFreeIndex = NextFreeIndex;
							}
							--NumFreeIndices;

							FreeIndex = NextFreeIndex;
						}
						else
						{
							FreeIndex = GetData(FreeIndex).NextFreeIndex;
						}
					}
				}

				// Truncate unallocated elements at the end of the data array.
				Data.RemoveAt(FirstIndexToRemove, Data.Num() - FirstIndexToRemove);
				AllocationFlags.RemoveAt(FirstIndexToRemove, AllocationFlags.Num() - FirstIndexToRemove);
			}

			// Shrink the data array.
			Data.Shrink();
		}

		/** Compacts the allocated elements into a contiguous index range. */
		/** Returns true if any elements were relocated, false otherwise. */
		bool Compact()
		{
			int32 NumFree = NumFreeIndices;
			if (NumFree == 0)
			{
				return false;
			}

			bool bResult = false;

			FElementOrFreeListLink* ElementData = Data.GetData();

			int32 EndIndex = Data.Num();
			int32 TargetIndex = EndIndex - NumFree;
			int32 FreeIndex = FirstFreeIndex;
			while (FreeIndex != -1)
			{
				int32 NextFreeIndex = GetData(FreeIndex).NextFreeIndex;
				if (FreeIndex < TargetIndex)
				{
					// We need an element here
					do
					{
						--EndIndex;
					} while (!AllocationFlags[EndIndex]);

					RelocateConstructItems<FElementOrFreeListLink>(ElementData + FreeIndex, ElementData + EndIndex, 1);
					AllocationFlags[FreeIndex] = true;

					bResult = true;
				}

				FreeIndex = NextFreeIndex;
			}

			Data.RemoveAt(TargetIndex, NumFree);
			AllocationFlags.RemoveAt(TargetIndex, NumFree);

			NumFreeIndices = 0;
			FirstFreeIndex = -1;

			return bResult;
		}

		/** Compacts the allocated elements into a contiguous index range. Does not change the iteration order of the elements. */
		/** Returns true if any elements were relocated, false otherwise. */
		bool CompactStable()
		{
			if (NumFreeIndices == 0)
			{
				return false;
			}

			// Copy the existing elements to a new array.
			TSparseArray<ElementType, Allocator> CompactedArray;
			CompactedArray.Empty(Num());
			for (TConstIterator It(*this); It; ++It)
			{
				new(CompactedArray.AddUninitialized()) ElementType(*It);
			}

			// Replace this array with the compacted array.
			Exchange(*this, CompactedArray);

			return true;
		}

		/** Sorts the elements using the provided comparison class. */
		template<typename PREDICATE_CLASS>
		void Sort(const PREDICATE_CLASS& Predicate)
		{
			if (Num() > 0)
			{
				// Compact the elements array so all the elements are contiguous.
				Compact();

				// Sort the elements according to the provided comparison class.
				::Sort(&GetData(0), Num(), FElementCompareClass< PREDICATE_CLASS >(Predicate));
			}
		}

		/** Sorts the elements assuming < operator is defined for ElementType. */
		void Sort()
		{
			Sort(TLess<ElementType>());
		}

		/** Stable sorts the elements using the provided comparison class. */
		template<typename PREDICATE_CLASS>
		void StableSort(const PREDICATE_CLASS& Predicate)
		{
			if (Num() > 0)
			{
				// Compact the elements array so all the elements are contiguous.
				CompactStable();

				// Sort the elements according to the provided comparison class.
				::StableSort(&GetData(0), Num(), FElementCompareClass<PREDICATE_CLASS>(Predicate));
			}
		}

		/** Stable sorts the elements assuming < operator is defined for ElementType. */
		void StableSort()
		{
			StableSort(TLess<ElementType>());
		}

		/**
		 * Helper function to return the amount of memory allocated by this container
		 * Only returns the size of allocations made directly by the container, not the elements themselves.
		 * @return number of bytes allocated by this container
		 */
		uint32 GetAllocatedSize(void) const
		{
			return	(Data.Max()) * sizeof(FElementOrFreeListLink) +
				AllocationFlags.GetAllocatedSize();
		}

		bool IsCompact() const
		{
			return NumFreeIndices == 0;
		}

		/**
		 * Equality comparison operator.
		 * Checks that both arrays have the same elements and element indices; that means that unallocated elements are signifigant!
		 */
		friend bool operator==(const TSparseArray& A, const TSparseArray& B)
		{
			if (A.GetMaxIndex() != B.GetMaxIndex())
			{
				return false;
			}

			for (int32 ElementIndex = 0; ElementIndex < A.GetMaxIndex(); ElementIndex++)
			{
				const bool bIsAllocatedA = A.IsAllocated(ElementIndex);
				const bool bIsAllocatedB = B.IsAllocated(ElementIndex);
				if (bIsAllocatedA != bIsAllocatedB)
				{
					return false;
				}
				else if (bIsAllocatedA)
				{
					if (A[ElementIndex] != B[ElementIndex])
					{
						return false;
					}
				}
			}

			return true;
		}

		/**
		 * Inequality comparison operator.
		 * Checks that both arrays have the same elements and element indices; that means that unallocated elements are signifigant!
		 */
		friend bool operator!=(const TSparseArray& A, const TSparseArray& B)
		{
			return !(A == B);
		}

		TSparseArray()
			: FirstFreeIndex(-1)
			, NumFreeIndices(0)
		{}
		TSparseArray(TSparseArray&& InCopy)
		{
			MoveOrCopy(*this, InCopy);
		}
		TSparseArray(const TSparseArray& InCopy)
			: FirstFreeIndex(-1)
			, NumFreeIndices(0)
		{
			*this = InCopy;
		}

		TSparseArray& operator=(TSparseArray&& InCopy)
		{
			if (this != &InCopy)
			{
				MoveOrCopy(*this, InCopy);
			}
			return *this;
		}
		TSparseArray& operator=(const TSparseArray& InCopy)
		{
			if (this != &InCopy)
			{
				int32 SrcMax = InCopy.GetMaxIndex();

				// Reallocate the array.
				Empty(SrcMax);
				Data.AddUninitialized(SrcMax);

				// Copy the other array's element allocation state.
				FirstFreeIndex = InCopy.FirstFreeIndex;
				NumFreeIndices = InCopy.NumFreeIndices;
				AllocationFlags = InCopy.AllocationFlags;

				// Determine whether we need per element construction or bulk copy is fine
				if constexpr (!std::is_trivially_copy_constructible_v<ElementType>)
				{
					FElementOrFreeListLink* DestData = (FElementOrFreeListLink*)Data.GetData();
					const FElementOrFreeListLink* SrcData = (FElementOrFreeListLink*)InCopy.Data.GetData();

					// Use the inplace new to copy the element to an array element
					for (int32 Index = 0; Index < SrcMax; ++Index)
					{
						FElementOrFreeListLink& DestElement = DestData[Index];
						const FElementOrFreeListLink& SrcElement = SrcData[Index];
						if (InCopy.IsAllocated(Index))
						{
							::new((uint8*)&DestElement.ElementData) ElementType(*(const ElementType*)&SrcElement.ElementData);
						}
						else
						{
							DestElement.PrevFreeIndex = SrcElement.PrevFreeIndex;
							DestElement.NextFreeIndex = SrcElement.NextFreeIndex;
						}
					}
				}
				else
				{
					// Use the much faster path for types that allow it
					Memcpy(Data.GetData(), InCopy.Data.GetData(), sizeof(FElementOrFreeListLink) * SrcMax);
				}
			}
			return *this;
		}

	private:
		template <typename SparseArrayType>
		FORCEINLINE static typename void MoveOrCopy(SparseArrayType& ToArray, SparseArrayType& FromArray)
		{
			if constexpr (TContainerTraits<SparseArrayType>::MoveWillEmptyContainer)
			{
				// Destruct the allocated elements.
				if constexpr (!std::is_trivially_destructible_v<ElementType>::Value)
				{
					for (ElementType& Element : ToArray)
					{
						DestructItem(&Element);
					}
				}

				ToArray.Data = (DataType&&)FromArray.Data;
				ToArray.AllocationFlags = (AllocationBitArrayType&&)FromArray.AllocationFlags;

				ToArray.FirstFreeIndex = FromArray.FirstFreeIndex;
				ToArray.NumFreeIndices = FromArray.NumFreeIndices;
				FromArray.FirstFreeIndex = -1;
				FromArray.NumFreeIndices = 0;
			}
			else
			{
				ToArray = FromArray;
			}
		}

	public:
		// Accessors.
		ElementType& operator[](int32 Index)
		{
			check(Index >= 0 && Index < Data.Num() && Index < AllocationFlags.Num());
			return *(ElementType*)&GetData(Index).ElementData;
		}
		const ElementType& operator[](int32 Index) const
		{
			check(Index >= 0 && Index < Data.Num() && Index < AllocationFlags.Num());
			return *(ElementType*)&GetData(Index).ElementData;
		}
		int32 PointerToIndex(const ElementType* Ptr) const
		{
			check(Data.Num());
			int32 Index = Ptr - &GetData(0);
			check(Index >= 0 && Index < Data.Num() && Index < AllocationFlags.Num() && AllocationFlags[Index]);
			return Index;
		}
		bool IsValidIndex(int32 Index) const
		{
			return AllocationFlags.IsValidIndex(Index) && AllocationFlags[Index];
		}
		bool IsAllocated(int32 Index) const { return AllocationFlags[Index]; }
		int32 GetMaxIndex() const { return Data.Num(); }
		int32 Num() const { return Data.Num() - NumFreeIndices; }

		FORCEINLINE void CheckAddress(const ElementType* Addr) const
		{
			Data.CheckAddress(Addr);
		}

	private:
		template<bool bConst>
		class TBaseIterator
		{
		public:
			typedef TConstSetBitIterator<typename Allocator::BitArrayAllocator> BitArrayItType;

		private:
			typedef typename std::conditional_t<bConst, const TSparseArray, TSparseArray> ArrayType;
			typedef typename std::conditional_t<bConst, const ElementType, ElementType> ItElementType;

		public:
			explicit TBaseIterator(ArrayType& InArray, const BitArrayItType& InBitArrayIt)
				: Array(InArray)
				, BitArrayIt(InBitArrayIt)
			{
			}

			FORCEINLINE TBaseIterator& operator++()
			{
				// Iterate to the next set allocation flag.
				++BitArrayIt;
				return *this;
			}

			FORCEINLINE int32 GetIndex() const { return BitArrayIt.GetIndex(); }

			FORCEINLINE friend bool operator==(const TBaseIterator& Lhs, const TBaseIterator& Rhs) { return Lhs.BitArrayIt == Rhs.BitArrayIt && &Lhs.Array == &Rhs.Array; }
			FORCEINLINE friend bool operator!=(const TBaseIterator& Lhs, const TBaseIterator& Rhs) { return Lhs.BitArrayIt != Rhs.BitArrayIt || &Lhs.Array != &Rhs.Array; }

			/** conversion to "bool" returning true if the iterator is valid. */
			FORCEINLINE explicit operator bool() const
			{
				return !!BitArrayIt;
			}

			/** inverse of the "bool" operator */
			FORCEINLINE bool operator !() const
			{
				return !(bool)*this;
			}

			FORCEINLINE ItElementType& operator*() const { return Array[GetIndex()]; }
			FORCEINLINE ItElementType* operator->() const { return &Array[GetIndex()]; }
			FORCEINLINE const FRelativeBitReference& GetRelativeBitReference() const { return BitArrayIt; }

		protected:
			ArrayType&     Array;
			BitArrayItType BitArrayIt;
		};

	public:

		/** Iterates over all allocated elements in a sparse array. */
		class TIterator : public TBaseIterator<false>
		{
		public:
			TIterator(TSparseArray& InArray)
				: TBaseIterator<false>(InArray, TConstSetBitIterator<typename Allocator::BitArrayAllocator>(InArray.AllocationFlags))
			{
			}

			TIterator(TSparseArray& InArray, const typename TBaseIterator<false>::BitArrayItType& InBitArrayIt)
				: TBaseIterator<false>(InArray, InBitArrayIt)
			{
			}

			/** Safely removes the current element from the array. */
			void RemoveCurrent()
			{
				this->Array.RemoveAt(this->GetIndex());
			}
		};

		/** Iterates over all allocated elements in a const sparse array. */
		class TConstIterator : public TBaseIterator<true>
		{
		public:
			TConstIterator(const TSparseArray& InArray)
				: TBaseIterator<true>(InArray, TConstSetBitIterator<typename Allocator::BitArrayAllocator>(InArray.AllocationFlags))
			{
			}

			TConstIterator(const TSparseArray& InArray, const typename TBaseIterator<true>::BitArrayItType& InBitArrayIt)
				: TBaseIterator<true>(InArray, InBitArrayIt)
			{
			}
		};

#if TSPARSEARRAY_RANGED_FOR_CHECKS
		class TRangedForIterator : public TIterator
		{
		public:
			TRangedForIterator(TSparseArray& InArray, const typename TBaseIterator<false>::BitArrayItType& InBitArrayIt)
				: TIterator(InArray, InBitArrayIt)
				, InitialNum(InArray.Num())
			{
			}

		private:
			int32 InitialNum;

			friend FORCEINLINE bool operator!=(const TRangedForIterator& Lhs, const TRangedForIterator& Rhs)
			{
				ensuref(Lhs.Array.Num() == Lhs.InitialNum, TEXT("Container has changed during ranged-for iteration!"));
				return *(TIterator*)&Lhs != *(TIterator*)&Rhs;
			}
		};

		class TRangedForConstIterator : public TConstIterator
		{
		public:
			TRangedForConstIterator(const TSparseArray& InArray, const typename TBaseIterator<true>::BitArrayItType& InBitArrayIt)
				: TConstIterator(InArray, InBitArrayIt)
				, InitialNum(InArray.Num())
			{
			}

		private:
			int32 InitialNum;

			friend FORCEINLINE bool operator!=(const TRangedForConstIterator& Lhs, const TRangedForConstIterator& Rhs)
			{
				ensuref(Lhs.Array.Num() == Lhs.InitialNum, TEXT("Container has changed during ranged-for iteration!"));
				return *(TIterator*)&Lhs != *(TIterator*)&Rhs;
			}
		};
#else
		using TRangedForIterator = TIterator;
		using TRangedForConstIterator = TConstIterator;
#endif

		/** Creates an iterator for the contents of this array */
		TIterator CreateIterator()
		{
			return TIterator(*this);
		}

		/** Creates a const iterator for the contents of this array */
		TConstIterator CreateConstIterator() const
		{
			return TConstIterator(*this);
		}

	public:
		FORCEINLINE TRangedForIterator      begin() { return TRangedForIterator(*this, TConstSetBitIterator<typename Allocator::BitArrayAllocator>(AllocationFlags)); }
		FORCEINLINE TRangedForConstIterator begin() const { return TRangedForConstIterator(*this, TConstSetBitIterator<typename Allocator::BitArrayAllocator>(AllocationFlags)); }
		FORCEINLINE TRangedForIterator      end() { return TRangedForIterator(*this, TConstSetBitIterator<typename Allocator::BitArrayAllocator>(AllocationFlags, AllocationFlags.Num())); }
		FORCEINLINE TRangedForConstIterator end() const { return TRangedForConstIterator(*this, TConstSetBitIterator<typename Allocator::BitArrayAllocator>(AllocationFlags, AllocationFlags.Num())); }

	public:
		// 仅仅访问在两个稀疏数组中都有的元素 
		template<typename SubsetAllocator = FDefaultBitArrayAllocator>
		class TConstSubsetIterator
		{
		public:
			TConstSubsetIterator(const TSparseArray& InArray, const TBitArray<SubsetAllocator>& InBitArray) :
				Array(InArray),
				BitArrayIt(InArray.AllocationFlags, InBitArray)
			{}
			FORCEINLINE TConstSubsetIterator& operator++()
			{
				// Iterate to the next element which is both allocated and has its bit set in the other bit array.
				++BitArrayIt;
				return *this;
			}
			FORCEINLINE int32 GetIndex() const { return BitArrayIt.GetIndex(); }

			/** conversion to "bool" returning true if the iterator is valid. */
			FORCEINLINE explicit operator bool() const
			{
				return !!BitArrayIt;
			}
			/** inverse of the "bool" operator */
			FORCEINLINE bool operator !() const
			{
				return !(bool)*this;
			}

			FORCEINLINE const ElementType& operator*() const { return Array(GetIndex()); }
			FORCEINLINE const ElementType* operator->() const { return &Array(GetIndex()); }
			FORCEINLINE const FRelativeBitReference& GetRelativeBitReference() const { return BitArrayIt; }
		private:
			const TSparseArray& Array;
			TConstDualSetBitIterator<typename Allocator::BitArrayAllocator, SubsetAllocator> BitArrayIt;
		};

		/** Concatenation operators */
		TSparseArray& operator+=(const TSparseArray& OtherArray)
		{
			this->Reserve(this->Num() + OtherArray.Num());
			for (typename TSparseArray::TConstIterator It(OtherArray); It; ++It)
			{
				this->Add(*It);
			}
			return *this;
		}
		TSparseArray& operator+=(const TArray<ElementType>& OtherArray)
		{
			this->Reserve(this->Num() + OtherArray.Num());
			for (int32 Idx = 0; Idx < OtherArray.Num(); Idx++)
			{
				this->Add(OtherArray[Idx]);
			}
			return *this;
		}

	private:
		// 使用占位符来避免TArray实例化冗余的元素(防止Free部分的元素依旧被调用构造) 
		typedef TSparseArrayElementOrFreeListLink<
			TAlignedBytes<sizeof(ElementType), alignof(ElementType)>
		> FElementOrFreeListLink;

		// 为稀疏数组的特殊数据存储创造的特殊套娃谓语判断 
		template <typename PREDICATE_CLASS>
		class FElementCompareClass
		{
			const PREDICATE_CLASS& Predicate;

		public:
			FElementCompareClass(const PREDICATE_CLASS& InPredicate)
				: Predicate(InPredicate)
			{}

			bool operator()(const FElementOrFreeListLink& A, const FElementOrFreeListLink& B) const
			{
				return Predicate(*(ElementType*)&A.ElementData, *(ElementType*)&B.ElementData);
			}
		};

		FElementOrFreeListLink& GetData(int32 Index)
		{
			return ((FElementOrFreeListLink*)Data.GetData())[Index];
		}

		const FElementOrFreeListLink& GetData(int32 Index) const
		{
			return ((FElementOrFreeListLink*)Data.GetData())[Index];
		}

		// 元素数据
		typedef TArray<FElementOrFreeListLink, typename Allocator::ElementAllocator> DataType;
		DataType Data;

		// 标记数组中Index的状况 
		typedef TBitArray<typename Allocator::BitArrayAllocator> AllocationBitArrayType;
		AllocationBitArrayType AllocationFlags;

		// 数组中第一个空闲元素的位置，同时也是FreeList的链表头
		int32 FirstFreeIndex;
		
		// FreeList中的元素数量 
		int32 NumFreeIndices;
	};
}

// Type traits
namespace Fuko
{
	template<typename ElementType, typename Allocator>
	struct TContainerTraits<TSparseArray<ElementType, Allocator> > : public TContainerTraitsBase<TSparseArray<ElementType, Allocator> >
	{
		enum {
			MoveWillEmptyContainer =
			TContainerTraits<typename TSparseArray<ElementType, Allocator>::DataType>::MoveWillEmptyContainer &&
			TContainerTraits<typename TSparseArray<ElementType, Allocator>::AllocationBitArrayType>::MoveWillEmptyContainer
		};
	};
}

// operator new 
inline void* operator new(size_t Size, const Fuko::FSparseArrayAllocationInfo& Allocation)
{
	check(Allocation.Pointer);
	return Allocation.Pointer;
}

