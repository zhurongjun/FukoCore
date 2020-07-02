#pragma once
#include "CoreConfig.h"
#include "CoreType.h"
#include "Templates/TypeTraits.h"
#include "CoreMinimal/Assert.h"

// std::forward 
namespace Fuko
{
	class FDefaultSetAllocator;
	template<typename ElementType, bool bInAllowDuplicateKeys = false>
	struct DefaultKeyFuncs;

	template<
		typename InElementType,
		typename KeyFuncs = DefaultKeyFuncs<InElementType>,
		typename Allocator = FDefaultSetAllocator>
		class TSet;
}

// Key functions 
namespace Fuko
{
	template<typename ElementType, typename InKeyType, bool bInAllowDuplicateKeys = false>
	struct BaseKeyFuncs
	{
		typedef InKeyType KeyType;
		typedef typename TCallTraits<InKeyType>::ParamType KeyInitType;
		typedef typename TCallTraits<ElementType>::ParamType ElementInitType;

		enum { bAllowDuplicateKeys = bInAllowDuplicateKeys };
	};

	template<typename ElementType, bool bInAllowDuplicateKeys>
	struct DefaultKeyFuncs : BaseKeyFuncs<ElementType, ElementType, bInAllowDuplicateKeys>
	{
		typedef typename TCallTraits<ElementType>::ParamType KeyInitType;
		typedef typename TCallTraits<ElementType>::ParamType ElementInitType;

		/**
		 * @return The key used to index the given element.
		 */
		static FORCEINLINE KeyInitType GetSetKey(ElementInitType Element)
		{
			return Element;
		}

		/**
		 * @return True if the keys match.
		 */
		static FORCEINLINE bool Matches(KeyInitType A, KeyInitType B)
		{
			return A == B;
		}

		/**
		 * @return True if the keys match.
		 */
		template<typename ComparableKey>
		static FORCEINLINE bool Matches(KeyInitType A, ComparableKey B)
		{
			return A == B;
		}

		/** Calculates a hash index for a key. */
		static FORCEINLINE uint32 GetKeyHash(KeyInitType Key)
		{
			return GetTypeHash(Key);
		}

		/** Calculates a hash index for a key. */
		template<typename ComparableKey>
		static FORCEINLINE uint32 GetKeyHash(ComparableKey Key)
		{
			return GetTypeHash(Key);
		}
	};
}

// Set Element
namespace Fuko
{
	class FSetElementId
	{
	public:
		template<typename, typename, typename>
		friend class TSet;

		FORCEINLINE FSetElementId() :
			Index(INDEX_NONE)
		{}

		FORCEINLINE bool IsValidId() const
		{
			return Index != INDEX_NONE;
		}

		FORCEINLINE friend bool operator==(const FSetElementId& A, const FSetElementId& B)
		{
			return A.Index == B.Index;
		}

		FORCEINLINE int32 AsInteger() const
		{
			return Index;
		}

		FORCEINLINE static FSetElementId FromInteger(int32 Integer)
		{
			return FSetElementId(Integer);
		}

	private:
		FORCEINLINE static void ResetRange(FSetElementId* Range, int32 Count)
		{
			for (int32 I = 0; I < Count; ++I)
			{
				Range[I] = FSetElementId();
			}
		}
		int32 Index;

		FORCEINLINE FSetElementId(int32 InIndex) :
			Index(InIndex)
		{}

		FORCEINLINE operator int32() const
		{
			return Index;
		}
	};

	template<typename InElementType>
	class TSetElementBase
	{
	public:
		typedef InElementType ElementType;

		FORCEINLINE TSetElementBase() {}
		template <typename InitType>
		FORCEINLINE explicit TSetElementBase(InitType&& InValue) : Value(std::forward<InitType>(InValue)) {}

		TSetElementBase(TSetElementBase&&) = default;
		TSetElementBase(const TSetElementBase&) = default;
		TSetElementBase& operator=(TSetElementBase&&) = default;
		TSetElementBase& operator=(const TSetElementBase&) = default;

		ElementType Value;

		mutable int32 HashIndex;
		mutable FSetElementId HashNextId;
	};

	template <typename InElementType>
	class TSetElement : public TSetElementBase<InElementType>
	{
		using Super = TSetElementBase<InElementType>;
	public:
		FORCEINLINE TSetElement() = default;
		template <typename InitType>
		explicit FORCEINLINE TSetElement(InitType&& InValue) : Super(std::forward<InitType>(InValue)) {}

		TSetElement(TSetElement&&) = default;
		TSetElement(const TSetElement&) = default;
		TSetElement& operator=(TSetElement&&) = default;
		TSetElement& operator=(const TSetElement&) = default;

		FORCEINLINE bool operator==(const TSetElement& Other) const
		{
			return this->Value == Other.Value;
		}
		FORCEINLINE bool operator!=(const TSetElement& Other) const
		{
			return this->Value != Other.Value;
		}
	};
}

// TSet
namespace Fuko
{
	template<typename InElementType, typename KeyFuncs, typename Allocator>
	class TSet
	{
	private:
		typedef typename KeyFuncs::KeyInitType     KeyInitType;
		typedef typename KeyFuncs::ElementInitType ElementInitType;

		typedef TSetElement<InElementType> SetElementType;

	public:
		typedef InElementType ElementType;

		FORCEINLINE TSet() : HashSize(0) {}
		FORCEINLINE TSet(const TSet& Copy) : HashSize(0) { *this = Copy; }
		FORCEINLINE explicit TSet(const TArray<ElementType>& InArray) : HashSize(0) { Append(InArray); }
		FORCEINLINE explicit TSet(TArray<ElementType>&& InArray) : HashSize(0) { Append(std::move(InArray)); }

		~TSet() = default;

		TSet& operator=(const TSet& Copy)
		{
			if (this != &Copy)
			{
				int32 CopyHashSize = Copy.HashSize;

				DestructItems((FSetElementId*)Hash.GetAllocation(), HashSize);
				Hash.ResizeAllocation(0, CopyHashSize, sizeof(FSetElementId));
				ConstructItems<FSetElementId>(Hash.GetAllocation(), (FSetElementId*)Copy.Hash.GetAllocation(), CopyHashSize);
				HashSize = CopyHashSize;

				Elements = Copy.Elements;
			}
			return *this;
		}

	private:
		template <typename SetType>
		static FORCEINLINE void MoveOrCopy(SetType& ToSet, SetType& FromSet)
		{
			ToSet.Elements = (ElementArrayType&&)FromSet.Elements;

			ToSet.Hash.MoveToEmpty(FromSet.Hash);

			ToSet.HashSize = FromSet.HashSize;
			FromSet.HashSize = 0;
		}
	public:
		/** Initializer list constructor. */
		TSet(std::initializer_list<ElementType> InitList) : HashSize(0) { Append(InitList); }

		/** Move constructor. */
		TSet(TSet&& Other) : HashSize(0) { MoveOrCopy(*this, Other); }

		/** Move assignment operator. */
		TSet& operator=(TSet&& Other)
		{
			if (this != &Other)
			{
				MoveOrCopy(*this, Other);
			}

			return *this;
		}

		template<typename OtherAllocator>
		TSet(TSet<ElementType, KeyFuncs, OtherAllocator>&& Other) : HashSize(0) { Append(std::move(Other)); }
		template<typename OtherAllocator>
		TSet(const TSet<ElementType, KeyFuncs, OtherAllocator>& Other) : HashSize(0) { Append(Other); }

		
		template<typename OtherAllocator>
		TSet& operator=(TSet<ElementType, KeyFuncs, OtherAllocator>&& Other)
		{
			Reset();
			Append(std::move(Other));
			return *this;
		}
		template<typename OtherAllocator>
		TSet& operator=(const TSet<ElementType, KeyFuncs, OtherAllocator>& Other)
		{
			Reset();
			Append(Other);
			return *this;
		}
		TSet& operator=(std::initializer_list<ElementType> InitList)
		{
			Reset();
			Append(InitList);
			return *this;
		}

		/**
		 * Removes all elements from the set, potentially leaving space allocated for an expected number of elements about to be added.
		 * @param ExpectedNumElements - The number of elements about to be added to the set.
		 */
		void Empty(int32 ExpectedNumElements = 0)
		{
			// Empty the elements array, and reallocate it for the expected number of elements.
			Elements.Empty(ExpectedNumElements);

			// Resize the hash to the desired size for the expected number of elements.
			if (!ConditionalRehash(ExpectedNumElements, true))
			{
				// If the hash was already the desired size, clear the references to the elements that have now been removed.
				for (int32 HashIndex = 0, LocalHashSize = HashSize; HashIndex < LocalHashSize; ++HashIndex)
				{
					GetTypedHash(HashIndex) = FSetElementId();
				}
			}
		}

		/** Efficiently empties out the set but preserves all allocations and capacities */
		void Reset()
		{
			if (Num() == 0)
			{
				return;
			}

			// Reset the elements array.
			Elements.Reset();

			// Clear the references to the elements that have now been removed.
			FSetElementId::ResetRange(Hash.GetAllocation(), HashSize);
		}

		/** Shrinks the set's element storage to avoid slack. */
		FORCEINLINE void Shrink()
		{
			Elements.Shrink();
			Relax();
		}

		/** Compacts the allocated elements into a contiguous range. */
		FORCEINLINE void Compact()
		{
			if (Elements.Compact())
			{
				Rehash();
			}
		}

		/** Compacts the allocated elements into a contiguous range. Does not change the iteration order of the elements. */
		FORCEINLINE void CompactStable()
		{
			if (Elements.CompactStable())
			{
				Rehash();
			}
		}

		/** Preallocates enough memory to contain Number elements */
		FORCEINLINE void Reserve(int32 Number)
		{
			// makes sense only when Number > Elements.Num() since TSparseArray::Reserve 
			// does any work only if that's the case
			if (Number > Elements.Num())
			{
				// Preallocates memory for array of elements
				Elements.Reserve(Number);

				// Calculate the corresponding hash size for the specified number of elements.
				const int32 NewHashSize = Allocator::GetNumberOfHashBuckets(Number);

				// If the hash hasn't been created yet, or is smaller than the corresponding hash size, rehash
				// to force a preallocation of the hash table
				if (!HashSize || HashSize < NewHashSize)
				{
					HashSize = NewHashSize;
					Rehash();
				}
			}
		}

		/** Relaxes the set's hash to a size strictly bounded by the number of elements in the set. */
		FORCEINLINE void Relax()
		{
			ConditionalRehash(Elements.Num(), true);
		}

		/**
		 * Helper function to return the amount of memory allocated by this container
		 * Only returns the size of allocations made directly by the container, not the elements themselves.
		 * @return number of bytes allocated by this container
		 */
		FORCEINLINE uint32 GetAllocatedSize(void) const
		{
			return Elements.GetAllocatedSize() + (HashSize * sizeof(FSetElementId));
		}

		/** @return the number of elements. */
		FORCEINLINE int32 Num() const
		{
			return Elements.Num();
		}

		FORCEINLINE int32 GetMaxIndex() const
		{
			return Elements.GetMaxIndex();
		}

		/**
		 * Checks whether an element id is valid.
		 * @param Id - The element id to check.
		 * @return true if the element identifier refers to a valid element in this set.
		 */
		FORCEINLINE bool IsValidId(FSetElementId Id) const
		{
			return	Id.IsValidId() &&
				Id >= 0 &&
				Id < Elements.GetMaxIndex() &&
				Elements.IsAllocated(Id);
		}

		/** Accesses the identified element's value. */
		FORCEINLINE ElementType& operator[](FSetElementId Id)
		{
			return Elements[Id].Value;
		}

		/** Accesses the identified element's value. */
		FORCEINLINE const ElementType& operator[](FSetElementId Id) const
		{
			return Elements[Id].Value;
		}

		/**
		 * Adds an element to the set.
		 *
		 * @param	InElement					Element to add to set
		 * @param	bIsAlreadyInSetPtr	[out]	Optional pointer to bool that will be set depending on whether element is already in set
		 * @return	A pointer to the element stored in the set.
		 */
		FORCEINLINE FSetElementId Add(const InElementType&  InElement, bool* bIsAlreadyInSetPtr = nullptr) { return Emplace(InElement, bIsAlreadyInSetPtr); }
		FORCEINLINE FSetElementId Add(InElementType&& InElement, bool* bIsAlreadyInSetPtr = nullptr) { return Emplace(std::moveIfPossible(InElement), bIsAlreadyInSetPtr); }

		/**
		 * Adds an element to the set.
		 *
		 * @see		Class documentation section on ByHash() functions
		 * @param	InElement					Element to add to set
		 * @param	bIsAlreadyInSetPtr	[out]	Optional pointer to bool that will be set depending on whether element is already in set
		 * @return  A handle to the element stored in the set
		 */
		FORCEINLINE FSetElementId AddByHash(uint32 KeyHash, const InElementType& InElement, bool* bIsAlreadyInSetPtr = nullptr)
		{
			return EmplaceByHash(KeyHash, InElement, bIsAlreadyInSetPtr);
		}
		FORCEINLINE FSetElementId AddByHash(uint32 KeyHash, InElementType&& InElement, bool* bIsAlreadyInSetPtr = nullptr)
		{
			return EmplaceByHash(KeyHash, std::moveIfPossible(InElement), bIsAlreadyInSetPtr);
		}

	private:
		FSetElementId EmplaceImpl(uint32 KeyHash, SetElementType& Element, FSetElementId ElementId, bool* bIsAlreadyInSetPtr)
		{
			bool bIsAlreadyInSet = false;
			if (!KeyFuncs::bAllowDuplicateKeys)
			{
				// If the set doesn't allow duplicate keys, check for an existing element with the same key as the element being added.

				// Don't bother searching for a duplicate if this is the first element we're adding
				if (Elements.Num() != 1)
				{
					FSetElementId ExistingId = FindIdByHash(KeyHash, KeyFuncs::GetSetKey(Element.Value));
					bIsAlreadyInSet = ExistingId.IsValidId();
					if (bIsAlreadyInSet)
					{
						RelocateConstructItems<InElementType>(&Elements[ExistingId].Value, &Element.Value, 1);

						// Then remove the new element.
						Elements.RemoveAtUninitialized(ElementId);

						// Then point the return value at the replaced element.
						ElementId = ExistingId;
					}
				}
			}

			if (!bIsAlreadyInSet)
			{
				// Check if the hash needs to be resized.
				if (!ConditionalRehash(Elements.Num()))
				{
					// If the rehash didn't add the new element to the hash, add it.
					LinkElement(ElementId, Element, KeyHash);
				}
			}

			if (bIsAlreadyInSetPtr)
			{
				*bIsAlreadyInSetPtr = bIsAlreadyInSet;
			}

			return ElementId;
		}

	public:
		/**
		 * Adds an element to the set.
		 *
		 * @param	Args						The argument(s) to be std::forwarded to the set element's constructor.
		 * @param	bIsAlreadyInSetPtr	[out]	Optional pointer to bool that will be set depending on whether element is already in set
		 * @return	A handle to the element stored in the set.
		 */
		template <typename ArgsType>
		FSetElementId Emplace(ArgsType&& Args, bool* bIsAlreadyInSetPtr = nullptr)
		{
			// Create a new element.
			FSparseArrayAllocationInfo ElementAllocation = Elements.AddUninitialized();
			SetElementType& Element = *new (ElementAllocation) SetElementType(std::forward<ArgsType>(Args));

			uint32 KeyHash = KeyFuncs::GetKeyHash(KeyFuncs::GetSetKey(Element.Value));
			return EmplaceImpl(KeyHash, Element, ElementAllocation.Index, bIsAlreadyInSetPtr);
		}

		/**
		 * Adds an element to the set.
		 *
		 * @see		Class documentation section on ByHash() functions
		 * @param	Args						The argument(s) to be std::forwarded to the set element's constructor.
		 * @param	bIsAlreadyInSetPtr	[out]	Optional pointer to bool that will be set depending on whether element is already in set
		 * @return	A handle to the element stored in the set.
		 */
		template <typename ArgsType>
		FSetElementId EmplaceByHash(uint32 KeyHash, ArgsType&& Args, bool* bIsAlreadyInSetPtr = nullptr)
		{
			// Create a new element.
			FSparseArrayAllocationInfo ElementAllocation = Elements.AddUninitialized();
			SetElementType& Element = *new (ElementAllocation) SetElementType(std::forward<ArgsType>(Args));

			return EmplaceImpl(KeyHash, Element, ElementAllocation.Index, bIsAlreadyInSetPtr);
		}

		template<typename ArrayAllocator>
		void Append(const TArray<ElementType, ArrayAllocator>& InElements)
		{
			Reserve(Elements.Num() + InElements.Num());
			for (const ElementType& Element : InElements)
			{
				Add(Element);
			}
		}

		template<typename ArrayAllocator>
		void Append(TArray<ElementType, ArrayAllocator>&& InElements)
		{
			Reserve(Elements.Num() + InElements.Num());
			for (ElementType& Element : InElements)
			{
				Add(std::move(Element));
			}
			InElements.Reset();
		}

		/**
		 * Add all items from another set to our set (union without creating a new set)
		 * @param OtherSet - The other set of items to add.
		 */
		template<typename OtherAllocator>
		void Append(const TSet<ElementType, KeyFuncs, OtherAllocator>& OtherSet)
		{
			Reserve(Elements.Num() + OtherSet.Num());
			for (const ElementType& Element : OtherSet)
			{
				Add(Element);
			}
		}

		template<typename OtherAllocator>
		void Append(TSet<ElementType, KeyFuncs, OtherAllocator>&& OtherSet)
		{
			Reserve(Elements.Num() + OtherSet.Num());
			for (ElementType& Element : OtherSet)
			{
				Add(std::move(Element));
			}
			OtherSet.Reset();
		}

		void Append(std::initializer_list<ElementType> InitList)
		{
			Reserve(Elements.Num() + (int32)InitList.size());
			for (const ElementType& Element : InitList)
			{
				Add(Element);
			}
		}

		/**
		 * Removes an element from the set.
		 * @param Element - A pointer to the element in the set, as returned by Add or Find.
		 */
		void Remove(FSetElementId ElementId)
		{
			if (Elements.Num())
			{
				const auto& ElementBeingRemoved = Elements[ElementId];

				// Remove the element from the hash.
				for (FSetElementId* NextElementId = &GetTypedHash(ElementBeingRemoved.HashIndex);
					NextElementId->IsValidId();
					NextElementId = &Elements[*NextElementId].HashNextId)
				{
					if (*NextElementId == ElementId)
					{
						*NextElementId = ElementBeingRemoved.HashNextId;
						break;
					}
				}
			}

			// Remove the element from the elements array.
			Elements.RemoveAt(ElementId);
		}

		/**
		 * Finds an element with the given key in the set.
		 * @param Key - The key to search for.
		 * @return The id of the set element matching the given key, or the NULL id if none matches.
		 */
		FSetElementId FindId(KeyInitType Key) const
		{
			if (Elements.Num())
			{
				for (FSetElementId ElementId = GetTypedHash(KeyFuncs::GetKeyHash(Key));
					ElementId.IsValidId();
					ElementId = Elements[ElementId].HashNextId)
				{
					if (KeyFuncs::Matches(KeyFuncs::GetSetKey(Elements[ElementId].Value), Key))
					{
						// Return the first match, regardless of whether the set has multiple matches for the key or not.
						return ElementId;
					}
				}
			}
			return FSetElementId();
		}

		/**
		 * Finds an element with a pre-calculated hash and a key that can be compared to KeyType
		 * @see	Class documentation section on ByHash() functions
		 * @return The element id that matches the key and hash or an invalid element id
		 */
		template<typename ComparableKey>
		FSetElementId FindIdByHash(uint32 KeyHash, const ComparableKey& Key) const
		{
			if (Elements.Num())
			{
				check(KeyHash == KeyFuncs::GetKeyHash(Key));

				for (FSetElementId ElementId = GetTypedHash(KeyHash);
					ElementId.IsValidId();
					ElementId = Elements[ElementId].HashNextId)
				{
					if (KeyFuncs::Matches(KeyFuncs::GetSetKey(Elements[ElementId].Value), Key))
					{
						// Return the first match, regardless of whether the set has multiple matches for the key or not.
						return ElementId;
					}
				}
			}
			return FSetElementId();
		}

		/**
		 * Finds an element with the given key in the set.
		 * @param Key - The key to search for.
		 * @return A pointer to an element with the given key.  If no element in the set has the given key, this will return NULL.
		 */
		FORCEINLINE ElementType* Find(KeyInitType Key)
		{
			FSetElementId ElementId = FindId(Key);
			if (ElementId.IsValidId())
			{
				return &Elements[ElementId].Value;
			}
			else
			{
				return nullptr;
			}
		}

		/**
		 * Finds an element with the given key in the set.
		 * @param Key - The key to search for.
		 * @return A const pointer to an element with the given key.  If no element in the set has the given key, this will return NULL.
		 */
		FORCEINLINE const ElementType* Find(KeyInitType Key) const
		{
			return const_cast<TSet*>(this)->Find(Key);
		}

		/**
		 * Finds an element with a pre-calculated hash and a key that can be compared to KeyType.
		 * @see	Class documentation section on ByHash() functions
		 * @return A pointer to the contained element or nullptr.
		 */
		template<typename ComparableKey>
		ElementType* FindByHash(uint32 KeyHash, const ComparableKey& Key)
		{
			FSetElementId ElementId = FindIdByHash(KeyHash, Key);
			if (ElementId.IsValidId())
			{
				return &Elements[ElementId].Value;
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

	private:
		template<typename ComparableKey>
		FORCEINLINE int32 RemoveImpl(uint32 KeyHash, const ComparableKey& Key)
		{
			int32 NumRemovedElements = 0;

			FSetElementId* NextElementId = &GetTypedHash(KeyHash);
			while (NextElementId->IsValidId())
			{
				auto& Element = Elements[*NextElementId];
				if (KeyFuncs::Matches(KeyFuncs::GetSetKey(Element.Value), Key))
				{
					// This element matches the key, remove it from the set.  Note that Remove sets *NextElementId to point to the next
					// element after the removed element in the hash bucket.
					Remove(*NextElementId);
					NumRemovedElements++;

					if (!KeyFuncs::bAllowDuplicateKeys)
					{
						// If the hash disallows duplicate keys, we're done removing after the first matched key.
						break;
					}
				}
				else
				{
					NextElementId = &Element.HashNextId;
				}
			}

			return NumRemovedElements;
		}

	public:
		/**
		 * Removes all elements from the set matching the specified key.
		 * @param Key - The key to match elements against.
		 * @return The number of elements removed.
		 */
		int32 Remove(KeyInitType Key)
		{
			if (Elements.Num())
			{
				return RemoveImpl(KeyFuncs::GetKeyHash(Key), Key);
			}

			return 0;
		}

		/**
		 * Removes all elements from the set matching the specified key.
		 *
		 * @see		Class documentation section on ByHash() functions
		 * @param	Key - The key to match elements against.
		 * @return	The number of elements removed.
		 */
		template<typename ComparableKey>
		int32 RemoveByHash(uint32 KeyHash, const ComparableKey& Key)
		{
			check(KeyHash == KeyFuncs::GetKeyHash(Key));

			if (Elements.Num())
			{
				return RemoveImpl(KeyHash, Key);
			}

			return 0;
		}

		/**
		 * Checks if the element contains an element with the given key.
		 * @param Key - The key to check for.
		 * @return true if the set contains an element with the given key.
		 */
		FORCEINLINE bool Contains(KeyInitType Key) const
		{
			return FindId(Key).IsValidId();
		}

		/**
		 * Checks if the element contains an element with the given key.
		 *
		 * @see	Class documentation section on ByHash() functions
		 */
		template<typename ComparableKey>
		FORCEINLINE bool ContainsByHash(uint32 KeyHash, const ComparableKey& Key) const
		{
			return FindIdByHash(KeyHash, Key).IsValidId();
		}

		/**
		 * Sorts the set's elements using the provided comparison class.
		 */
		template <typename PREDICATE_CLASS>
		void Sort(const PREDICATE_CLASS& Predicate)
		{
			// Sort the elements according to the provided comparison class.
			Elements.Sort(FElementCompareClass< PREDICATE_CLASS >(Predicate));

			// Rehash.
			Rehash();
		}

		/**
		 * Stable sorts the set's elements using the provided comparison class.
		 */
		template <typename PREDICATE_CLASS>
		void StableSort(const PREDICATE_CLASS& Predicate)
		{
			// Sort the elements according to the provided comparison class.
			Elements.StableSort(FElementCompareClass< PREDICATE_CLASS >(Predicate));

			// Rehash.
			Rehash();
		}

		bool VerifyHashElementsKey(KeyInitType Key)
		{
			bool bResult = true;
			if (Elements.Num())
			{
				// iterate over all elements for the hash entry of the given key 
				// and verify that the ids are valid
				FSetElementId ElementId = GetTypedHash(KeyFuncs::GetKeyHash(Key));
				while (ElementId.IsValidId())
				{
					if (!IsValidId(ElementId))
					{
						bResult = false;
						break;
					}
					ElementId = Elements[ElementId].HashNextId;
				}
			}
			return bResult;
		}

		// Legacy comparison operators.  Note that these also test whether the set's elements were added in the same order!
		friend bool LegacyCompareEqual(const TSet& A, const TSet& B)
		{
			return A.Elements == B.Elements;
		}
		friend bool LegacyCompareNotEqual(const TSet& A, const TSet& B)
		{
			return A.Elements != B.Elements;
		}

		/** @return the intersection of two sets. (A AND B)*/
		TSet Intersect(const TSet& OtherSet) const
		{
			const bool bOtherSmaller = (Num() > OtherSet.Num());
			const TSet& A = (bOtherSmaller ? OtherSet : *this);
			const TSet& B = (bOtherSmaller ? *this : OtherSet);

			TSet Result;
			Result.Reserve(A.Num()); // Worst case is everything in smaller is in larger

			for (TConstIterator SetIt(A); SetIt; ++SetIt)
			{
				if (B.Contains(KeyFuncs::GetSetKey(*SetIt)))
				{
					Result.Add(*SetIt);
				}
			}
			return Result;
		}

		/** @return the union of two sets. (A OR B)*/
		TSet Union(const TSet& OtherSet) const
		{
			TSet Result;
			Result.Reserve(Num() + OtherSet.Num()); // Worst case is 2 totally unique Sets

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

		/** @return the complement of two sets. (A not in B where A is this and B is Other)*/
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

		/**
		 * Determine whether the specified set is entirely included within this set
		 *
		 * @param OtherSet	Set to check
		 *
		 * @return True if the other set is entirely included in this set, false if it is not
		 */
		bool Includes(const TSet<ElementType, KeyFuncs, Allocator>& OtherSet) const
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

		/** @return a TArray of the elements */
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

		/**
		 * Checks that the specified address is not part of an element within the container.  Used for implementations
		 * to check that reference arguments aren't going to be invalidated by possible reallocation.
		 *
		 * @param Addr The address to check.
		 */
		FORCEINLINE void CheckAddress(const ElementType* Addr) const
		{
			Elements.CheckAddress(Addr);
		}

	private:
		/** Extracts the element value from the set's element structure and passes it to the user provided comparison class. */
		template <typename PREDICATE_CLASS>
		class FElementCompareClass
		{
			TDereferenceWrapper< ElementType, PREDICATE_CLASS > Predicate;

		public:
			FORCEINLINE FElementCompareClass(const PREDICATE_CLASS& InPredicate)
				: Predicate(InPredicate)
			{}

			FORCEINLINE bool operator()(const SetElementType& A, const SetElementType& B) const
			{
				return Predicate(A.Value, B.Value);
			}
		};

		typedef TSparseArray<SetElementType, typename Allocator::SparseArrayAllocator>     ElementArrayType;
		typedef typename Allocator::HashAllocator::template ForElementType<FSetElementId> HashType;

		ElementArrayType Elements;

		mutable HashType Hash;
		mutable int32	 HashSize;
	private:

		FORCEINLINE FSetElementId& GetTypedHash(int32 HashIndex) const
		{
			return ((FSetElementId*)Hash.GetAllocation())[HashIndex & (HashSize - 1)];
		}

		/**
		 * Accesses an element in the set.
		 * This is needed because the iterator classes aren't friends of FSetElementId and so can't access the element index.
		 */
		FORCEINLINE const SetElementType& GetInternalElement(FSetElementId Id) const
		{
			return Elements[Id];
		}
		FORCEINLINE SetElementType& GetInternalElement(FSetElementId Id)
		{
			return Elements[Id];
		}

		/**
		 * Translates an element index into an element ID.
		 * This is needed because the iterator classes aren't friends of FSetElementId and so can't access the FSetElementId private constructor.
		 */
		static FORCEINLINE FSetElementId IndexToId(int32 Index)
		{
			return FSetElementId(Index);
		}

		/** Links an added element to the hash chain. */
		FORCEINLINE void LinkElement(FSetElementId ElementId, const SetElementType& Element, uint32 KeyHash) const
		{
			// Compute the hash bucket the element goes in.
			Element.HashIndex = KeyHash & (HashSize - 1);

			// Link the element into the hash bucket.
			Element.HashNextId = GetTypedHash(Element.HashIndex);
			GetTypedHash(Element.HashIndex) = ElementId;
		}

		/** Hashes and links an added element to the hash chain. */
		FORCEINLINE void HashElement(FSetElementId ElementId, const SetElementType& Element) const
		{
			LinkElement(ElementId, Element, KeyFuncs::GetKeyHash(KeyFuncs::GetSetKey(Element.Value)));
		}

		/**
		 * Checks if the hash has an appropriate number of buckets, and if not resizes it.
		 * @param NumHashedElements - The number of elements to size the hash for.
		 * @param bAllowShrinking - true if the hash is allowed to shrink.
		 * @return true if the set was rehashed.
		 */
		bool ConditionalRehash(int32 NumHashedElements, bool bAllowShrinking = false) const
		{
			// Calculate the desired hash size for the specified number of elements.
			const int32 DesiredHashSize = Allocator::GetNumberOfHashBuckets(NumHashedElements);

			// If the hash hasn't been created yet, or is smaller than the desired hash size, rehash.
			if (NumHashedElements > 0 &&
				(!HashSize ||
					HashSize < DesiredHashSize ||
					(HashSize > DesiredHashSize && bAllowShrinking)))
			{
				HashSize = DesiredHashSize;
				Rehash();
				return true;
			}
			else
			{
				return false;
			}
		}

		/** Resizes the hash. */
		void Rehash() const
		{
			// Free the old hash.
			Hash.ResizeAllocation(0, 0, sizeof(FSetElementId));

			int32 LocalHashSize = HashSize;
			if (LocalHashSize)
			{
				// Allocate the new hash.
				check(FMath::IsPowerOfTwo(HashSize));
				Hash.ResizeAllocation(0, LocalHashSize, sizeof(FSetElementId));
				for (int32 HashIndex = 0; HashIndex < LocalHashSize; ++HashIndex)
				{
					GetTypedHash(HashIndex) = FSetElementId();
				}

				// Add the existing elements to the new hash.
				for (typename ElementArrayType::TConstIterator ElementIt(Elements); ElementIt; ++ElementIt)
				{
					HashElement(FSetElementId(ElementIt.GetIndex()), *ElementIt);
				}
			}
		}

		/** The base type of whole set iterators. */
		template<bool bConst, bool bRangedFor = false>
		class TBaseIterator
		{
		private:
			friend class TSet;

			typedef typename std::conditional_t<bConst, const ElementType, ElementType> ItElementType;

		public:
			typedef typename std::conditional_t<
				bConst,
				typename std::conditional_t<bRangedFor, typename ElementArrayType::TRangedForConstIterator, typename ElementArrayType::TConstIterator>,
				typename std::conditional_t<bRangedFor, typename ElementArrayType::TRangedForIterator, typename ElementArrayType::TIterator     >
			> ElementItType;

			FORCEINLINE TBaseIterator(const ElementItType& InElementIt)
				: ElementIt(InElementIt)
			{
			}

			/** Advances the iterator to the next element. */
			FORCEINLINE TBaseIterator& operator++()
			{
				++ElementIt;
				return *this;
			}

			/** conversion to "bool" returning true if the iterator is valid. */
			FORCEINLINE explicit operator bool() const
			{
				return !!ElementIt;
			}
			/** inverse of the "bool" operator */
			FORCEINLINE bool operator !() const
			{
				return !(bool)*this;
			}

			// Accessors.
			FORCEINLINE FSetElementId GetId() const
			{
				return TSet::IndexToId(ElementIt.GetIndex());
			}
			FORCEINLINE ItElementType* operator->() const
			{
				return &ElementIt->Value;
			}
			FORCEINLINE ItElementType& operator*() const
			{
				return ElementIt->Value;
			}

			FORCEINLINE friend bool operator==(const TBaseIterator& Lhs, const TBaseIterator& Rhs) { return Lhs.ElementIt == Rhs.ElementIt; }
			FORCEINLINE friend bool operator!=(const TBaseIterator& Lhs, const TBaseIterator& Rhs) { return Lhs.ElementIt != Rhs.ElementIt; }

			ElementItType ElementIt;
		};

		/** The base type of whole set iterators. */
		template<bool bConst>
		class TBaseKeyIterator
		{
		private:
			typedef typename std::conditional_t<bConst, const TSet, TSet> SetType;
			typedef typename std::conditional_t<bConst, const ElementType, ElementType> ItElementType;

		public:
			/** Initialization constructor. */
			FORCEINLINE TBaseKeyIterator(SetType& InSet, KeyInitType InKey)
				: Set(InSet)
				, Key(InKey)
				, Id()
			{
				// The set's hash needs to be initialized to find the elements with the specified key.
				Set.ConditionalRehash(Set.Elements.Num());
				if (Set.HashSize)
				{
					NextId = Set.GetTypedHash(KeyFuncs::GetKeyHash(Key));
					++(*this);
				}
			}

			/** Advances the iterator to the next element. */
			FORCEINLINE TBaseKeyIterator& operator++()
			{
				Id = NextId;

				while (Id.IsValidId())
				{
					NextId = Set.GetInternalElement(Id).HashNextId;
					check(Id != NextId);

					if (KeyFuncs::Matches(KeyFuncs::GetSetKey(Set[Id]), Key))
					{
						break;
					}

					Id = NextId;
				}
				return *this;
			}

			/** conversion to "bool" returning true if the iterator is valid. */
			FORCEINLINE explicit operator bool() const
			{
				return Id.IsValidId();
			}
			/** inverse of the "bool" operator */
			FORCEINLINE bool operator !() const
			{
				return !(bool)*this;
			}

			// Accessors.
			FORCEINLINE ItElementType* operator->() const
			{
				return &Set[Id];
			}
			FORCEINLINE ItElementType& operator*() const
			{
				return Set[Id];
			}

		protected:
			SetType& Set;
			typename TTypeTraits<typename KeyFuncs::KeyType>::ConstPointerType Key;
			FSetElementId Id;
			FSetElementId NextId;
		};

	public:

		/** Used to iterate over the elements of a const TSet. */
		class TConstIterator : public TBaseIterator<true>
		{
			friend class TSet;

		public:
			FORCEINLINE TConstIterator(const TSet& InSet)
				: TBaseIterator<true>(InSet.Elements.begin())
			{
			}
		};

		/** Used to iterate over the elements of a TSet. */
		class TIterator : public TBaseIterator<false>
		{
			friend class TSet;

		public:
			FORCEINLINE TIterator(TSet& InSet)
				: TBaseIterator<false>(InSet.Elements.begin())
				, Set(InSet)
			{
			}

			/** Removes the current element from the set. */
			FORCEINLINE void RemoveCurrent()
			{
				Set.Remove(TBaseIterator<false>::GetId());
			}

		private:
			TSet& Set;
		};

		using TRangedForConstIterator = TBaseIterator<true, true>;
		using TRangedForIterator = TBaseIterator<false, true>;

		/** Used to iterate over the elements of a const TSet. */
		class TConstKeyIterator : public TBaseKeyIterator<true>
		{
		public:
			FORCEINLINE TConstKeyIterator(const TSet& InSet, KeyInitType InKey) :
				TBaseKeyIterator<true>(InSet, InKey)
			{}
		};

		/** Used to iterate over the elements of a TSet. */
		class TKeyIterator : public TBaseKeyIterator<false>
		{
		public:
			FORCEINLINE TKeyIterator(TSet& InSet, KeyInitType InKey)
				: TBaseKeyIterator<false>(InSet, InKey)
				, Set(InSet)
			{}

			/** Removes the current element from the set. */
			FORCEINLINE void RemoveCurrent()
			{
				Set.Remove(TBaseKeyIterator<false>::Id);
				TBaseKeyIterator<false>::Id = FSetElementId();
			}
		private:
			TSet& Set;
		};

		/** Creates an iterator for the contents of this set */
		FORCEINLINE TIterator CreateIterator()
		{
			return TIterator(*this);
		}

		/** Creates a const iterator for the contents of this set */
		FORCEINLINE TConstIterator CreateConstIterator() const
		{
			return TConstIterator(*this);
		}

	public:
		// Support foreach  
		FORCEINLINE TRangedForIterator      begin() { return TRangedForIterator(Elements.begin()); }
		FORCEINLINE TRangedForConstIterator begin() const { return TRangedForConstIterator(Elements.begin()); }
		FORCEINLINE TRangedForIterator      end() { return TRangedForIterator(Elements.end()); }
		FORCEINLINE TRangedForConstIterator end() const { return TRangedForConstIterator(Elements.end()); }
	};
}
