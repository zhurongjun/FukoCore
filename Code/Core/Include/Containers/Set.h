#pragma once
#include "CoreConfig.h"
#include "CoreType.h"
#include "Templates/TypeTraits.h"
#include "CoreMinimal/Assert.h"
#include "SparseArray.h"
#include "Memory/Memory.h"
#include "Memory/Allocators.h"
#include "../Templates/Functor.h"

// forward
namespace Fuko
{
	template<typename T, typename Alloc = PmrAllocator, typename KeyFuncs = DefaultKeyFuncs<T>>
	class TSet;
}

// Key functions 
namespace Fuko
{
	template<typename T, bool bInAllowDuplicateKeys = false>
	struct DefaultKeyFuncs
	{
		using KeyType = T;

		static constexpr bool bAllowDuplicateKeys = bInAllowDuplicateKeys;
		
		static FORCEINLINE const KeyType& Key(const T& Element) { return Element; }
		static FORCEINLINE bool Matches(const KeyType& A, const KeyType& B) { return A == B; }
		static FORCEINLINE uint32 Hash(KeyType Key) { return GetTypeHash(Key); }
	};
}

// TSet
namespace Fuko
{
	template<typename T, typename Alloc,typename KeyFuncs>
	class TSet
	{
	public:
		using KeyType = typename KeyFuncs::KeyType;
		using SizeType = typename Alloc::SizeType;
		
		class SetElementId
		{
			SizeType Index;
		public:
			FORCEINLINE SetElementId(SizeType InIndex = INDEX_NONE) : Index(InIndex) {}
			FORCEINLINE bool IsValid() const { return Index != INDEX_NONE; }
			FORCEINLINE operator SizeType() const { return Index; }
			FORCEINLINE friend bool operator==(const SetElementId& A, const SetElementId& B) { return A.Index == B.Index; }
			FORCEINLINE void Reset() { Index = INDEX_NONE; }
		};
		class SetElement
		{
		public:
			T						Value;
			mutable uint32			Hash;
			mutable SetElementId	HashNextId;
		public:
			FORCEINLINE SetElement() = default;
			FORCEINLINE SetElement(SetElement&&) = default;
			FORCEINLINE SetElement(const SetElement&) = default;
			FORCEINLINE SetElement& operator=(SetElement&&) = default;
			FORCEINLINE SetElement& operator=(const SetElement&) = default;

			template <typename...Ts>
			explicit FORCEINLINE SetElement(Ts&&...Args) : Value(std::forward<Ts>(Args)...), Hash(0), HashNextId() {}

			FORCEINLINE bool operator==(const SetElement& Other) const { return this->Value == Other.Value; }
			FORCEINLINE bool operator!=(const SetElement& Other) const { return this->Value != Other.Value; }
		};

		mutable SetElementId*			m_Hash;				// hash bucket 
		mutable SizeType				m_HashSize;			// hash bucket size 
		TSparseArray<SetElement, Alloc>	m_Elements;			// elements 
		//------------------------------Begin helper functions------------------------------
		static FORCEINLINE SizeType _GetNumberOfHashBuckets(SizeType NumHashedElements)
		{
			static constexpr SizeType MinNumberOfHashedElements = 4;
			static constexpr SizeType BaseNumberOfHashBuckets = 8;
			static constexpr SizeType AverageNumberOfElementsPerHashBucket = 2;

			if (NumHashedElements >= MinNumberOfHashedElements)
			{
				return FMath::RoundUpToPowerOfTwo(NumHashedElements / AverageNumberOfElementsPerHashBucket + BaseNumberOfHashBuckets);
			}

			return 1;
		}

		FORCEINLINE void _CleanBucket()
		{
			auto Ptr = m_Hash;
			auto End = m_Hash + m_HashSize;
			for (; Ptr != End; ++Ptr)
			{
				Ptr->Reset();
			}
		}

		// get index in hash bucket 
		FORCEINLINE SetElementId& _BucketId(uint32 HashIndex) const { return m_Hash[HashIndex & (m_HashSize - 1)]; }
		
		// rehash, before call it, m_HashSize Must be updated 
		void _Rehash() const
		{
			check(FMath::IsPowerOfTwo(m_HashSize));
			
			// realloc hash
			m_HashSize = const_cast<TSet*>(this)->m_Elements.GetAllocator().Reserve(m_Hash, m_HashSize);
			
			if (m_HashSize)
			{
				// reset hash 
				for (SetElementId* Ptr = m_Hash, *End = m_Hash + m_HashSize; Ptr != End; ++Ptr)
				{
					Ptr->Reset();
				}

				// Add the existing elements to the new hash.
				for (auto It = m_Elements.begin(); It; ++It)
				{
					SetElementId& IdRef = _BucketId(It->Hash);
					It->HashNextId = IdRef;
					IdRef = It.GetIndex();
				}
			}
		}
		bool _ConditionalRehash(int32 NumHashedElements, bool bAllowShrinking = false) const
		{
			// Calculate the desired hash size for the specified number of elements.
			const int32 DesiredHashSize = _GetNumberOfHashBuckets(NumHashedElements);

			// If the hash hasn't been created yet, or is smaller than the desired hash size, rehash.
			if (NumHashedElements > 0 &&	// must have element 
				(	!m_HashSize ||			// no exist allocation 
					m_HashSize < DesiredHashSize ||	// exist allocation is too small
					(m_HashSize > DesiredHashSize && bAllowShrinking)))	// need shrinking 
			{
				m_HashSize = DesiredHashSize;
				_Rehash();
				return true;
			}
			else
			{
				return false;
			}
		}

		// link element to hash bucket 
		FORCEINLINE void _LinkElement(SetElementId ElementId, const SetElement& Element, uint32 KeyHash) const
		{
			// Compute the hash bucket the element goes in.
			Element.Hash = KeyHash;

			// Link the element into the hash bucket.
			SetElementId& IdRef = _BucketId(KeyHash);
			Element.HashNextId = IdRef;
			IdRef = ElementId;
		}

		// implement emplace, before call this function, element has been added to sparse array
		// we only need check duplicate and link element to bucket 
		SetElementId _EmplaceImpl(uint32 KeyHash, SetElement& Element, SetElementId ElementId, bool* bIsAlreadyInSetPtr)
		{
			Element.Hash = KeyHash;
			// if we not support duplicate key, then check whether the key is unique 
			if constexpr (!KeyFuncs::bAllowDuplicateKeys)
			{
				bool bIsAlreadyInSet = false;
				// Don't bother searching for a duplicate if this is the first element we're adding
				if (m_Elements.Num() != 1)
				{
					SetElementId ExistingId = FindIdByHash(KeyHash, KeyFuncs::Key(Element.Value));
					bIsAlreadyInSet = ExistingId.IsValid();
					if (bIsAlreadyInSet)
					{
						// cover the old element 
						Memmove(&m_Elements[ExistingId].Value, &Element.Value, sizeof(SetElement));

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
					if (!_ConditionalRehash(m_Elements.Num()))
					{
						// If the rehash didn't add the new element to the hash, add it.
						_LinkElement(ElementId, Element, KeyHash);
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
				if (!_ConditionalRehash(m_Elements.Num()))
				{
					// If the rehash didn't add the new element to the hash, add it.
					_LinkElement(ElementId, Element, KeyHash);
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
		FORCEINLINE SizeType _RemoveImpl(uint32 KeyHash, const KeyType& Key)
		{
			int32 NumRemovedElements = 0;

			// get the head of hash linked list 
			SetElementId* Ptr = &_BucketId(KeyHash);
			while (Ptr->IsValid())
			{
				auto& Element = m_Elements[*Ptr];
				if (KeyFuncs::Matches(KeyFuncs::Key(Element.Value), Key))
				{
					SetElementId RemoveIndex = *Ptr;
					// link to next element id 
					*Ptr = Element.HashNextId;
					// remove element
					m_Elements.RemoveAt(RemoveIndex);

					++NumRemovedElements;
					if constexpr (!KeyFuncs::bAllowDuplicateKeys) break;
				}
				else
				{
					Ptr = &Element.HashNextId;
				}
			}

			return NumRemovedElements;
		}
		//----------------------------End helper functions----------------------------
	public:
		// construct 
		FORCEINLINE TSet(Alloc&& InAlloc = Alloc())
			: m_HashSize(0)
			, m_Hash(nullptr)
			, m_Elements(std::move(InAlloc))
		{ }
		FORCEINLINE TSet(std::initializer_list<T> InitList, Alloc&& InAlloc = Alloc())
			: m_HashSize(0)
			, m_Hash(nullptr)
			, m_Elements(std::move(InAlloc))
		{
			*this += (InitList);
		}
		FORCEINLINE explicit TSet(const TArray<T>& InArray, Alloc&& InAlloc = Alloc())
			: m_HashSize(0)
			, m_Hash(nullptr)
			, m_Elements(std::move(InAlloc))
		{
			*this += (InArray);
		}
		FORCEINLINE explicit TSet(TArray<T>&& InArray, Alloc&& InAlloc = Alloc())
			: m_HashSize(0)
			, m_Hash(nullptr)
			, m_Elements(std::move(InAlloc))
		{
			*this += (std::move(InArray));
		}

		// copy construct 
		FORCEINLINE TSet(const TSet& Other, Alloc&& InAlloc = Alloc())
			: m_HashSize(0)
			, m_Hash(nullptr)
			, m_Elements(std::move(InAlloc))
		{
			*this = Other;
		}

		// move construct 
		TSet(TSet&& Other)
			: m_HashSize(Other.m_HashSize)
			, m_Hash(Other.m_Hash)
			, m_Elements(std::move(Other.m_Elements))
		{
			Other.m_HashSize = 0;
			Other.m_Hash = nullptr;
		}

		// destructor 
		~TSet() { if (m_Hash) m_HashSize = m_Elements.GetAllocator().Free(m_Hash); }

		// assign 
		TSet& operator=(const TSet& Copy)
		{
			if (this == &Copy) return *this;
			// copy hash bucket 
			m_HashSize = m_Elements.GetAllocator().Reserve(m_Hash, Copy.m_HashSize);
			ConstructItems(m_Hash, Copy.m_Hash, Copy.m_HashSize);

			// copy other info 
			m_HashSize = Copy.m_HashSize;
			m_Elements = Copy.m_Elements;
			return *this;
		}
		TSet& operator=(std::initializer_list<T> InitList)
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
			if (m_Hash) m_HashSize = m_Elements.GetAllocator().Free(m_Hash);

			// move data 
			m_Elements = std::move(Other.m_Elements);
			m_Hash = Other.m_Hash;
			m_HashSize = Other.m_HashSize;

			// invalidate other 
			Other.m_HashSize = 0;
			Other.m_Hash = nullptr;
			return *this;
		}

		// get information 
		FORCEINLINE SizeType Num() const { return m_Elements.Num(); }
		FORCEINLINE SizeType GetMaxIndex() const { return m_Elements.GetMaxIndex(); }
		FORCEINLINE SizeType Max() const { return m_Elements.Max(); }
		FORCEINLINE Alloc& GetAllocator() { return m_Elements.GetAllocator(); }
		FORCEINLINE const Alloc& GetAllocator() const { return m_Elements.GetAllocator(); }

		// empty & reset & shrink & reserve 
		FORCEINLINE void Empty(SizeType ExpectedNumElements = 0)
		{
			m_Elements.Empty(ExpectedNumElements);
			if (!_ConditionalRehash(ExpectedNumElements, true))  _CleanBucket();
		}
		FORCEINLINE void Reset(SizeType ExpectedNumElements = 0)
		{
			if (Num() == 0) return;
			m_Elements.Reset(ExpectedNumElements);
			_CleanBucket();
		}
		FORCEINLINE void Shrink()
		{
			m_Elements.Shrink();
			Relax();
		}
		FORCEINLINE void Reserve(SizeType ExpectedNumElements)
		{
			if (ExpectedNumElements <= m_Elements.Num()) return;
			m_Elements.Reserve(ExpectedNumElements);
			const SizeType NewHashSize = _GetNumberOfHashBuckets(ExpectedNumElements);
			if (!m_HashSize || m_HashSize < NewHashSize)
			{
				m_HashSize = NewHashSize;
				_Rehash();
			}
		}

		// compact 
		FORCEINLINE void Compact() { if (m_Elements.Compact()) _Rehash(); }
		FORCEINLINE void CompactStable() { if (m_Elements.CompactStable()) _Rehash(); }

		// relax, the element will symmetrical distribution 
		FORCEINLINE void Relax() { _ConditionalRehash(m_Elements.Num(), true); }

		// check is valid 
		FORCEINLINE bool IsValid(SetElementId Id) const { return Id.IsValid() && Id < m_Elements.IsValidIndex(Id); }

		// accessor
		FORCEINLINE T& operator[](SetElementId Id) { return m_Elements[Id].Value; }
		FORCEINLINE const T& operator[](SetElementId Id) const { return m_Elements[Id].Value; }

		// add 
		FORCEINLINE SetElementId Add(const T& InElement, bool* bIsAlreadyInSetPtr = nullptr) { return Emplace(InElement, bIsAlreadyInSetPtr); }
		FORCEINLINE SetElementId Add(T&& InElement, bool* bIsAlreadyInSetPtr = nullptr) { return Emplace(std::move(InElement), bIsAlreadyInSetPtr); }
		FORCEINLINE SetElementId AddByHash(uint32 KeyHash, const T& InElement, bool* bIsAlreadyInSetPtr = nullptr) { return EmplaceByHash(KeyHash, InElement, bIsAlreadyInSetPtr); }
		FORCEINLINE SetElementId AddByHash(uint32 KeyHash, T&& InElement, bool* bIsAlreadyInSetPtr = nullptr) { return EmplaceByHash(KeyHash, std::move(InElement), bIsAlreadyInSetPtr); }

		// emplace 
		template <typename ArgsType>
		SetElementId Emplace(ArgsType&& Args, bool* bIsAlreadyInSetPtr = nullptr)
		{
			// Create a new element.
			auto ElementAllocation = m_Elements.AddUninitialized();
			SetElement& Element = *new (ElementAllocation.Pointer) SetElement(std::forward<ArgsType>(Args));

			// compute hash 
			uint32 KeyHash = KeyFuncs::Hash(KeyFuncs::Key(Element.Value));
			
			// perform emplace 
			return _EmplaceImpl(KeyHash, Element, ElementAllocation.Index, bIsAlreadyInSetPtr);
		}
		template <typename ArgsType>
		SetElementId EmplaceByHash(uint32 KeyHash, ArgsType&& Args, bool* bIsAlreadyInSetPtr = nullptr)
		{
			// Create a new element.
			auto ElementAllocation = m_Elements.AddUninitialized();
			SetElement& Element = *new (ElementAllocation.Pointer) SetElement(std::forward<ArgsType>(Args));

			return _EmplaceImpl(KeyHash, Element, ElementAllocation.Index, bIsAlreadyInSetPtr);
		}

		// append 
		TSet& operator+=(const TArray<T>& InElements)
		{
			Reserve(m_Elements.Num() + InElements.Num());
			for (const T& Element : InElements)
			{
				Add(Element);
			}
			return *this;
		}
		TSet& operator+=(TArray<T>&& InElements)
		{
			Reserve(m_Elements.Num() + InElements.Num());
			for (T& Element : InElements)
			{
				Add(std::move(Element));
			}
			InElements.Reset();
			return *this;
		}
		TSet& operator+=(const TSet& OtherSet)
		{
			Reserve(m_Elements.Num() + OtherSet.Num());
			for (const T& Element : OtherSet)
			{
				Add(Element);
			}
			return *this;
		}
		TSet& operator+=(TSet&& OtherSet)
		{
			Reserve(m_Elements.Num() + OtherSet.Num());
			for (T& Element : OtherSet)
			{
				Add(std::move(Element));
			}
			OtherSet.Reset();
			return *this;
		}
		TSet& operator+=(std::initializer_list<T> InitList)
		{
			Reserve(m_Elements.Num() + (int32)InitList.size());
			for (const T& Element : InitList)
			{
				Add(Element);
			}
			return *this;
		}

		// remove
		void Remove(SetElementId ElementId)
		{
			if (m_Elements.Num())
			{
				const auto& ElementBeingRemoved = m_Elements[ElementId];

				// Remove the element from the hash bucket 
				for (SetElementId* NextElementId = &_BucketId(ElementBeingRemoved.Hash);
					NextElementId->IsValid();
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
		SizeType Remove(const KeyType& Key)
		{
			if (m_Elements.Num()) return _RemoveImpl(KeyFuncs::Hash(Key), Key); 
			return 0;
		}
		SizeType RemoveByHash(uint32 KeyHash, const KeyType& Key)
		{
			check(KeyHash == KeyFuncs::Hash(Key));
			if (m_Elements.Num()) return _RemoveImpl(KeyHash, Key); 
			return 0;
		}

		// find 
		SetElementId FindId(const KeyType& Key) const
		{
			if (m_Elements.Num())
			{
				for (SetElementId ElementId = _BucketId(KeyFuncs::Hash(Key));
					ElementId.IsValid();
					ElementId = m_Elements[ElementId].HashNextId)
				{
					if (KeyFuncs::Matches(KeyFuncs::Key(m_Elements[ElementId].Value), Key))
					{
						return ElementId;
					}
				}
			}
			return SetElementId();
		}
		SetElementId FindIdByHash(uint32 KeyHash, const KeyType& Key) const
		{
			if (m_Elements.Num())
			{
				check(KeyHash == KeyFuncs::Hash(Key));

				for (SetElementId ElementId = _BucketId(KeyHash);
					ElementId.IsValid();
					ElementId = m_Elements[ElementId].HashNextId)
				{
					if (KeyFuncs::Matches(KeyFuncs::Key(m_Elements[ElementId].Value), Key))
					{
						return ElementId;
					}
				}
			}
			return SetElementId();
		}
		FORCEINLINE T* Find(const KeyType& Key)
		{
			SetElementId ElementId = FindId(Key);
			if (ElementId.IsValid())
			{
				return &m_Elements[ElementId].Value;
			}
			else
			{
				return nullptr;
			}
		}
		FORCEINLINE const T* Find(const KeyType& Key) const
		{
			return const_cast<TSet*>(this)->Find(Key);
		}
		T* FindByHash(uint32 KeyHash, const KeyType& Key)
		{
			SetElementId ElementId = FindIdByHash(KeyHash, Key);
			if (ElementId.IsValid())
			{
				return &m_Elements[ElementId].Value;
			}
			else
			{
				return nullptr;
			}
		}
		const T* FindByHash(uint32 KeyHash, const KeyType& Key) const
		{
			return const_cast<TSet*>(this)->FindByHash(KeyHash, Key);
		}

		// contains 
		FORCEINLINE bool Contains(const KeyType& Key) const { return FindId(Key).IsValid(); }
		FORCEINLINE bool ContainsByHash(uint32 KeyHash, const KeyType& Key) const { return FindIdByHash(KeyHash, Key).IsValid(); }

		// sort 
		template <typename TPred = TLess<T>>
		void Sort(TPred&& Pred = TLess<T>())
		{
			m_Elements.Sort([&](const SetElement& A, const SetElement& B)->bool { return Pred(A.Value, B.Value); });
			_Rehash();
		}
		template <typename TPred = TLess<T>>
		void StableSort(TPred&& Pred = TLess<T>())
		{
			m_Elements.StableSort([&](const SetElement& A, const SetElement& B)->bool { return Pred(A.Value, B.Value); });
			_Rehash();
		}

		// iterate over all elements for the hash entry of the given key, and verify that the ids are valid
		bool VerifyHashElementsKey(const KeyType& Key)
		{
			bool bResult = true;
			if (m_Elements.Num())
			{
				SetElementId ElementId = _BucketId(KeyFuncs::Hash(Key));
				while (ElementId.IsValid())
				{
					if (!IsValid(ElementId))
					{
						bResult = false;
						break;
					}
					ElementId = m_Elements[ElementId].HashNextId;
				}
			}
			return bResult;
		}

		// compare 
		bool operator==(const TSet& Other) const
		{
			if (Other.Num() != Num()) return false;
			return this->Includes(Other);
		}
		bool operator!=(const TSet& Other) const { return *this != Other; }

		// And Or Diff Include
		TSet Intersect(const TSet& OtherSet) const
		{
			const bool bOtherSmaller = (Num() > OtherSet.Num());
			const TSet& A = (bOtherSmaller ? OtherSet : *this);
			const TSet& B = (bOtherSmaller ? *this : OtherSet);

			TSet Result(std::move(const_cast<TSet*>(this)->GetAllocator()));
			Result.Reserve(A.Num()); // Worst case

			for (TConstIterator SetIt(A); SetIt; ++SetIt)
			{
				if (B.Contains(KeyFuncs::Key(*SetIt)))
				{
					Result.Add(*SetIt);
				}
			}
			return Result;
		}
		TSet Union(const TSet& OtherSet) const
		{
			TSet Result(std::move(const_cast<TSet*>(this)->GetAllocator()));
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
		TSet Difference(const TSet& OtherSet) const
		{
			TSet Result(std::move(const_cast<TSet*>(this)->GetAllocator()));
			Result.Reserve(Num()); // Worst case is no elements of this are in Other

			for (TConstIterator SetIt(*this); SetIt; ++SetIt)
			{
				if (!OtherSet.Contains(KeyFuncs::Key(*SetIt)))
				{
					Result.Add(*SetIt);
				}
			}
			return Result;
		}
		bool Includes(const TSet& OtherSet) const
		{
			bool bIncludesSet = true;
			if (OtherSet.Num() <= Num())
			{
				for (TConstIterator OtherSetIt(OtherSet); OtherSetIt; ++OtherSetIt)
				{
					if (!Contains(KeyFuncs::Key(*OtherSetIt)))
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
		TArray<T> Array() const
		{
			TArray<T> Result(GetAllocator());
			Result.Reserve(Num());
			for (TConstIterator SetIt(*this); SetIt; ++SetIt)
			{
				Result.Add(*SetIt);
			}
			return Result;
		}

		// Debug check 
		FORCEINLINE void CheckAddress(const T* Addr) const { m_Elements.CheckAddress(Addr); }

		//-----------------------------------------------iterators-----------------------------------------------
		class TIterator
		{
			using ElementItType = typename TSparseArray<SetElement, Alloc>::TIterator;
			ElementItType	m_ElementIt;
			TSet&			m_Set;
		public:
			FORCEINLINE TIterator(TSet& InSet, SizeType StartIndex = 0) : m_ElementIt(InSet.m_Elements, StartIndex), m_Set(InSet) {}

			FORCEINLINE TIterator& operator++() { ++m_ElementIt; return *this; }
			FORCEINLINE explicit operator bool() const { return !!m_ElementIt; }
			FORCEINLINE bool operator !() const { return !(bool)*this; }

			FORCEINLINE T* operator->() const { return &m_ElementIt->Value; }
			FORCEINLINE T& operator*() const { return m_ElementIt->Value; }

			FORCEINLINE friend bool operator==(const TIterator& Lhs, const TIterator& Rhs) { return Lhs.m_ElementIt == Rhs.m_ElementIt; }
			FORCEINLINE friend bool operator!=(const TIterator& Lhs, const TIterator& Rhs) { return Lhs.m_ElementIt != Rhs.m_ElementIt; }

			FORCEINLINE SetElementId GetId() const { return m_ElementIt.GetIndex(); }
			FORCEINLINE void RemoveCurrent() 	{ Set.Remove(GetId); }
		};
		class TConstIterator
		{
			using ElementItType = typename TSparseArray<SetElement, Alloc>::TConstIterator;
			ElementItType	m_ElementIt;
		public:
			FORCEINLINE TConstIterator(const TSet& InSet, SizeType StartIndex = 0) : m_ElementIt(InSet.m_Elements, StartIndex) {}

			FORCEINLINE TConstIterator& operator++() { ++m_ElementIt; return *this; }
			FORCEINLINE explicit operator bool() const { return !!m_ElementIt; }
			FORCEINLINE bool operator !() const { return !(bool)*this; }

			FORCEINLINE const T* operator->() const { return &m_ElementIt->Value; }
			FORCEINLINE const T& operator*() const { return m_ElementIt->Value; }

			FORCEINLINE friend bool operator==(const TConstIterator& Lhs, const TConstIterator& Rhs) { return Lhs.m_ElementIt == Rhs.m_ElementIt; }
			FORCEINLINE friend bool operator!=(const TConstIterator& Lhs, const TConstIterator& Rhs) { return Lhs.m_ElementIt != Rhs.m_ElementIt; }

			FORCEINLINE SetElementId GetId() const { return m_ElementIt.GetIndex(); }
		};
	
		// Support foreach  
		FORCEINLINE TIterator      begin()		{ return TIterator(*this); }
		FORCEINLINE TConstIterator begin() const{ return TConstIterator(*this); }
		FORCEINLINE TIterator      end()		{ return TIterator(*this, GetMaxIndex()); }
		FORCEINLINE TConstIterator end() const	{ return TConstIterator(*this, GetMaxIndex()); }
	};
}
