#pragma once
#include "CoreConfig.h"
#include "CoreType.h"
#include "Templates/TypeTraits.h"
#include "CoreMinimal/Assert.h"
#include "SparseArray.h"
#include "Memory/Memory.h"
#include "Memory/Allocators.h"

// Key functions 
namespace Fuko
{
	template<typename T, bool bInAllowDuplicateKeys = false>
	struct DefaultKeyFuncs
	{
		using KeyType = T;
		using ElementType = T;

		static constexpr bool bAllowDuplicateKeys = bInAllowDuplicateKeys;

		static FORCEINLINE const KeyType& GetSetKey(const ElementType& Element) { return Element; }
		template<typename ComparableKey>
		static FORCEINLINE bool Matches(const KeyType& A, ComparableKey B) { return A == B; }
		template<typename ComparableKey>
		static FORCEINLINE uint32 GetKeyHash(ComparableKey Key) { return GetTypeHash(Key); }
	};
}

// Set Element
namespace Fuko
{
	class SetElementId
	{
		int32 Index;
	public:
		FORCEINLINE SetElementId(int32 InIndex = INDEX_NONE) : Index(InIndex) {}
		FORCEINLINE bool IsValidId() const { return Index != INDEX_NONE; }
		FORCEINLINE operator int32() const { return Index; }
		FORCEINLINE friend bool operator==(const SetElementId& A, const SetElementId& B) { return A.Index == B.Index; }
		FORCEINLINE void Reset() { Index = INDEX_NONE; }
	};

	template <typename InElementType>
	class TSetElement
	{
	public:
		using ElementType = InElementType;

		ElementType Value;
		mutable int32 HashIndex;
		mutable SetElementId HashNextId;
	public:
		TSetElement() = default;
		TSetElement(TSetElement&&) = default;
		TSetElement(const TSetElement&) = default;
		TSetElement& operator=(TSetElement&&) = default;
		TSetElement& operator=(const TSetElement&) = default;

		template <typename InitType>
		explicit FORCEINLINE TSetElement(InitType&& InValue) : Value(std::forward<InitType>(InValue)) {}

		FORCEINLINE bool operator==(const TSetElement& Other) const { return this->Value == Other.Value; }
		FORCEINLINE bool operator!=(const TSetElement& Other) const { return this->Value != Other.Value; }
	};
}

// TSet
namespace Fuko
{
	template<typename InElementType, typename KeyFuncs = DefaultKeyFuncs<InElementType>>
	class TSet
	{
	private:
		// Key and element 
		using KeyType = typename KeyFuncs::KeyType;
		using ElementType = InElementType;
		using SetElementType = TSetElement<ElementType>;
		using ElementArrayType = TSparseArray<SetElementType>;

		// special compare for set element 
		template <typename Predicate>
		class FElementCompareClass
		{
			TDereferenceWrapper<ElementType, Predicate> Pred;
		public:
			FORCEINLINE FElementCompareClass(const Predicate& InPredicate) : Pred(InPredicate) {}
			FORCEINLINE bool operator()(const SetElementType& A, const SetElementType& B) const { return Pred(A.Value, B.Value); }
		};

		mutable IAllocator*		m_HashAllocator;	// hash bucket allocator 
		mutable SetElementId*	m_Hash;				// hash bucket 
		mutable int32			m_HashSize;			// hash bucket size 
		ElementArrayType		m_Elements;			// elements 

		//------------------------------helper functions------------------------------
		static FORCEINLINE uint32 GetNumberOfHashBuckets(uint32 NumHashedElements)
		{
			static constexpr uint32 MinNumberOfHashedElements = 4;
			static constexpr uint32 BaseNumberOfHashBuckets = 8;
			static constexpr uint32 AverageNumberOfElementsPerHashBucket = 2;

			if (NumHashedElements >= MinNumberOfHashedElements)
			{
				return FMath::RoundUpToPowerOfTwo(NumHashedElements / AverageNumberOfElementsPerHashBucket + BaseNumberOfHashBuckets);
			}

			return 1;
		}

		// get elementid in hash bucket 
		FORCEINLINE SetElementId& BucketId(int32 HashIndex) const { return m_Hash[HashIndex & (m_HashSize - 1)]; }
		
		// rehash, before call it, m_HashSize Must be updated 
		void Rehash() const
		{
			check(FMath::IsPowerOfTwo(m_HashSize));
			
			// realloc hash 
			if (m_Hash) (SetElementId*)m_HashAllocator->Realloc(m_Hash, m_HashSize * sizeof(SetElementId));
			else m_Hash = (SetElementId*)m_HashAllocator->Alloc(m_HashSize * sizeof(SetElementId));
			
			if (m_HashSize)
			{
				// reset hash 
				for (SetElementId* Ptr = m_Hash, *End = m_Hash + m_HashSize; Ptr != End; ++Ptr)
				{
					Ptr->Reset();
				}

				// Add the existing elements to the new hash.
				for (typename ElementArrayType::TConstIterator ElementIt(m_Elements); ElementIt; ++ElementIt)
				{
					HashElement(SetElementId(ElementIt.GetIndex()), *ElementIt);
				}
			}
		}
		bool ConditionalRehash(int32 NumHashedElements, bool bAllowShrinking = false) const
		{
			// Calculate the desired hash size for the specified number of elements.
			const int32 DesiredHashSize = GetNumberOfHashBuckets(NumHashedElements);

			// If the hash hasn't been created yet, or is smaller than the desired hash size, rehash.
			if (NumHashedElements > 0 &&	// must have element 
				(	!m_HashSize ||			// no exist allocation 
					m_HashSize < DesiredHashSize ||	// exist allocation is too small
					(m_HashSize > DesiredHashSize && bAllowShrinking)))	// need shrinking 
			{
				m_HashSize = DesiredHashSize;
				Rehash();
				return true;
			}
			else
			{
				return false;
			}
		}

		// link element to hash bucket 
		FORCEINLINE void LinkElement(SetElementId ElementId, const SetElementType& Element, uint32 KeyHash) const
		{
			// Compute the hash bucket the element goes in.
			Element.HashIndex = KeyHash & (m_HashSize - 1);

			// Link the element into the hash bucket.
			SetElementId& IdRef = m_Hash[Element.HashIndex];
			Element.HashNextId = IdRef;
			IdRef = ElementId;
		}

		// compute hash and link it to hash bucket 
		FORCEINLINE void HashElement(SetElementId ElementId, const SetElementType& Element) const
		{
			LinkElement(ElementId, Element, KeyFuncs::GetKeyHash(KeyFuncs::GetSetKey(Element.Value)));
		}

		// implement emplace, before call this function, element has been added to sparse array
		// we only need check duplicate and link element to bucket 
		SetElementId EmplaceImpl(uint32 KeyHash, SetElementType& Element, SetElementId ElementId, bool* bIsAlreadyInSetPtr)
		{
			// if we not support duplicate key, then check whether the key is unique 
			if constexpr (!KeyFuncs::bAllowDuplicateKeys)
			{
				bool bIsAlreadyInSet = false;
				// Don't bother searching for a duplicate if this is the first element we're adding
				if (m_Elements.Num() != 1)
				{
					SetElementId ExistingId = FindIdByHash(KeyHash, KeyFuncs::GetSetKey(Element.Value));
					bIsAlreadyInSet = ExistingId.IsValidId();
					if (bIsAlreadyInSet)
					{
						// cover the old element 
						Memmove(&m_Elements[ExistingId].Value, &Element.Value, sizeof(SetElementType));

						// Then remove the new element.
						m_Elements.RemoveAtUninitialized(ElementId);

						// Then point the return value at the replaced element.
						ElementId = ExistingId;
					}
				}
				// add element 
				if (!bIsAlreadyInSet)
				{
					// Check if the hash needs to be resized.
					if (!ConditionalRehash(m_Elements.Num()))
					{
						// If the rehash didn't add the new element to the hash, add it.
						LinkElement(ElementId, Element, KeyHash);
					}
				}
				// return whether the element is in set
				if (bIsAlreadyInSetPtr)
				{
					*bIsAlreadyInSetPtr = bIsAlreadyInSet;
				}
			}
			else
			{
				// Check if the hash needs to be resized.
				if (!ConditionalRehash(m_Elements.Num()))
				{
					// If the rehash didn't add the new element to the hash, add it.
					LinkElement(ElementId, Element, KeyHash);
				}
				// always add 
				if (bIsAlreadyInSetPtr)
				{
					*bIsAlreadyInSetPtr = false;
				}
			}
			return ElementId;
		}

		// implement remove 
		template<typename ComparableKey>
		FORCEINLINE int32 RemoveImpl(uint32 KeyHash, const ComparableKey& Key)
		{
			int32 NumRemovedElements = 0;

			// get the head of hash linked list 
			SetElementId* NextElementId = &BucketId(KeyHash);
			while (NextElementId->IsValidId())
			{
				auto& Element = m_Elements[*NextElementId];
				if (KeyFuncs::Matches(KeyFuncs::GetSetKey(Element.Value), Key))
				{
					// link to next element id 
					*NextElementId = Element.HashNextId;
					// remove element
					m_Elements.RemoveAt(*NextElementId);

					NumRemovedElements++;
					if constexpr (!KeyFuncs::bAllowDuplicateKeys) break;
				}
				else
				{
					NextElementId = &Element.HashNextId;
				}
			}

			return NumRemovedElements;
		}
		//----------------------------end helper functions----------------------------
	public:
		// construct 
		FORCEINLINE TSet(
			IAllocator* HashAlloc = DefaultAllocator(),
			IAllocator* BitArrayAlloc = DefaultAllocator(),
			IAllocator* ElementAlloc = DefaultAllocator())
			: m_HashSize(0)
			, m_Hash(nullptr)
			, m_HashAllocator(HashAlloc)
			, m_Elements(BitArrayAlloc, ElementAlloc)
		{
			check(m_HashAllocator != nullptr);
		}
		TSet(std::initializer_list<ElementType> InitList,
			IAllocator* HashAlloc = DefaultAllocator(),
			IAllocator* BitArrayAlloc = DefaultAllocator(),
			IAllocator* ElementAlloc = DefaultAllocator())
			: m_HashSize(0)
			, m_Hash(nullptr)
			, m_HashAllocator(HashAlloc)
			, m_Elements(BitArrayAlloc, ElementAlloc) 
		{
			check(m_HashAllocator != nullptr);
			Append(InitList);
		}


		// copy construct 
		FORCEINLINE TSet(const TSet& Other,
			IAllocator* HashAlloc = DefaultAllocator(),
			IAllocator* BitArrayAlloc = DefaultAllocator(),
			IAllocator* ElementAlloc = DefaultAllocator())
			: m_HashSize(0)
			, m_Hash(nullptr)
			, m_HashAllocator(HashAlloc)
			, m_Elements(BitArrayAlloc, ElementAlloc)
		{
			check(m_HashAllocator != nullptr);
			*this = Other;
		}
		FORCEINLINE explicit TSet(const TArray<ElementType>& InArray,
			IAllocator* HashAlloc = DefaultAllocator(),
			IAllocator* BitArrayAlloc = DefaultAllocator(),
			IAllocator* ElementAlloc = DefaultAllocator())
			: m_HashSize(0)
			, m_Hash(nullptr)
			, m_HashAllocator(HashAlloc)
			, m_Elements(BitArrayAlloc, ElementAlloc)
		{ 
			check(m_HashAllocator != nullptr);
			Append(InArray);
		}

		// move construct 
		TSet(TSet&& Other)
			: m_HashSize(Other.m_HashSize)
			, m_Hash(Other.m_Hash)
			, m_HashAllocator(Other.m_HashAllocator)
			, m_Elements(std::move(Other.m_Elements))
		{
			check(m_HashAllocator != nullptr);
			Other.m_HashSize = 0;
			Other.m_Hash = nullptr;
		}
		FORCEINLINE explicit TSet(TArray<ElementType>&& InArray,
			IAllocator* HashAlloc = DefaultAllocator(),
			IAllocator* BitArrayAlloc = DefaultAllocator(),
			IAllocator* ElementAlloc = DefaultAllocator())
			: m_HashSize(0)
			, m_Hash(nullptr)
			, m_HashAllocator(HashAlloc)
			, m_Elements(BitArrayAlloc, ElementAlloc)
		{ 
			check(m_HashAllocator != nullptr);
			Append(std::move(InArray));
		}

		// destructor 
		~TSet()
		{
			if (m_Hash)
			{
				m_HashAllocator->Free(m_Hash);
				m_Hash = nullptr;
				m_HashSize = 0;
			}
		}

		// assign 
		TSet& operator=(const TSet& Copy)
		{
			if (this == &Copy) return *this;
			
			int32 CopyHashSize = Copy.m_HashSize;

			// copy hash bucket 
			DestructItems(m_Hash, m_HashSize);
			if (m_Hash) m_Hash = m_HashAllocator->Realloc(m_Hash, CopyHashSize * sizeof(SetElementId), alignof(SetElementId));
			else m_Hash = m_HashAllocator->Alloc(CopyHashSize * sizeof(SetElementId), alignof(SetElementId));
			ConstructItems(m_Hash, Copy.m_Hash, CopyHashSize);

			// copy other info 
			m_HashSize = CopyHashSize;
			m_Elements = Copy.m_Elements;
			
			return *this;
		}
		TSet& operator=(std::initializer_list<ElementType> InitList)
		{
			Reset();
			Append(InitList);
			return *this;
		}

		// move assign 
		TSet& operator=(TSet&& Other)
		{
			if (this == &Other) return *this;
			
			// free memory 
			if (m_Hash) m_HashAllocator->Free(m_Hash);

			// move data 
			m_Elements = std::move(Other.m_Elements);
			m_Hash = Other.m_Hash;
			m_HashAllocator = Other.m_HashAllocator;
			m_HashSize = Other.m_HashSize;

			// invalidate other 
			Other.m_HashSize = 0;
			Other.m_Hash = nullptr;

			return *this;
		}

		// get information 
		FORCEINLINE int32 Num() const { return m_Elements.Num(); }
		FORCEINLINE int32 GetMaxIndex() const { return m_Elements.GetMaxIndex(); }

		// empty & reset & shrink & reserve 
		FORCEINLINE void Empty(int32 ExpectedNumElements = 0)
		{
			m_Elements.Empty(ExpectedNumElements);

			// Resize the hash to the desired size for the expected number of elements.
			if (!ConditionalRehash(ExpectedNumElements, true))
			{
				// If the hash was already the desired size, clear the references to the elements that have now been removed.
				for (int32 HashIndex = 0, LocalHashSize = m_HashSize; HashIndex < LocalHashSize; ++HashIndex)
				{
					BucketId(HashIndex) = SetElementId();
				}
			}
		}
		FORCEINLINE void Reset()
		{
			if (Num() == 0) return;

			// Reset the elements array.
			m_Elements.Reset();

			// Clear the references to the elements that have now been removed.
			SetElementId* Ptr = m_Hash;
			SetElementId* End = m_Hash + m_HashSize;
			for (; Ptr != End; ++Ptr)
			{
				Ptr->Reset();
			}
		}
		FORCEINLINE void Shrink()
		{
			m_Elements.Shrink();
			Relax();
		}
		FORCEINLINE void Reserve(int32 Number)
		{
			if (Number <= m_Elements.Num()) return;
			
			// Preallocates memory for array of elements
			m_Elements.Reserve(Number);

			// Calculate the corresponding hash size for the specified number of elements.
			const int32 NewHashSize = GetNumberOfHashBuckets(Number);

			// If the hash hasn't been created yet, or is smaller than the corresponding hash size, rehash
			// to force a preallocation of the hash table
			if (!m_HashSize || m_HashSize < NewHashSize)
			{
				m_HashSize = NewHashSize;
				Rehash();
			}
			
		}

		// compact 
		FORCEINLINE void Compact()
		{
			if (m_Elements.Compact())
			{
				Rehash();
			}
		}
		FORCEINLINE void CompactStable()
		{
			if (m_Elements.CompactStable())
			{
				Rehash();
			}
		}

		// relax, the element will symmetrical distribution 
		FORCEINLINE void Relax()
		{
			ConditionalRehash(m_Elements.Num(), true);
		}

		// check is valid 
		FORCEINLINE bool IsValidId(SetElementId Id) const
		{
			return	Id.IsValidId() &&
				Id >= 0 &&
				Id < m_Elements.GetMaxIndex() &&
				m_Elements.IsAllocated(Id);
		}

		// accessor
		FORCEINLINE ElementType& operator[](SetElementId Id)
		{
			return m_Elements[Id].Value;
		}
		FORCEINLINE const ElementType& operator[](SetElementId Id) const
		{
			return m_Elements[Id].Value;
		}

		// add 
		FORCEINLINE SetElementId Add(const InElementType&  InElement, bool* bIsAlreadyInSetPtr = nullptr) 
		{ 
			return Emplace(InElement, bIsAlreadyInSetPtr); 
		}
		FORCEINLINE SetElementId Add(InElementType&& InElement, bool* bIsAlreadyInSetPtr = nullptr) 
		{ 
			return Emplace(std::move(InElement), bIsAlreadyInSetPtr); 
		}
		FORCEINLINE SetElementId AddByHash(uint32 KeyHash, const InElementType& InElement, bool* bIsAlreadyInSetPtr = nullptr)
		{
			return EmplaceByHash(KeyHash, InElement, bIsAlreadyInSetPtr);
		}
		FORCEINLINE SetElementId AddByHash(uint32 KeyHash, InElementType&& InElement, bool* bIsAlreadyInSetPtr = nullptr)
		{
			return EmplaceByHash(KeyHash, std::move(InElement), bIsAlreadyInSetPtr);
		}

		// emplace 
		template <typename ArgsType>
		SetElementId Emplace(ArgsType&& Args, bool* bIsAlreadyInSetPtr = nullptr)
		{
			// Create a new element.
			FSparseArrayAllocationInfo ElementAllocation = m_Elements.AddUninitialized();
			SetElementType& Element = *new (ElementAllocation) SetElementType(std::forward<ArgsType>(Args));

			// compute hash 
			uint32 KeyHash = KeyFuncs::GetKeyHash(KeyFuncs::GetSetKey(Element.Value));
			
			// perform emplace 
			return EmplaceImpl(KeyHash, Element, ElementAllocation.Index, bIsAlreadyInSetPtr);
		}
		template <typename ArgsType>
		SetElementId EmplaceByHash(uint32 KeyHash, ArgsType&& Args, bool* bIsAlreadyInSetPtr = nullptr)
		{
			// Create a new element.
			FSparseArrayAllocationInfo ElementAllocation = m_Elements.AddUninitialized();
			SetElementType& Element = *new (ElementAllocation) SetElementType(std::forward<ArgsType>(Args));

			return EmplaceImpl(KeyHash, Element, ElementAllocation.Index, bIsAlreadyInSetPtr);
		}

		// append 
		void Append(const TArray<ElementType>& InElements)
		{
			Reserve(m_Elements.Num() + InElements.Num());
			for (const ElementType& Element : InElements)
			{
				Add(Element);
			}
		}
		void Append(TArray<ElementType>&& InElements)
		{
			Reserve(m_Elements.Num() + InElements.Num());
			for (ElementType& Element : InElements)
			{
				Add(std::move(Element));
			}
			InElements.Reset();
		}
		void Append(const TSet<ElementType, KeyFuncs>& OtherSet)
		{
			Reserve(m_Elements.Num() + OtherSet.Num());
			for (const ElementType& Element : OtherSet)
			{
				Add(Element);
			}
		}
		void Append(TSet<ElementType, KeyFuncs>&& OtherSet)
		{
			Reserve(m_Elements.Num() + OtherSet.Num());
			for (ElementType& Element : OtherSet)
			{
				Add(std::move(Element));
			}
			OtherSet.Reset();
		}
		void Append(std::initializer_list<ElementType> InitList)
		{
			Reserve(m_Elements.Num() + (int32)InitList.size());
			for (const ElementType& Element : InitList)
			{
				Add(Element);
			}
		}

		// remove
		void Remove(SetElementId ElementId)
		{
			if (m_Elements.Num())
			{
				const auto& ElementBeingRemoved = m_Elements[ElementId];

				// Remove the element from the hash bucket 
				for (SetElementId* NextElementId = &BucketId(ElementBeingRemoved.HashIndex);
					NextElementId->IsValidId();
					NextElementId = &m_Elements[*NextElementId].HashNextId)
				{
					// link the list 
					if (*NextElementId == ElementId)
					{
						*NextElementId = ElementBeingRemoved.HashNextId;
						break;
					}
				}

				// Remove the element from the elements array.
				m_Elements.RemoveAt(ElementId);
			}
		}
		int32 Remove(const KeyType& Key)
		{
			if (m_Elements.Num())
			{
				return RemoveImpl(KeyFuncs::GetKeyHash(Key), Key);
			}
			return 0;
		}
		template<typename ComparableKey>
		int32 RemoveByHash(uint32 KeyHash, const ComparableKey& Key)
		{
			check(KeyHash == KeyFuncs::GetKeyHash(Key));

			if (m_Elements.Num())
			{
				return RemoveImpl(KeyHash, Key);
			}

			return 0;
		}

		// find 
		SetElementId FindId(const KeyType& Key) const
		{
			if (m_Elements.Num())
			{
				for (SetElementId ElementId = BucketId(KeyFuncs::GetKeyHash(Key));
					ElementId.IsValidId();
					ElementId = m_Elements[ElementId].HashNextId)
				{
					if (KeyFuncs::Matches(KeyFuncs::GetSetKey(m_Elements[ElementId].Value), Key))
					{
						return ElementId;
					}
				}
			}
			return SetElementId();
		}
		template<typename ComparableKey>
		SetElementId FindIdByHash(uint32 KeyHash, const ComparableKey& Key) const
		{
			if (m_Elements.Num())
			{
				check(KeyHash == KeyFuncs::GetKeyHash(Key));

				for (SetElementId ElementId = BucketId(KeyHash);
					ElementId.IsValidId();
					ElementId = m_Elements[ElementId].HashNextId)
				{
					if (KeyFuncs::Matches(KeyFuncs::GetSetKey(m_Elements[ElementId].Value), Key))
					{
						return ElementId;
					}
				}
			}
			return SetElementId();
		}
		FORCEINLINE ElementType* Find(const KeyType& Key)
		{
			SetElementId ElementId = FindId(Key);
			if (ElementId.IsValidId())
			{
				return &m_Elements[ElementId].Value;
			}
			else
			{
				return nullptr;
			}
		}
		FORCEINLINE const ElementType* Find(const KeyType& Key) const
		{
			return const_cast<TSet*>(this)->Find(Key);
		}
		template<typename ComparableKey>
		ElementType* FindByHash(uint32 KeyHash, const ComparableKey& Key)
		{
			SetElementId ElementId = FindIdByHash(KeyHash, Key);
			if (ElementId.IsValidId())
			{
				return &m_Elements[ElementId].Value;
			}
			else
			{
				return nullptr;
			}
		}
		template<typename ComparableKey>
		const ElementType* FindByHash(uint32 KeyHash, const ComparableKey& Key) const
		{
			return const_cast<TSet*>(this)->FindByHash(KeyHash, Key);
		}

		// contains 
		FORCEINLINE bool Contains(const KeyType& Key) const { return FindId(Key).IsValidId(); }
		template<typename ComparableKey>
		FORCEINLINE bool ContainsByHash(uint32 KeyHash, const ComparableKey& Key) const { return FindIdByHash(KeyHash, Key).IsValidId(); }

		// sort 
		template <typename Predicate>
		void Sort(const Predicate& Pred)
		{
			m_Elements.Sort(FElementCompareClass<Predicate>(Pred));
			Rehash();
		}
		template <typename Predicate>
		void StableSort(const Predicate& Pred)
		{
			m_Elements.StableSort(FElementCompareClass<Predicate>(Pred));
			Rehash();
		}

		// iterate over all elements for the hash entry of the given key, and verify that the ids are valid
		bool VerifyHashElementsKey(const KeyType& Key)
		{
			bool bResult = true;
			if (m_Elements.Num())
			{
				SetElementId ElementId = BucketId(KeyFuncs::GetKeyHash(Key));
				while (ElementId.IsValidId())
				{
					if (!IsValidId(ElementId))
					{
						bResult = false;
						break;
					}
					ElementId = m_Elements[ElementId].HashNextId;
				}
			}
			return bResult;
		}

		// A AND B
		TSet Intersect(const TSet& OtherSet) const
		{
			const bool bOtherSmaller = (Num() > OtherSet.Num());
			const TSet& A = (bOtherSmaller ? OtherSet : *this);
			const TSet& B = (bOtherSmaller ? *this : OtherSet);

			TSet Result;
			Result.Reserve(A.Num()); // Worst case

			for (TConstIterator SetIt(A); SetIt; ++SetIt)
			{
				if (B.Contains(KeyFuncs::GetSetKey(*SetIt)))
				{
					Result.Add(*SetIt);
				}
			}
			return Result;
		}

		// A OR B
		TSet Union(const TSet& OtherSet) const
		{
			TSet Result;
			Result.Reserve(Num() + OtherSet.Num()); // Worst case

			for (TConstIterator SetIt(*this); SetIt; ++SetIt)
			{
				Result.Add(*SetIt);
			}
			for (TConstIterator SetIt(OtherSet); SetIt; ++SetIt)
			{
				Result.Add(*SetIt);
			}
			return Result;
		}

		// return the element not in OtherSet
		TSet Difference(const TSet& OtherSet) const
		{
			TSet Result;
			Result.Reserve(Num()); // Worst case is no elements of this are in Other

			for (TConstIterator SetIt(*this); SetIt; ++SetIt)
			{
				if (!OtherSet.Contains(KeyFuncs::GetSetKey(*SetIt)))
				{
					Result.Add(*SetIt);
				}
			}
			return Result;
		}

		// Determine whether the specified set is entirely included within this set
		bool Includes(const TSet& OtherSet) const
		{
			bool bIncludesSet = true;
			if (OtherSet.Num() <= Num())
			{
				for (TConstIterator OtherSetIt(OtherSet); OtherSetIt; ++OtherSetIt)
				{
					if (!Contains(KeyFuncs::GetSetKey(*OtherSetIt)))
					{
						bIncludesSet = false;
						break;
					}
				}
			}
			else
			{
				// Not possible to include if it is bigger than us
				bIncludesSet = false;
			}
			return bIncludesSet;
		}

		// To Array 
		TArray<ElementType> Array() const
		{
			TArray<ElementType> Result;
			Result.Reserve(Num());
			for (TConstIterator SetIt(*this); SetIt; ++SetIt)
			{
				Result.Add(*SetIt);
			}
			return Result;
		}

		// Debug check 
		FORCEINLINE void CheckAddress(const ElementType* Addr) const
		{
			m_Elements.CheckAddress(Addr);
		}

	private:
		//-----------------------------------------------iterators-----------------------------------------------
		template<bool bConst, bool bRangedFor = false>
		class TBaseIterator
		{
		public:
			typedef typename std::conditional_t<bConst, const ElementType, ElementType> ItElementType;
			typedef std::conditional_t<
				bConst,
				typename std::conditional_t<bRangedFor, typename ElementArrayType::TRangedForConstIterator, typename ElementArrayType::TConstIterator>,
				typename std::conditional_t<bRangedFor, typename ElementArrayType::TRangedForIterator, typename ElementArrayType::TIterator     >
			> ElementItType;

			FORCEINLINE TBaseIterator(const ElementItType& InElementIt) : ElementIt(InElementIt) {}

			/** Advances the iterator to the next element. */
			FORCEINLINE TBaseIterator& operator++()
			{
				++ElementIt;
				return *this;
			}

			/** conversion to "bool" returning true if the iterator is valid. */
			FORCEINLINE explicit operator bool() const { return !!ElementIt; }
			FORCEINLINE bool operator !() const { return !(bool)*this; }

			// Accessors.
			FORCEINLINE SetElementId GetId() const { return SetElementId(ElementIt.GetIndex()); }
			FORCEINLINE ItElementType* operator->() const { return &ElementIt->Value; }
			FORCEINLINE ItElementType& operator*() const { return ElementIt->Value; }

			FORCEINLINE friend bool operator==(const TBaseIterator& Lhs, const TBaseIterator& Rhs) { return Lhs.ElementIt == Rhs.ElementIt; }
			FORCEINLINE friend bool operator!=(const TBaseIterator& Lhs, const TBaseIterator& Rhs) { return Lhs.ElementIt != Rhs.ElementIt; }

			ElementItType ElementIt;
		};
		template<bool bConst>
		class TBaseKeyIterator
		{
		private:
			typedef typename std::conditional_t<bConst, const TSet, TSet> SetType;
			typedef typename std::conditional_t<bConst, const ElementType, ElementType> ItElementType;

		public:
			/** Initialization constructor. */
			FORCEINLINE TBaseKeyIterator(SetType& InSet, const KeyType& InKey)
				: Set(InSet)
				, Key(InKey)
				, Id()
			{
				Set.ConditionalRehash(Set.m_Elements.Num());
				if (Set.m_HashSize)
				{
					NextId = Set.BucketId(KeyFuncs::GetKeyHash(Key));
					++(*this);
				}
			}

			/** Advances the iterator to the next element. */
			FORCEINLINE TBaseKeyIterator& operator++()
			{
				Id = NextId;

				while (Id.IsValidId())
				{
					NextId = Set.m_Elements[Id].HashNextId;
					check(Id != NextId);

					if (KeyFuncs::Matches(KeyFuncs::GetSetKey(Set[Id]), Key))
					{
						break;
					}

					Id = NextId;
				}
				return *this;
			}

			FORCEINLINE explicit operator bool() const { return Id.IsValidId(); }
			FORCEINLINE bool operator !() const { return !(bool)*this; }

			// Accessors.
			FORCEINLINE ItElementType* operator->() const { return &Set[Id]; }
			FORCEINLINE ItElementType& operator*() const { return Set[Id]; }

		protected:
			SetType&		Set;
			const KeyType&	Key;
			SetElementId	Id;
			SetElementId	NextId;
		};
	public:
		class TIterator : public TBaseIterator<false>
		{
			friend class TSet;

		public:
			FORCEINLINE TIterator(TSet& InSet): TBaseIterator<false>(InSet.m_Elements.begin()), Set(InSet) {}

			/** Removes the current element from the set. */
			FORCEINLINE void RemoveCurrent()
			{
				Set.Remove(TBaseIterator<false>::GetId());
			}

		private:
			TSet& Set;
		};
		class TConstIterator : public TBaseIterator<true>
		{
			friend class TSet;
		public:
			FORCEINLINE TConstIterator(const TSet& InSet): TBaseIterator<true>(InSet.m_Elements.begin()) {}
		};

		using TRangedForIterator = TBaseIterator<false, true>;
		using TRangedForConstIterator = TBaseIterator<true, true>;

		class TKeyIterator : public TBaseKeyIterator<false>
		{
		public:
			FORCEINLINE TKeyIterator(TSet& InSet, const KeyType& InKey) : TBaseKeyIterator<false>(InSet, InKey) , Set(InSet) {}
			FORCEINLINE void RemoveCurrent()
			{
				Set.Remove(TBaseKeyIterator<false>::Id);
				TBaseKeyIterator<false>::Id = SetElementId();
			}
		private:
			TSet& Set;
		};
		class TConstKeyIterator : public TBaseKeyIterator<true>
		{
		public:
			FORCEINLINE TConstKeyIterator(const TSet& InSet, const KeyType& InKey) : TBaseKeyIterator<true>(InSet, InKey) {}
		};

		FORCEINLINE TIterator CreateIterator() { return TIterator(*this); }
		FORCEINLINE TConstIterator CreateConstIterator() const { return TConstIterator(*this); }

	public:
		// Support foreach  
		FORCEINLINE TRangedForIterator      begin() { return TRangedForIterator(m_Elements.begin()); }
		FORCEINLINE TRangedForConstIterator begin() const { return TRangedForConstIterator(m_Elements.begin()); }
		FORCEINLINE TRangedForIterator      end() { return TRangedForIterator(m_Elements.end()); }
		FORCEINLINE TRangedForConstIterator end() const { return TRangedForConstIterator(m_Elements.end()); }
	};
}
