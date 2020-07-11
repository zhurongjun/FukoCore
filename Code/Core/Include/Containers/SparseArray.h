#pragma once
#include "CoreType.h"
#include "CoreConfig.h"
#include "Memory/Allocators.h"
#include "Array.h"
#include "CoreMinimal/Assert.h"
#include "BitArray.h"

// forward
namespace Fuko
{
	template<typename T,typename Alloc = PmrAllocator>
	class TSparseArray;
}

// Structs
namespace Fuko
{
	template<typename SizeType>
	struct TSparseArrayAllocationInfo
	{
		SizeType Index;
		void* Pointer;
	};

	template<typename T,typename SizeType>
	union TElementOrFreeList
	{
		T ElementData;
		struct
		{
			SizeType Last;
			SizeType Next;
		};
	};
}

// TSparseArray
namespace Fuko
{
	template<typename T,typename Alloc>
	class TSparseArray final
	{
	public:
		// 使用占位符来避免TArray实例化冗余的元素(防止Free部分的元素依旧被调用构造) 
		using SizeType = typename Alloc::SizeType;
		using ElementOrFreeListLink = TElementOrFreeList<TStorage<T>,SizeType>;		
		using SparseArrayAllocationInfo = TSparseArrayAllocationInfo<SizeType>;
		using DataArrayType = TArray<ElementOrFreeListLink, Alloc>;
	private:
		uint32*			m_BitArray;			// bit array
		DataArrayType	m_Data;				// contains all elements
		SizeType		m_BitArraySize;		// bit array size
		SizeType		m_FirstFreeIndex;	// first free index in data array
		SizeType		m_NumFreeIndices;	// free indices num

		// for sparse array compare 
		template <typename Predicate>
		class FElementCompareClass
		{
			const Predicate& Pred;

		public:
			FElementCompareClass(const Predicate& InPredicate)
				: Pred(InPredicate)
			{}

			bool operator()(const ElementOrFreeListLink& A, const ElementOrFreeListLink& B) const
			{
				return Pred((const T&)A.ElementData, (const T&)B.ElementData);
			}
		};

		//---------------------------------Begin help functions---------------------------------
		FORCEINLINE uint32* _GetBitArray() const { return m_BitArray; }
		FORCEINLINE uint32* _GetBitArray() { return m_BitArray; }
		FORCEINLINE void _SetBit(SizeType Index, bool Value) { Algo::SetBit(_GetBitArray(), Index, Value); }
		FORCEINLINE bool _GetBit(SizeType Index) const { return Algo::GetBit(_GetBitArray(), Index); }
		FORCEINLINE void _SetBitRange(SizeType Index, bool Value, SizeType Count) { Algo::SetBitRange(_GetBitArray(), Index, Count, Value); }

		FORCEINLINE void _ResizeBitArray()
		{
			if (m_BitArraySize / NumBitsPerDWORD == m_Data.Max() / NumBitsPerDWORD) return;
			// resize
			auto& BitAlloc = m_Data.GetAllocator();
			SizeType OldSize = Algo::CalculateNumWords(m_BitArraySize);
			SizeType NewSize = Algo::CalculateNumWords(m_Data.Max());
			NewSize = BitAlloc.Reserve(m_BitArray, NewSize);
			// clean memory 
			if (NewSize > OldSize) Algo::SetWords(m_BitArray + OldSize, NewSize - OldSize, false);
			m_BitArraySize = NewSize * NumBitsPerDWORD;
		}
		FORCEINLINE void _GrowBitArray()
		{
			if (m_BitArraySize >= m_Data.Max()) return;
			// grow
			auto& BitAlloc = m_Data.GetAllocator();
			SizeType OldSize = Algo::CalculateNumWords(m_BitArraySize);
			SizeType NewSize = BitAlloc.GetGrow(Algo::CalculateNumWords(m_Data.Max()), Algo::CalculateNumWords(m_BitArraySize));
			NewSize = BitAlloc.Reserve(m_BitArray, NewSize);
			// clean memory 
			if(NewSize > OldSize) Algo::SetWords(m_BitArray + OldSize, NewSize - OldSize, false);
			m_BitArraySize = NewSize * NumBitsPerDWORD;
		}
		FORCEINLINE void _FreeBitArray()
		{
			if (m_BitArray) m_BitArraySize = m_Data.GetAllocator().Free(m_BitArray) * NumBitsPerDWORD;
		}
		//----------------------------------End help functions----------------------------------
	public:
		// construct 
		TSparseArray(const Alloc& InAlloc = Alloc())
			: m_Data(InAlloc)
			, m_BitArray(nullptr)
			, m_BitArraySize(0)
			, m_FirstFreeIndex(INDEX_NONE)
			, m_NumFreeIndices(0)
		{}

		// copy construct 
		TSparseArray(const TSparseArray& InCopy, const Alloc& InAlloc = Alloc())
			: m_Data(InCopy.m_Data, InAlloc)
			, m_BitArray(nullptr)
			, m_BitArraySize(0)
			, m_FirstFreeIndex(INDEX_NONE)
			, m_NumFreeIndices(0)
		{
			*this = InCopy;
		}

		// move construct 
		TSparseArray(TSparseArray&& Other)
			: m_Data(std::move(Other.m_Data))
			, m_BitArray(Other.m_BitArray)
			, m_BitArraySize(Other.m_BitArraySize)
			, m_FirstFreeIndex(Other.m_FirstFreeIndex)
			, m_NumFreeIndices(Other.m_NumFreeIndices)
		{
			Other.m_BitArray = nullptr;
			Other.m_BitArraySize = 0;
			Other.m_FirstFreeIndex = INDEX_NONE;
			Other.m_NumFreeIndices = 0;
		}

		// copy assign operator 
		TSparseArray& operator=(const TSparseArray& Other)
		{
			if (this == &Other) return *this;

			SizeType SrcMax = Other.GetMaxIndex();

			// Reallocate the array.
			Empty(SrcMax);
			m_Data.AddUninitialized(SrcMax);
			_ResizeBitArray();

			// Copy the other array's element allocation state.
			m_FirstFreeIndex = Other.m_FirstFreeIndex;
			m_NumFreeIndices = Other.m_NumFreeIndices;

			if constexpr (!std::is_trivially_copy_constructible_v<T>)
			{
				ElementOrFreeListLink* DestData = (ElementOrFreeListLink*)m_Data.GetData();
				const ElementOrFreeListLink* SrcData = (ElementOrFreeListLink*)Other.m_Data.GetData();

				for (SizeType Index = 0; Index < SrcMax; ++Index)
				{
					ElementOrFreeListLink& DestElement = DestData[Index];
					const ElementOrFreeListLink& SrcElement = SrcData[Index];
					if (Other.IsAllocated(Index))
					{
						// call copy construct
						::new((uint8*)&DestElement.ElementData) T(*(const T*)&SrcElement.ElementData);
					}
					else
					{
						// copy free list info 
						DestElement.Last = SrcElement.Last;
						DestElement.Next = SrcElement.Next;
					}
				}
			}
			else
			{
				// Use the much faster path for types that allow it
				Memcpy(m_Data.GetData(), Other.m_Data.GetData(), sizeof(ElementOrFreeListLink) * SrcMax);
			}

			return *this;
		}

		// move assign operator 
		TSparseArray& operator=(TSparseArray&& Other)
		{
			if (this != &Other)
			{
				// Destruct the allocated elements.
				if constexpr (!std::is_trivially_destructible_v<T>)
				{
					for (T& Element : *this)
					{
						DestructItems(&Element);
					}
				}
				_FreeBitArray();

				// move data 
				m_Data = std::move(Other.m_Data);
				m_BitArray = Other.m_BitArray;
				m_FirstFreeIndex = Other.m_FirstFreeIndex;
				m_NumFreeIndices = Other.m_NumFreeIndices;

				// invalidate other 
				Other.m_FirstFreeIndex = INDEX_NONE;
				Other.m_NumFreeIndices = 0;
			}
			return *this;
		}
		
		// destructor
		~TSparseArray()
		{
			Empty();
		}

		// get information 
		bool IsCompact() const { return m_NumFreeIndices == 0; }
		bool IsAllocated(SizeType Index) const { return _GetBit(Index); }
		SizeType GetMaxIndex() const { return m_Data.Num(); }
		SizeType Num() const { return m_Data.Num() - m_NumFreeIndices; }
		SizeType Max() const { return m_Data.Max(); }
		Alloc& GetAllocator() { return m_Data.GetAllocator(); }
		const Alloc& GetAllocator() const { return m_Data.GetAllocator(); }

		// allocate index, set the index as allocated, but won't call construct and remove from free list
		SparseArrayAllocationInfo AllocateIndex(SizeType Index)
		{
			check(Index >= 0);
			check(Index < GetMaxIndex());
			check(!IsAllocated(Index));

			// Flag the element as allocated.
			_SetBit(Index, true);

			// Set the allocation info.
			SparseArrayAllocationInfo Result;
			Result.Index = Index;
			Result.Pointer = &m_Data[Index].ElementData;

			return Result;
		}

		// special add 
		SparseArrayAllocationInfo AddUninitialized()
		{
			SizeType Index;
			if (m_NumFreeIndices)
			{
				// Remove and use the first index from the list of free elements.
				Index = m_FirstFreeIndex;
				m_FirstFreeIndex = m_Data[m_FirstFreeIndex].Next;
				--m_NumFreeIndices;

				// break link of first node 
				if (m_NumFreeIndices) m_Data[m_FirstFreeIndex].Last = INDEX_NONE;
			}
			else
			{
				// Add a new element.
				Index = m_Data.AddUninitialized(1);
				_GrowBitArray();
			}

			return AllocateIndex(Index);
		}
		SparseArrayAllocationInfo AddUninitializedAtLowestFreeIndex(SizeType& LowestFreeIndexSearchStart)
		{
			SizeType Index;
			if (m_NumFreeIndices)
			{
				Index = Algo::FindBit(_GetBitArray(), m_Data.Num(), false);
				LowestFreeIndexSearchStart = Index + 1;

				auto& IndexData = m_Data[Index];

				// Update FirstFreeIndex
				if (m_FirstFreeIndex == Index) m_FirstFreeIndex = IndexData.Next;

				// Link the linked list for remove a node 
				if (IndexData.Next != INDEX_NONE) m_Data[IndexData.Next].Last = IndexData.Last;
				if (IndexData.Last != INDEX_NONE) m_Data[IndexData.Last].Next = IndexData.Next;

				--m_NumFreeIndices;
			}
			else
			{
				// Add a new element.
				Index = m_Data.AddUninitialized(1);
				_GrowBitArray();
			}

			return AllocateIndex(Index);
		}

		// add
		SizeType Add(const T& Element)
		{
			SparseArrayAllocationInfo Allocation = AddUninitialized();
			new(Allocation.Pointer) T(Element);
			return Allocation.Index;
		}
		SizeType Add(T&& Element)
		{
			SparseArrayAllocationInfo Allocation = AddUninitialized();
			new(Allocation.Pointer) T(std::move(Element));
			return Allocation.Index;
		}
		SizeType AddAtLowestFreeIndex(const T& Element, SizeType& LowestFreeIndexSearchStart)
		{
			SparseArrayAllocationInfo Allocation = AddUninitializedAtLowestFreeIndex(LowestFreeIndexSearchStart);
			new(Allocation) T(Element);
			return Allocation.Index;
		}

		// insert 
		SparseArrayAllocationInfo InsertUninitialized(SizeType Index)
		{
			check(Index >= 0 && Index < m_Data.Num());
			check(m_NumFreeIndices > 0);
			check(!IsAllocated(Index));

			// Remove the index from the list of free elements.
			--m_NumFreeIndices;

			// Update FirstFreeIndex
			if (m_FirstFreeIndex == Index) m_FirstFreeIndex = IndexData.Next;

			// Link the linked list for remove a node 
			if (IndexData.Next != INDEX_NONE) m_Data[IndexData.Next].Last = IndexData.Last;
			if (IndexData.Last != INDEX_NONE) m_Data[IndexData.Last].Next = IndexData.Next;

			return AllocateIndex(Index);
		}
		void Insert(SizeType Index, const T& Element)
		{
			new(InsertUninitialized(Index)) T(Element);
		}

		// remove
		void RemoveAtUninitialized(SizeType Index, SizeType Count = 1)
		{
			for (; Count; --Count)
			{
				check(IsAllocated(Index));

				// link head to self 
				if (m_NumFreeIndices) m_Data[m_FirstFreeIndex].Last = Index;

				auto& IndexData = m_Data[Index];
				IndexData.Last = INDEX_NONE;
				
				// link self to head
				IndexData.Next = m_NumFreeIndices > 0 ? m_FirstFreeIndex: INDEX_NONE;
				
				// instead head to self
				m_FirstFreeIndex = Index;
				++m_NumFreeIndices;

				// make allocation flag 
				_SetBit(Index, false);
				++Index;
			}
		}
		void RemoveAt(SizeType Index, SizeType Count = 1)
		{
			if constexpr (!std::is_trivially_destructible_v<T>)
			{
				for (SizeType It = Index, ItCount = Count; ItCount; ++It, --ItCount)
				{
					((T&)m_Data[It].ElementData).~T();
				}
			}
			RemoveAtUninitialized(Index, Count);
		}

		// set num & empty & shrink...
		void Empty(SizeType ExpectedNumElements = 0)
		{
			// Destruct the allocated elements.
			if constexpr (!std::is_trivially_destructible_v<T>)
			{
				for (TIterator It(*this); It; ++It)
				{
					T& Element = *It;
					Element.~T();
				}
			}

			// Free the allocated elements.
			if (_GetBitArray()) Algo::SetWords(_GetBitArray(), Algo::CalculateNumWords(m_BitArraySize), false);
			m_Data.Empty(ExpectedNumElements);
			_ResizeBitArray();
			m_FirstFreeIndex = INDEX_NONE;
			m_NumFreeIndices = 0;
		}
		void Reset(SizeType ExpectedNumElements = 0)
		{
			// Destruct the allocated elements.
			if constexpr (!std::is_trivially_destructible_v<T>)
			{
				for (TIterator It(*this); It; ++It)
				{
					T& Element = *It;
					Element.~T();
				}
			}

			// Free the allocated elements.
			if (_GetBitArray()) Algo::SetWords(_GetBitArray(), Algo::CalculateNumWords(m_BitArraySize), false);
			m_Data.Reset(ExpectedNumElements);
			_ResizeBitArray();
			m_FirstFreeIndex = INDEX_NONE;
			m_NumFreeIndices = 0;
		}
		void Reserve(SizeType ExpectedNumElements)
		{
			if (ExpectedNumElements < m_Data.Num()) return;
			
			m_Data.Reserve(ExpectedNumElements);
			_ResizeBitArray();
		}
		void Shrink()
		{
			// Determine the highest allocated index in the data array.
			SizeType MaxAllocatedIndex = Algo::FindLastBit(_GetBitArray(), m_Data.Num(), true);

			const SizeType FirstIndexToRemove = MaxAllocatedIndex + 1;
			if (FirstIndexToRemove < m_Data.Num())
			{
				// remove free element over bounds
				if (m_NumFreeIndices > 0)
				{
					// Look for elements in the free list that are in the memory to be freed.
					SizeType FreeIndex = m_FirstFreeIndex;
					while (FreeIndex != INDEX_NONE)
					{
						if (FreeIndex >= FirstIndexToRemove)
						{
							auto& IndexData = m_Data[FreeIndex];
							// Update FirstFreeIndex
							if (m_FirstFreeIndex == FreeIndex) m_FirstFreeIndex = IndexData.Next;

							// Link the linked list for remove a node 
							if (IndexData.Next != INDEX_NONE) m_Data[IndexData.Next].Last = IndexData.Last;
							if (IndexData.Last != INDEX_NONE) m_Data[IndexData.Last].Next = IndexData.Next;
							--m_NumFreeIndices;

							FreeIndex = IndexData.Next;
						}
						else
						{
							FreeIndex = m_Data[FreeIndex].Next;
						}
					}
				}

				// Truncate unallocated elements at the end of the data array.
				SizeType NumRemove = m_Data.Num() - FirstIndexToRemove;
				m_Data.RemoveAt(FirstIndexToRemove, NumRemove);
				_SetBitRange(FirstIndexToRemove, false, NumRemove);
				_ResizeBitArray();
			}

			// Shrink the data array.
			m_Data.Shrink();
		}

		// compact elements return whether any element been removed
		bool Compact()
		{
			if (m_NumFreeIndices == 0) return false;

			SizeType NumFree = m_NumFreeIndices;
			bool bResult = false;
			ElementOrFreeListLink* ElementData = m_Data.GetData();

			SizeType EndIndex = m_Data.Num();
			SizeType TargetIndex = EndIndex - NumFree;
			SizeType FreeIndex = m_FirstFreeIndex;
			while (FreeIndex != INDEX_NONE)
			{
				SizeType NextIndex = m_Data[FreeIndex].Next;
				if (FreeIndex < TargetIndex)
				{
					// find last allocated element
					do
					{
						--EndIndex;
					} while (!IsAllocated(EndIndex));

					// move element to the hole 
					RelocateConstructItems(ElementData + FreeIndex, ElementData + EndIndex, 1);
					_SetBit(FreeIndex, true);
					bResult = true;
				}
				FreeIndex = NextIndex;
			}

			// remove unused data 
			m_Data.RemoveAt(TargetIndex, NumFree);
			_SetBitRange(TargetIndex, false, NumFree);
			_ResizeBitArray();
			m_NumFreeIndices = 0;
			m_FirstFreeIndex = INDEX_NONE;
			return bResult;
		}
		bool CompactStable()
		{
			if (m_NumFreeIndices == 0) return false;
			SizeType NumFree = m_NumFreeIndices;
			bool bResult = false;
			ElementOrFreeListLink* ElementData = m_Data.GetData();

			SizeType EndIndex = m_Data.Num();
			SizeType TargetIndex = EndIndex - NumFree;
			SizeType WriteIndex = 0;
			SizeType ReadIndex = 0;
			// skip compact range 
			while (IsAllocated(WriteIndex) && WriteIndex != EndIndex) ++WriteIndex;
			ReadIndex = WriteIndex + 1;
			while (ReadIndex != EndIndex)
			{
				// skip hole
				while (!IsAllocated(ReadIndex) && ReadIndex != EndIndex) ++ReadIndex;

				// move item 
				while (ReadIndex != EndIndex && IsAllocated(ReadIndex))
				{
					RelocateConstructItems(ElementData + WriteIndex, ElementData + ReadIndex, 1);
					_SetBit(WriteIndex, true);
					++WriteIndex;
					++ReadIndex;
				}
			}

			// remove unused data 
			m_Data.RemoveAt(TargetIndex, NumFree);
			_SetBitRange(TargetIndex, false, NumFree);
			_ResizeBitArray();
			m_NumFreeIndices = 0;
			m_FirstFreeIndex = INDEX_NONE;
			return bResult;
		}

		// sort 
		void Sort()
		{
			Sort(TLess<T>());
		}
		template<typename Predicate>
		void Sort(Predicate&& Pred)
		{
			if (Num() > 0)
			{
				// Compact the elements array so all the elements are contiguous.
				Compact();

				// Sort the elements according to the provided comparison class.
				::Sort(m_Data.GetData(), Num(), FElementCompareClass<Predicate>(std::forward<Predicate>(Pred)));
			}
		}
		void StableSort()
		{
			StableSort(TLess<T>());
		}
		template<typename Predicate>
		void StableSort(Predicate&& Pred)
		{
			if (Num() > 0)
			{
				// Compact the elements array so all the elements are contiguous.
				CompactStable();

				// Sort the elements according to the provided comparison class.
				::StableSort(m_Data.GetData(), Num(), FElementCompareClass<Predicate>(std::forward<Predicate>(Pred)));
			}
		}

		// compare 
		friend bool operator==(const TSparseArray& A, const TSparseArray& B) 
		{
			if (A.GetMaxIndex() != B.GetMaxIndex()) return false;

			for (SizeType ElementIndex = 0; ElementIndex < A.GetMaxIndex(); ElementIndex++)
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
		friend bool operator!=(const TSparseArray& A, const TSparseArray& B) { return !(A == B); }

		// accessor
		T& operator[](SizeType Index)
		{
			check(Index >= 0 && Index < m_Data.Num());
			return (T&)m_Data[Index].ElementData;
		}
		const T& operator[](SizeType Index) const
		{
			check(Index >= 0 && Index < m_Data.Num());
			return (const T&)m_Data[Index].ElementData;
		}
				
		// check 
		bool IsValidIndex(SizeType Index) const { return  m_Data.IsValidIndex(Index) && IsAllocated(Index); }
		// debug check 
		FORCEINLINE void CheckAddress(const T* Addr) const { m_Data.CheckAddress(Addr); }

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
		TSparseArray& operator+=(const TArray<T>& OtherArray)
		{
			this->Reserve(this->Num() + OtherArray.Num());
			for (SizeType Idx = 0; Idx < OtherArray.Num(); Idx++)
			{
				this->Add(OtherArray[Idx]);
			}
			return *this;
		}

		//------------------------------------------Iterator---------------------------------------------
		class TIterator
		{
			TSparseArray&					m_Array;
			TConstSetBitIterator<SizeType>	m_BitArrayIt;
#if FUKO_DEBUG
			SizeType m_InitialNum;
#endif
		public:
			explicit TIterator(TSparseArray& InArray, SizeType StartIndex = 0)
				: m_Array(InArray)
				, m_BitArrayIt(m_Array._GetBitArray(), m_Array.GetMaxIndex(), StartIndex)
#if FUKO_DEBUG
				, m_InitialNum(InArray.Num())
#endif
			{}
			FORCEINLINE TIterator& operator++()
			{
				++m_BitArrayIt;
				return *this;
			}

			FORCEINLINE SizeType GetIndex() const { return m_BitArrayIt.GetIndex(); }
			FORCEINLINE friend bool operator==(const TIterator& Lhs, const TIterator& Rhs) { return Lhs.m_BitArrayIt == Rhs.m_BitArrayIt && &Lhs.m_Array == &Rhs.m_Array; }
			FORCEINLINE friend bool operator!=(const TIterator& Lhs, const TIterator& Rhs)
			{
#if FUKO_DEBUG
				ensuref(Lhs.m_Array.Num() == Lhs.m_InitialNum, TEXT("Container has changed during ranged-for iteration!"));
#endif
				return Lhs.m_BitArrayIt != Rhs.m_BitArrayIt || &Lhs.m_Array != &Rhs.m_Array;
			}
			FORCEINLINE explicit operator bool() const { return !!m_BitArrayIt; }
			FORCEINLINE bool operator !() const { return !(bool)*this; }
			FORCEINLINE T& operator*() const { return m_Array[GetIndex()]; }
			FORCEINLINE T* operator->() const { return &m_Array[GetIndex()]; }
			void RemoveCurrent() { this->m_Array.RemoveAt(this->GetIndex()); }
		};
		class TConstIterator
		{
			const TSparseArray&				m_Array;
			TConstSetBitIterator<SizeType>	m_BitArrayIt;
#if FUKO_DEBUG
			SizeType m_InitialNum;
#endif
		public:
			explicit TConstIterator(const TSparseArray& InArray, SizeType StartIndex = 0)
				: m_Array(InArray)
				, m_BitArrayIt(m_Array._GetBitArray(), m_Array.GetMaxIndex())
#if FUKO_DEBUG
				, m_InitialNum(InArray.Num())
#endif
			{}
			FORCEINLINE TConstIterator& operator++()
			{
				++m_BitArrayIt;
				return *this;
			}

			FORCEINLINE SizeType GetIndex() const { return m_BitArrayIt.GetIndex(); }
			FORCEINLINE friend bool operator==(const TConstIterator& Lhs, const TConstIterator
				& Rhs) { return Lhs.m_BitArrayIt == Rhs.m_BitArrayIt && &Lhs.m_Array == &Rhs.m_Array; }
			FORCEINLINE friend bool operator!=(const TConstIterator& Lhs, const TConstIterator& Rhs)
			{
#if FUKO_DEBUG
				ensuref(Lhs.m_Array.Num() == Lhs.m_InitialNum, TEXT("Container has changed during ranged-for iteration!"));
#endif
				return Lhs.m_BitArrayIt != Rhs.m_BitArrayIt || &Lhs.m_Array != &Rhs.m_Array;
			}
			FORCEINLINE explicit operator bool() const { return !!m_BitArrayIt; }
			FORCEINLINE bool operator !() const { return !(bool)*this; }
			FORCEINLINE const T& operator*() const { return m_Array[GetIndex()]; }
			FORCEINLINE const T* operator->() const { return &m_Array[GetIndex()]; }
		};
		FORCEINLINE TIterator      begin()			{ return TIterator(*this); }
		FORCEINLINE TConstIterator begin() const	{ return TConstIterator(*this); }
		FORCEINLINE TIterator      end()			{ return TIterator(*this, GetMaxIndex()); }
		FORCEINLINE TConstIterator end() const		{ return TConstIterator(*this, GetMaxIndex()); }
	};
}
