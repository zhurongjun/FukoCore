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
	template<typename T>
	class TSparseArray final
	{
		using ElementType = T;

		// 使用占位符来避免TArray实例化冗余的元素(防止Free部分的元素依旧被调用构造) 
		using FElementOrFreeListLink = TSparseArrayElementOrFreeListLink<TAlignedBytes<sizeof(ElementType), alignof(ElementType)>>;
		using DataArrayType = TArray<FElementOrFreeListLink>;
		
		DataArrayType	m_Data;				// contains all element 
		BitArray		m_AllocationFlags;	// whether the element is be set
		int32			m_FirstFreeIndex;	// first free index in data array
		int32			m_NumFreeIndices;	// free indices num

	private:
		// for sparse array compare 
		template <typename Predicate>
		class FElementCompareClass
		{
			const Predicate& Pred;

		public:
			FElementCompareClass(const Predicate& InPredicate)
				: Pred(InPredicate)
			{}

			bool operator()(const FElementOrFreeListLink& A, const FElementOrFreeListLink& B) const
			{
				return Pred(*(ElementType*)&A.ElementData, *(ElementType*)&B.ElementData);
			}
		};

		FElementOrFreeListLink& GetData(int32 Index) { return m_Data[Index];}
		const FElementOrFreeListLink& GetData(int32 Index) const { return m_Data[Index]; }

	public:
		// construct 
		TSparseArray(IAllocator* BitArrayAlloc = DefaultAllocator(), IAllocator* ElementAlloc = DefaultAllocator())
			: m_Data(ElementAlloc)
			, m_AllocationFlags(BitArrayAlloc)
			, m_FirstFreeIndex(-1)
			, m_NumFreeIndices(0)
		{}

		// copy construct 
		TSparseArray(const TSparseArray& InCopy, IAllocator* BitArrayAlloc = DefaultAllocator(), IAllocator* ElementAlloc = DefaultAllocator())
			: m_Data(ElementAlloc)
			, m_AllocationFlags(BitArrayAlloc)
			, m_FirstFreeIndex(-1)
			, m_NumFreeIndices(0)
		{
			*this = InCopy;
		}

		// move construct 
		TSparseArray(TSparseArray&& Other)
			: m_Data(std::move(Other.m_Data))
			, m_AllocationFlags(std::move(Other.m_AllocationFlags))
			, m_FirstFreeIndex(Other.m_FirstFreeIndex)
			, m_NumFreeIndices(Other.m_NumFreeIndices)
		{
			Other.m_FirstFreeIndex = -1;
			Other.NumFreeIndices = 0;
		}

		// copy assign operator 
		TSparseArray& operator=(const TSparseArray& Other)
		{
			if (this == &Other) return *this;

			int32 SrcMax = Other.GetMaxIndex();

			// Reallocate the array.
			Empty(SrcMax);
			m_Data.AddUninitialized(SrcMax);

			// Copy the other array's element allocation state.
			m_FirstFreeIndex = Other.m_FirstFreeIndex;
			m_NumFreeIndices = Other.m_NumFreeIndices;
			m_AllocationFlags = Other.m_AllocationFlags;

			if constexpr (!std::is_trivially_copy_constructible_v<ElementType>)
			{
				FElementOrFreeListLink* DestData = (FElementOrFreeListLink*)m_Data.GetData();
				const FElementOrFreeListLink* SrcData = (FElementOrFreeListLink*)Other.m_Data.GetData();

				for (int32 Index = 0; Index < SrcMax; ++Index)
				{
					FElementOrFreeListLink& DestElement = DestData[Index];
					const FElementOrFreeListLink& SrcElement = SrcData[Index];
					if (Other.IsAllocated(Index))
					{
						// call copy construct
						::new((uint8*)&DestElement.ElementData) ElementType(*(const ElementType*)&SrcElement.ElementData);
					}
					else
					{
						// copy free list info 
						DestElement.PrevFreeIndex = SrcElement.PrevFreeIndex;
						DestElement.NextFreeIndex = SrcElement.NextFreeIndex;
					}
				}
			}
			else
			{
				// Use the much faster path for types that allow it
				Memcpy(m_Data.GetData(), Other.m_Data.GetData(), sizeof(FElementOrFreeListLink) * SrcMax);
			}

			return *this;
		}

		// move assign operator 
		TSparseArray& operator=(TSparseArray&& Other)
		{
			if (this != &Other)
			{
				// Destruct the allocated elements.
				if constexpr (!std::is_trivially_destructible_v<ElementType>::Value)
				{
					for (ElementType& Element : *this)
					{
						DestructItem(&Element);
					}
				}

				m_Data = std::move(Other.Data);
				m_AllocationFlags = std::move(Other.m_AllocationFlags);

				m_FirstFreeIndex = Other.m_FirstFreeIndex;
				m_NumFreeIndices = Other.m_NumFreeIndices;
				Other.m_FirstFreeIndex = -1;
				Other.m_NumFreeIndices = 0;
			}
			return *this;
		}
		
		// destructor
		~TSparseArray()
		{
			Empty();
		}

		// allocate index, set the index as allocated, but won't call construct 
		FSparseArrayAllocationInfo AllocateIndex(int32 Index)
		{
			check(Index >= 0);
			check(Index < GetMaxIndex());
			check(!m_AllocationFlags[Index]);

			// Flag the element as allocated.
			m_AllocationFlags[Index] = true;

			// Set the allocation info.
			FSparseArrayAllocationInfo Result;
			Result.Index = Index;
			Result.Pointer = &GetData(Result.Index).ElementData;

			return Result;
		}

		// special add 
		FSparseArrayAllocationInfo AddUninitialized()
		{
			int32 Index;
			if (m_NumFreeIndices)
			{
				// Remove and use the first index from the list of free elements.
				Index = m_FirstFreeIndex;
				m_FirstFreeIndex = GetData(m_FirstFreeIndex).NextFreeIndex;
				--m_NumFreeIndices;
				if (m_NumFreeIndices)
				{
					GetData(m_FirstFreeIndex).PrevFreeIndex = -1;
				}
			}
			else
			{
				// Add a new element.
				Index = m_Data.AddUninitialized(1);
				m_AllocationFlags.Add(false);
			}

			return AllocateIndex(Index);
		}
		FSparseArrayAllocationInfo AddUninitializedAtLowestFreeIndex(int32& LowestFreeIndexSearchStart)
		{
			int32 Index;
			if (m_NumFreeIndices)
			{
				Index = m_AllocationFlags.FindAndSetFirstZeroBit(LowestFreeIndexSearchStart);
				LowestFreeIndexSearchStart = Index + 1;

				auto& IndexData = GetData(Index);

				// Update FirstFreeIndex
				if (m_FirstFreeIndex == Index)
				{
					m_FirstFreeIndex = IndexData.NextFreeIndex;
				}

				// Link the linked list for remove a node 
				if (IndexData.NextFreeIndex >= 0)
				{
					GetData(IndexData.NextFreeIndex).PrevFreeIndex = IndexData.PrevFreeIndex;
				}
				if (IndexData.PrevFreeIndex >= 0)
				{
					GetData(IndexData.PrevFreeIndex).NextFreeIndex = IndexData.NextFreeIndex;
				}

				--m_NumFreeIndices;
			}
			else
			{
				// Add a new element.
				Index = m_Data.AddUninitialized(1);
				m_AllocationFlags.Add(true);
			}

			FSparseArrayAllocationInfo Result;
			Result.Index = Index;
			Result.Pointer = &GetData(Result.Index).ElementData;
			return Result;
		}

		// add
		int32 Add(const ElementType& Element)
		{
			FSparseArrayAllocationInfo Allocation = AddUninitialized();
			new(Allocation) ElementType(Element);
			return Allocation.Index;
		}
		int32 Add(ElementType&& Element)
		{
			FSparseArrayAllocationInfo Allocation = AddUninitialized();
			new(Allocation) ElementType(std::move(Element));
			return Allocation.Index;
		}
		int32 AddAtLowestFreeIndex(const ElementType& Element, int32& LowestFreeIndexSearchStart)
		{
			FSparseArrayAllocationInfo Allocation = AddUninitializedAtLowestFreeIndex(LowestFreeIndexSearchStart);
			new(Allocation) ElementType(Element);
			return Allocation.Index;
		}

		// special insert 
		FSparseArrayAllocationInfo InsertUninitialized(int32 Index)
		{
			// Enlarge the array to include the given index.
			if (Index >= m_Data.Num())
			{
				m_Data.AddUninitialized(Index + 1 - m_Data.Num());
				while (m_AllocationFlags.Num() < m_Data.Num())
				{
					const int32 FreeIndex = m_AllocationFlags.Num();
					GetData(FreeIndex).PrevFreeIndex = -1;
					GetData(FreeIndex).NextFreeIndex = m_FirstFreeIndex;
					if (m_NumFreeIndices)
					{
						GetData(m_FirstFreeIndex).PrevFreeIndex = FreeIndex;
					}
					m_FirstFreeIndex = FreeIndex;
					always_check(m_AllocationFlags.Add(false) == FreeIndex);
					++m_NumFreeIndices;
				};
			}

			// Verify that the specified index is free.
			check(!m_AllocationFlags[Index]);

			// Remove the index from the list of free elements.
			--m_NumFreeIndices;
			const int32 PrevFreeIndex = GetData(Index).PrevFreeIndex;
			const int32 NextFreeIndex = GetData(Index).NextFreeIndex;
			if (PrevFreeIndex != -1)
			{
				GetData(PrevFreeIndex).NextFreeIndex = NextFreeIndex;
			}
			else
			{
				m_FirstFreeIndex = NextFreeIndex;
			}
			if (NextFreeIndex != -1)
			{
				GetData(NextFreeIndex).PrevFreeIndex = PrevFreeIndex;
			}

			return AllocateIndex(Index);
		}

		// insert 
		void Insert(int32 Index, typename TTypeTraits<ElementType>::ConstInitType Element)
		{
			new(InsertUninitialized(Index)) ElementType(Element);
		}

		// remove at 
		void RemoveAt(int32 Index, int32 Count = 1)
		{
			if constexpr (!std::is_trivially_destructible_v<ElementType>::Value)
			{
				for (int32 It = Index, ItCount = Count; ItCount; ++It, --ItCount)
				{
					((ElementType&)GetData(It).ElementData).~ElementType();
				}
			}

			RemoveAtUninitialized(Index, Count);
		}

		// remove, but won't destruct item 
		void RemoveAtUninitialized(int32 Index, int32 Count = 1)
		{
			for (; Count; --Count)
			{
				check(m_AllocationFlags[Index]);

				// Mark the element as free and add it to the free element list.
				if (m_NumFreeIndices)
				{
					GetData(m_FirstFreeIndex).PrevFreeIndex = Index;
				}
				auto& IndexData = GetData(Index);
				IndexData.PrevFreeIndex = -1;
				IndexData.NextFreeIndex = m_NumFreeIndices > 0 ? m_FirstFreeIndex: INDEX_NONE;
				m_FirstFreeIndex = Index;
				++m_NumFreeIndices;
				m_AllocationFlags[Index] = false;

				++Index;
			}
		}

		// set num & empty & shrink...
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
			m_Data.Empty(ExpectedNumElements);
			m_FirstFreeIndex = -1;
			m_NumFreeIndices = 0;
			m_AllocationFlags.Empty(ExpectedNumElements);
		}
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
			m_Data.Reset();
			m_FirstFreeIndex = -1;
			m_NumFreeIndices = 0;
			m_AllocationFlags.Reset();
		}
		void Reserve(int32 ExpectedNumElements)
		{
			if (ExpectedNumElements < m_Data.Num()) return;
			
			const int32 ElementsToAdd = ExpectedNumElements - m_Data.Num();

			// allocate memory in the array itself
			int32 ElementIndex = m_Data.AddUninitialized(ElementsToAdd);

			// now mark the new elements as free
			for (int32 FreeIndex = ExpectedNumElements - 1; FreeIndex >= ElementIndex; --FreeIndex)
			{
				if (m_NumFreeIndices)
				{
					GetData(m_FirstFreeIndex).PrevFreeIndex = FreeIndex;
				}
				GetData(FreeIndex).PrevFreeIndex = -1;
				GetData(FreeIndex).NextFreeIndex = m_NumFreeIndices> 0 ? m_NumFreeIndices : INDEX_NONE;
				m_FirstFreeIndex = FreeIndex;
				++m_NumFreeIndices;
			}

			if (ElementsToAdd == ExpectedNumElements)
			{
				m_AllocationFlags.Init(false, ElementsToAdd);
			}
			else
			{
				m_AllocationFlags.Add(false, ElementsToAdd);
			}
		}
		void Shrink()
		{
			// Determine the highest allocated index in the data array.
			int32 MaxAllocatedIndex = m_AllocationFlags.FindLast(true);

			const int32 FirstIndexToRemove = MaxAllocatedIndex + 1;
			if (FirstIndexToRemove < m_Data.Num())
			{
				if (m_NumFreeIndices > 0)
				{
					// Look for elements in the free list that are in the memory to be freed.
					int32 FreeIndex = m_FirstFreeIndex;
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
								m_FirstFreeIndex = NextFreeIndex;
							}
							--m_NumFreeIndices;

							FreeIndex = NextFreeIndex;
						}
						else
						{
							FreeIndex = GetData(FreeIndex).NextFreeIndex;
						}
					}
				}

				// Truncate unallocated elements at the end of the data array.
				m_Data.RemoveAt(FirstIndexToRemove, m_Data.Num() - FirstIndexToRemove);
				m_AllocationFlags.RemoveAt(FirstIndexToRemove, m_AllocationFlags.Num() - FirstIndexToRemove);
			}

			// Shrink the data array.
			m_Data.Shrink();
		}

		// compact elements return whether any element been removed
		bool Compact()
		{
			if (m_NumFreeIndices == 0) return false;

			int32 NumFree = m_NumFreeIndices;
			bool bResult = false;
			FElementOrFreeListLink* ElementData = m_Data.GetData();

			int32 EndIndex = m_Data.Num();
			int32 TargetIndex = EndIndex - NumFree;
			int32 FreeIndex = m_FirstFreeIndex;
			while (FreeIndex != -1)
			{
				int32 NextFreeIndex = GetData(FreeIndex).NextFreeIndex;
				if (FreeIndex < TargetIndex)
				{
					// find last allocated element
					do
					{
						--EndIndex;
					} while (!m_AllocationFlags[EndIndex]);

					// move element to the hole 
					RelocateConstructItems(ElementData + FreeIndex, ElementData + EndIndex, 1);
					m_AllocationFlags[FreeIndex] = true;

					bResult = true;
				}

				FreeIndex = NextFreeIndex;
			}

			m_Data.RemoveAt(TargetIndex, NumFree);
			m_AllocationFlags.RemoveAt(TargetIndex, NumFree);

			m_NumFreeIndices = 0;
			m_FirstFreeIndex = -1;

			return bResult;
		}
		bool CompactStable()
		{
			if (m_NumFreeIndices == 0) return false;

			// Copy the existing elements to a new array.
			TSparseArray<ElementType> CompactedArray(m_AllocationFlags.GetAllocator(),m_Data.GetAllocator());
			CompactedArray.Empty(Num());
			for (TConstIterator It(*this); It; ++It)
			{
				new(CompactedArray.AddUninitialized()) ElementType(std::move(*It));
			}

			// Replace this array with the compacted array.
			Swap(*this, CompactedArray);

			return true;
		}

		// sort 
		void Sort()
		{
			Sort(TLess<ElementType>());
		}
		template<typename Predicate>
		void Sort(const Predicate& Pred)
		{
			if (Num() > 0)
			{
				// Compact the elements array so all the elements are contiguous.
				Compact();

				// Sort the elements according to the provided comparison class.
				::Sort(&GetData(0), Num(), FElementCompareClass<Predicate>(Pred));
			}
		}
		void StableSort()
		{
			StableSort(TLess<ElementType>());
		}
		template<typename Predicate>
		void StableSort(const Predicate& Pred)
		{
			if (Num() > 0)
			{
				// Compact the elements array so all the elements are contiguous.
				CompactStable();

				// Sort the elements according to the provided comparison class.
				::StableSort(&GetData(0), Num(), FElementCompareClass<Predicate>(Pred));
			}
		}

		// get information 
		bool IsCompact() const { return m_NumFreeIndices == 0; }
		bool IsAllocated(int32 Index) const { return m_AllocationFlags[Index]; }
		int32 GetMaxIndex() const { return m_Data.Num(); }
		int32 Num() const { return m_Data.Num() - m_NumFreeIndices; }

		// compare 
		friend bool operator==(const TSparseArray& A, const TSparseArray& B)
		{
			if (A.GetMaxIndex() != B.GetMaxIndex()) return false;

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
		friend bool operator!=(const TSparseArray& A, const TSparseArray& B)
		{
			return !(A == B);
		}

		// accessor
		ElementType& operator[](int32 Index)
		{
			check(Index >= 0 && Index < m_Data.Num() && Index < m_AllocationFlags.Num());
			return *(ElementType*)&GetData(Index).ElementData;
		}
		const ElementType& operator[](int32 Index) const
		{
			check(Index >= 0 && Index < m_Data.Num() && Index < m_AllocationFlags.Num());
			return *(ElementType*)&GetData(Index).ElementData;
		}
		
		// convert pointer to index 
		int32 PointerToIndex(const ElementType* Ptr) const
		{
			check(m_Data.Num());
			int32 Index = Ptr - &GetData(0);
			check(Index >= 0 && Index < m_Data.Num() && Index < m_AllocationFlags.Num() && m_AllocationFlags[Index]);
			return Index;
		}
		
		// check 
		bool IsValidIndex(int32 Index) const
		{
			return m_AllocationFlags.IsValidIndex(Index) && m_AllocationFlags[Index];
		}
		
		// debug check 
		FORCEINLINE void CheckAddress(const ElementType* Addr) const
		{
			m_Data.CheckAddress(Addr);
		}

		// append 
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

		//------------------------------------------Iterator---------------------------------------------
	private:
		template<bool bConst>
		class TBaseIterator
		{
		public:
			typedef TConstSetBitIterator BitArrayItType;

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
		class TIterator : public TBaseIterator<false>
		{
		public:
			TIterator(TSparseArray& InArray)
				: TBaseIterator<false>(InArray, TConstSetBitIterator(InArray.m_AllocationFlags))
			{}

			TIterator(TSparseArray& InArray, const typename TBaseIterator<false>::BitArrayItType& InBitArrayIt)
				: TBaseIterator<false>(InArray, InBitArrayIt)
			{}

			/** Safely removes the current element from the array. */
			void RemoveCurrent()
			{
				this->Array.RemoveAt(this->GetIndex());
			}
		};
		class TConstIterator : public TBaseIterator<true>
		{
		public:
			TConstIterator(const TSparseArray& InArray)
				: TBaseIterator<true>(InArray, TConstSetBitIterator(InArray.m_AllocationFlags))
			{}

			TConstIterator(const TSparseArray& InArray, const typename TBaseIterator<true>::BitArrayItType& InBitArrayIt)
				: TBaseIterator<true>(InArray, InBitArrayIt)
			{}
		};

#if TSPARSEARRAY_RANGED_FOR_CHECKS
		class TRangedForIterator : public TIterator
		{
		public:
			TRangedForIterator(TSparseArray& InArray, const typename TBaseIterator<false>::BitArrayItType& InBitArrayIt)
				: TIterator(InArray, InBitArrayIt)
				, InitialNum(InArray.Num())
			{}

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
			{}

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
	public:
		FORCEINLINE TRangedForIterator      begin()			{ return TRangedForIterator(*this, TConstSetBitIterator(m_AllocationFlags)); }
		FORCEINLINE TRangedForConstIterator begin() const	{ return TRangedForConstIterator(*this, TConstSetBitIterator(m_AllocationFlags)); }
		FORCEINLINE TRangedForIterator      end()			{ return TRangedForIterator(*this, TConstSetBitIterator(m_AllocationFlags, m_AllocationFlags.Num())); }
		FORCEINLINE TRangedForConstIterator end() const		{ return TRangedForConstIterator(*this, TConstSetBitIterator(m_AllocationFlags, m_AllocationFlags.Num())); }

		// create iterator 
		TIterator CreateIterator()
		{
			return TIterator(*this);
		}
		TConstIterator CreateConstIterator() const
		{
			return TConstIterator(*this);
		}
	public:
		// 仅仅访问在两个稀疏数组中都有的元素 
		class TConstSubsetIterator
		{
		public:
			TConstSubsetIterator(const TSparseArray& InArray, const BitArray& InBitArray) :
				Array(InArray),
				BitArrayIt(InArray.m_AllocationFlags, InBitArray)
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
			const TSparseArray&			Array;
			TConstDualSetBitIterator	BitArrayIt;
		};
	};
}

// operator new 
inline void* operator new(size_t Size, const Fuko::FSparseArrayAllocationInfo& Allocation)
{
	check(Allocation.Pointer);
	return Allocation.Pointer;
}

