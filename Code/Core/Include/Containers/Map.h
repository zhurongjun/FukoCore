#pragma once
#include "CoreType.h"
#include "CoreConfig.h"
#include "Templates/Pair.h"
#include "Set.h"
#include <Memory/Allocators.h>

// KeyFunctions 
namespace Fuko
{
	template<typename TK, typename TV, bool bInAllowDuplicateKeys>
	struct TDefaultMapKeyFuncs
	{
		using KeyType = TK;
		using ElementType = TPair<TK, TV>;
		static constexpr bool bAllowDuplicateKeys = bInAllowDuplicateKeys;

		static FORCEINLINE const KeyType& Key(const ElementType& Element) { return Element.Key; }
		template<typename ComparableKey>
		static FORCEINLINE bool Matches(const KeyType& A, ComparableKey B) { return A == B; }
		template<typename ComparableKey>
		static FORCEINLINE uint32 Hash(ComparableKey Key) { return GetTypeHash(Key); }
	};
}

// Base map
namespace Fuko
{
	template <typename KeyType, typename ValueType, template<typename> typename Alloc,typename KeyFuncs>
	class TMapBase
	{
		//-----------------------------begin help function-----------------------------
		// implement find or add 
		template <typename InitKeyType>
		ValueType& FindOrAddImpl(uint32 KeyHash, InitKeyType&& Key)
		{
			if (auto* Pair = m_Pairs.FindByHash(KeyHash, Key)) return Pair->Value;
			return AddByHash(KeyHash, std::forward<InitKeyType>(Key));
		}
		template <typename InitKeyType, typename InitValueType>
		ValueType& FindOrAddImpl(uint32 KeyHash, InitKeyType&& Key, InitValueType&& Value)
		{
			if (auto* Pair = m_Pairs.FindByHash(KeyHash, Key)) return Pair->Value;
			return AddByHash(KeyHash, std::forward<InitKeyType>(Key), std::forward<InitValueType>(Value));
		}
		//------------------------------end help function------------------------------
	public:
		using ElementType = TPair<KeyType, ValueType>;
		using ElementSetType = TSet<ElementType, Alloc, KeyFuncs>;
		using AllocType = typename ElementSetType::AllocType;
		using SizeType = typename AllocType::SizeType;

		// special predicate for compare key
		template<typename Predicate>
		class FKeyComparisonClass
		{
			TDereferenceWrapper<KeyType, Predicate> Pred;
		public:
			FORCEINLINE FKeyComparisonClass(Predicate&& InPredicate) : Pred(std::move(InPredicate)) {}
			FORCEINLINE bool operator()(const ElementType& A, const ElementType& B) const
			{
				return Pred(A.Key, B.Key);
			}
		};

		// special predicate for compare value 
		template<typename Predicate>
		class FValueComparisonClass
		{
			TDereferenceWrapper< ValueType, Predicate> Pred;
		public:
			FORCEINLINE FValueComparisonClass(const Predicate& InPredicate) : Pred(InPredicate) {}
			FORCEINLINE bool operator()(const typename ElementType& A, const typename ElementType& B) const
			{
				return Pred(A.Value, B.Value);
			}
		};
	protected:
		ElementSetType	m_Pairs;

		// construct 
		FORCEINLINE TMapBase(AllocType&& InAlloc)
			: m_Pairs(std::move(InAlloc)) {}
		
		// copy construct
		FORCEINLINE TMapBase(const TMapBase& Other, AllocType&& InAlloc)
			: m_Pairs(Other.m_Pairs, std::move(InAlloc)) {}
		
		// move construct 
		FORCEINLINE TMapBase(TMapBase&&) = default;
		FORCEINLINE TMapBase& operator=(const TMapBase&) = default;
		FORCEINLINE TMapBase& operator=(TMapBase&&) = default;
	public:
		// operators 
		FORCEINLINE void Empty(SizeType ExpectedNumElements = 0) { m_Pairs.Empty(ExpectedNumElements); }
		FORCEINLINE void Reset(SizeType ExpectedNumElements = 0) { m_Pairs.Reset(ExpectedNumElements); }
		FORCEINLINE void Shrink() { m_Pairs.Shrink(); }
		FORCEINLINE void Compact(){ m_Pairs.Compact(); }
		FORCEINLINE void CompactStable() { m_Pairs.CompactStable(); }
		FORCEINLINE void Reserve(SizeType Number) { m_Pairs.Reserve(Number); }
		FORCEINLINE SizeType Num() const { return m_Pairs.Num(); }
		FORCEINLINE SizeType Max() const { return m_Pairs.Max(); }
		FORCEINLINE SizeType GetMaxIndex() const { return m_Pairs.GetMaxIndex(); }
		
		// compare operator that not care element order, may be very slow 
		bool OrderIndependentCompareEqual(const TMapBase& Other) const
		{
			if (Num() != Other.Num()) return false;
			for (typename ElementSetType::TConstIterator It(m_Pairs); It; ++It)
			{
				const ValueType* BVal = Other.Find(It->Key);
				if (BVal == nullptr) return false;
				if (!(*BVal == It->Value)) return false;
			}
			return true;
		}

		// get unique key array 
		int32 GetKeys(TArray<KeyType>& OutKeys) const
		{
			TSet<KeyType> VisitedKeys;
			for (typename ElementSetType::TConstIterator It(m_Pairs); It; ++It)
			{
				if (!VisitedKeys.Contains(It->Key))
				{
					OutKeys.Add(It->Key);
					VisitedKeys.Add(It->Key);
				}
			}
			return OutKeys.Num();
		}

		// Add by key and value 
		FORCEINLINE ValueType& Add(const KeyType&  InKey, const ValueType&  InValue) { return Emplace(InKey, InValue); }
		FORCEINLINE ValueType& Add(const KeyType&  InKey, ValueType&& InValue) { return Emplace(InKey, std::move(InValue)); }
		FORCEINLINE ValueType& Add(KeyType&& InKey, const ValueType&  InValue) { return Emplace(std::move(InKey), InValue); }
		FORCEINLINE ValueType& Add(KeyType&& InKey, ValueType&& InValue) { return Emplace(std::move(InKey), std::move(InValue)); }
		FORCEINLINE ValueType& AddByHash(uint32 KeyHash, const KeyType&  InKey, const ValueType&  InValue) { return EmplaceByHash(KeyHash, InKey, InValue); }
		FORCEINLINE ValueType& AddByHash(uint32 KeyHash, const KeyType&  InKey, ValueType&& InValue) { return EmplaceByHash(KeyHash, InKey, std::move(InValue)); }
		FORCEINLINE ValueType& AddByHash(uint32 KeyHash, KeyType&& InKey, const ValueType&  InValue) { return EmplaceByHash(KeyHash, std::move(InKey), InValue); }
		FORCEINLINE ValueType& AddByHash(uint32 KeyHash, KeyType&& InKey, ValueType&& InValue) { return EmplaceByHash(KeyHash, std::move(InKey), std::move(InValue)); }
		
		// Add key only 
		FORCEINLINE ValueType& Add(const KeyType&  InKey) { return Emplace(InKey); }
		FORCEINLINE ValueType& Add(KeyType&& InKey) { return Emplace(std::move(InKey)); }
		FORCEINLINE ValueType& AddByHash(uint32 KeyHash, const KeyType&  InKey) { return EmplaceByHash(KeyHash, InKey); }
		FORCEINLINE ValueType& AddByHash(uint32 KeyHash, KeyType&& InKey) { return EmplaceByHash(KeyHash, std::move(InKey)); }
		
		// Add by tuple 
		FORCEINLINE ValueType& Add(const TTuple<KeyType, ValueType>&  InKeyValue) { return Emplace(InKeyValue.Key, InKeyValue.Value); }
		FORCEINLINE ValueType& Add(TTuple<KeyType, ValueType>&& InKeyValue) { return Emplace(std::move(InKeyValue.Key), std::move(InKeyValue.Value)); }

		// emplace 
		template <typename InitKeyType, typename InitValueType>
		ValueType& Emplace(InitKeyType&& InKey, InitValueType&& InValue)
		{
			const SetElementId PairId = m_Pairs.Emplace(ElementType(std::forward<InitKeyType>(InKey), std::forward<InitValueType>(InValue)));
			return m_Pairs[PairId].Value;
		}
		template <typename InitKeyType, typename InitValueType>
		ValueType& EmplaceByHash(uint32 KeyHash, InitKeyType&& InKey, InitValueType&& InValue)
		{
			const SetElementId PairId = m_Pairs.EmplaceByHash(KeyHash, ElementType(std::forward<InitKeyType>(InKey), std::forward<InitValueType>(InValue)));
			return m_Pairs[PairId].Value;
		}
		template <typename InitKeyType>
		ValueType& Emplace(InitKeyType&& InKey)
		{
			const SetElementId PairId = m_Pairs.Emplace(ElementType(std::forward<InitKeyType>(InKey)));
			return m_Pairs[PairId].Value;
		}
		template <typename InitKeyType>
		ValueType& EmplaceByHash(uint32 KeyHash, InitKeyType&& InKey)
		{
			const SetElementId PairId = m_Pairs.EmplaceByHash(KeyHash, ElementType(std::forward<InitKeyType>(InKey)));
			return m_Pairs[PairId].Value;
		}

		// remove 
		FORCEINLINE int32 Remove(const KeyType& InKey) { return m_Pairs.Remove(InKey); }
		template<typename ComparableKey>
		FORCEINLINE int32 RemoveByHash(uint32 KeyHash, const ComparableKey& Key) { return m_Pairs.RemoveByHash(KeyHash, Key); }

		// Find 
		const KeyType* FindKey(const ValueType& Value) const
		{
			for (typename ElementSetType::TConstIterator PairIt(m_Pairs); PairIt; ++PairIt)
			{
				if (PairIt->Value == Value)
				{
					return &PairIt->Key;
				}
			}
			return nullptr;
		}
		FORCEINLINE ValueType* Find(const KeyType& Key)
		{
			if (auto* Pair = m_Pairs.Find(Key))
			{
				return &Pair->Value;
			}
			return nullptr;
		}
		FORCEINLINE const ValueType* Find(const KeyType& Key) const
		{
			return const_cast<TMapBase*>(this)->Find(Key);
		}
		template<typename ComparableKey>
		FORCEINLINE ValueType* FindByHash(uint32 KeyHash, const ComparableKey& Key)
		{
			if (auto* Pair = m_Pairs.FindByHash(KeyHash, Key))
			{
				return &Pair->Value;
			}
			return nullptr;
		}
		template<typename ComparableKey>
		FORCEINLINE const ValueType* FindByHash(uint32 KeyHash, const ComparableKey& Key) const
		{
			return const_cast<TMapBase*>(this)->FindByHash(KeyHash, Key);
		}

		// find or add key only 
		FORCEINLINE ValueType& FindOrAdd(const KeyType& Key) { return FindOrAddImpl(KeyFuncs::Hash(Key), Key); }
		FORCEINLINE ValueType& FindOrAdd(KeyType&& Key) { return FindOrAddImpl(KeyFuncs::Hash(Key), std::move(Key)); }
		FORCEINLINE ValueType& FindOrAddByHash(uint32 KeyHash, const KeyType& Key) { return FindOrAddImpl(KeyHash, Key); }
		FORCEINLINE ValueType& FindOrAddByHash(uint32 KeyHash, KeyType&& Key) { return FindOrAddImpl(KeyHash, std::move(Key)); }

		// find or add by key and value 
		FORCEINLINE ValueType& FindOrAdd(const KeyType&  Key, const ValueType&  Value) { return FindOrAddImpl(KeyFuncs::Hash(Key), Key, Value); }
		FORCEINLINE ValueType& FindOrAdd(const KeyType&  Key, ValueType&&       Value) { return FindOrAddImpl(KeyFuncs::Hash(Key), Key, std::move(Value)); }
		FORCEINLINE ValueType& FindOrAdd(KeyType&& Key, const ValueType&  Value) { return FindOrAddImpl(KeyFuncs::Hash(Key), std::move(Key), Value); }
		FORCEINLINE ValueType& FindOrAdd(KeyType&& Key, ValueType&&       Value) { return FindOrAddImpl(KeyFuncs::Hash(Key), std::move(Key), std::move(Value)); }
		FORCEINLINE ValueType& FindOrAddByHash(uint32 KeyHash, const KeyType&  Key, const ValueType&  Value) { return FindOrAddImpl(KeyHash, Key, Value); }
		FORCEINLINE ValueType& FindOrAddByHash(uint32 KeyHash, const KeyType&  Key, ValueType&& Value) { return FindOrAddImpl(KeyHash, Key, std::move(Value)); }
		FORCEINLINE ValueType& FindOrAddByHash(uint32 KeyHash, KeyType&& Key, const ValueType&  Value) { return FindOrAddImpl(KeyHash, std::move(Key), Value); }
		FORCEINLINE ValueType& FindOrAddByHash(uint32 KeyHash, KeyType&& Key, ValueType&& Value) { return FindOrAddImpl(KeyHash, std::move(Key), std::move(Value)); }

		// find with check instead of return a nullprt
		FORCEINLINE const ValueType& FindChecked(const KeyType& Key) const
		{
			const auto* Pair = m_Pairs.Find(Key);
			check(Pair != nullptr);
			return m_Pairs->Value;
		}
		FORCEINLINE ValueType& FindChecked(const KeyType& Key)
		{
			auto* Pair = m_Pairs.Find(Key);
			check(Pair != nullptr);
			return Pair->Value;
		}
		
		// find, return a reference 
		FORCEINLINE ValueType FindRef(const KeyType& Key) const
		{
			if (const auto* Pair = m_Pairs.Find(Key))
			{
				return Pair->Value;
			}

			return ValueType();
		}

		// Contains 
		FORCEINLINE bool Contains(const KeyType& Key) const { return m_Pairs.Contains(Key); }
		template<typename ComparableKey>
		FORCEINLINE bool ContainsByHash(uint32 KeyHash, const ComparableKey& Key) const { return m_Pairs.ContainsByHash(KeyHash, Key); }

		// Generate key & value array 
		void GenerateKeyArray(TArray<KeyType>& OutArray) const
		{
			OutArray.Empty(m_Pairs.Num());
			for (typename ElementSetType::TConstIterator PairIt(m_Pairs); PairIt; ++PairIt)
			{
				new(OutArray) KeyType(PairIt->Key);
			}
		}
		void GenerateValueArray(TArray<ValueType>& OutArray) const
		{
			OutArray.Empty(m_Pairs.Num());
			for (typename ElementSetType::TConstIterator PairIt(m_Pairs); PairIt; ++PairIt)
			{
				new(OutArray) ValueType(PairIt->Value);
			}
		}

		// Sort 
		template<typename Predicate>
		FORCEINLINE void KeySort(const Predicate& Pred) { m_Pairs.Sort(FKeyComparisonClass<Predicate>(Pred)); }
		template<typename Predicate>
		FORCEINLINE void KeyStableSort(const Predicate& Pred) { m_Pairs.StableSort(FKeyComparisonClass<Predicate>(Pred)); }

		template<typename Predicate>
		FORCEINLINE void ValueSort(const Predicate& Pred) { m_Pairs.Sort(FValueComparisonClass<Predicate>(Pred)); }
		template<typename Predicate>
		FORCEINLINE void ValueStableSort(const Predicate& Pred) { m_Pairs.StableSort(FValueComparisonClass<Predicate>(Pred)); }

		//----------------------------------------iterators----------------------------------------
		class TIterator
		{
			using ItType = typename ElementSetType::TIterator;
			TMapBase&	m_Map;
			ItType		m_SetIt;
		public:
			FORCEINLINE TIterator(TMapBase& InMap, SizeType StartIndex = 0)
				: m_SetIt(InMap.m_Pairs, StartIndex)
				, m_Map(InMap) {}

			FORCEINLINE TIterator& operator++()
			{
				++m_SetIt;
				return *this;
			}

			FORCEINLINE explicit operator bool() const { return !!m_SetIt; }
			FORCEINLINE bool operator !() const { return !(bool)*this; }

			FORCEINLINE friend bool operator==(const TIterator& Lhs, const TIterator& Rhs) { return Lhs.m_SetIt == Rhs.m_SetIt; }
			FORCEINLINE friend bool operator!=(const TIterator& Lhs, const TIterator& Rhs) { return Lhs.m_SetIt != Rhs.m_SetIt; }

			FORCEINLINE KeyType&   Key()   const { return m_SetIt->Key; }
			FORCEINLINE ValueType& Value() const { return m_SetIt->Value; }

			FORCEINLINE ElementType& operator* () const { return  *m_SetIt; }
			FORCEINLINE ElementType* operator->() const { return &*m_SetIt; }

			FORCEINLINE void RemoveCurrent() { m_SetIt->RemoveCurrent(); }
		};
		class TConstIterator
		{
			using ItType = typename ElementSetType::TConstIterator;
			ItType	m_SetIt;
		public:
			FORCEINLINE TConstIterator(const TMapBase& InMap, SizeType StartIndex = 0)
				: m_SetIt(InMap.m_Pairs, StartIndex)
				, m_Map(InMap) {}

			FORCEINLINE TConstIterator& operator++()
			{
				++m_SetIt;
				return *this;
			}

			FORCEINLINE explicit operator bool() const { return !!m_SetIt; }
			FORCEINLINE bool operator !() const { return !(bool)*this; }

			FORCEINLINE friend bool operator==(const TConstIterator& Lhs, const TConstIterator& Rhs) { return Lhs.m_SetIt == Rhs.m_SetIt; }
			FORCEINLINE friend bool operator!=(const TConstIterator& Lhs, const TConstIterator& Rhs) { return Lhs.m_SetIt != Rhs.m_SetIt; }

			FORCEINLINE const KeyType&   Key()   const { return m_SetIt->Key; }
			FORCEINLINE const ValueType& Value() const { return m_SetIt->Value; }

			FORCEINLINE const ElementType& operator* () const { return  *m_SetIt; }
			FORCEINLINE const ElementType* operator->() const { return &*m_SetIt; }
		};
		// Support foreach 
		FORCEINLINE TIterator      begin()			{ return TIterator(*this); }
		FORCEINLINE TConstIterator begin() const	{ return TConstIterator(*this); }
		FORCEINLINE TIterator      end()			{ return TIterator(*this, GetMaxIndex()); }
		FORCEINLINE TConstIterator end() const		{ return TConstIterator(*this, GetMaxIndex()); }
	};
}

// Signal map
namespace Fuko
{
	template<typename KeyType, typename ValueType
		, template<typename> typename Alloc = TPmrAllocator
		, typename KeyFuncs = TDefaultMapKeyFuncs<KeyType, ValueType, false>>
	class TMap : public TMapBase<KeyType, ValueType, Alloc, KeyFuncs>
	{
		static_assert(!KeyFuncs::bAllowDuplicateKeys, "TMap cannot be instantiated with a KeyFuncs which allows duplicate keys");
	public:
		using Super = TMapBase<KeyType, ValueType, Alloc, KeyFuncs>;
		using typename Super::ElementType;
		using typename Super::SizeType;
		using typename Super::AllocType;

		// construct 
		FORCEINLINE TMap(AllocType&& InAlloc) :Super(InAlloc) {}
		TMap(std::initializer_list<ElementType> InitList,AllocType&& InAlloc) : Super(InAlloc)
		{
			this->Reserve((int32)InitList.size());
			for (const ElementType& Element : InitList)
			{
				this->Add(Element.Key, Element.Value);
			}
		}
		
		// move construct 
		FORCEINLINE TMap(TMap&&) = default;

		// copy construct 
		FORCEINLINE TMap(const TMap& Other, AllocType&& InAlloc) :Super(Other, InAlloc) {}

		// assign 
		FORCEINLINE TMap& operator=(const TMap&) = default;
		TMap& operator=(std::initializer_list<ElementType> InitList)
		{
			this->Empty((int32)InitList.size());
			for (const ElementType& Element : InitList)
			{
				this->Add(Element.Key, Element.Value);
			}
			return *this;
		}

		// move assign 
		FORCEINLINE TMap& operator=(TMap&&) = default;

		FORCEINLINE bool RemoveAndCopyValue(const KeyType& Key, ValueType& OutRemovedValue)
		{
			const SetElementId PairId = Super::m_Pairs.FindId(Key);
			if (!PairId.IsValidId())
				return false;

			OutRemovedValue = std::move(Super::m_Pairs[PairId].Value);
			Super::m_Pairs.Remove(PairId);
			return true;
		}
		FORCEINLINE ValueType FindAndRemoveChecked(const KeyType& Key)
		{
			const SetElementId PairId = Super::m_Pairs.FindId(Key);
			check(PairId.IsValidId());
			ValueType Result = std::move(Super::m_Pairs[PairId].Value);
			Super::m_Pairs.Remove(PairId);
			return Result;
		}

		void Append(TMap&& OtherMap)
		{
			this->Reserve(this->Num() + OtherMap.Num());
			for (auto& Pair : OtherMap)
			{
				this->Add(std::move(Pair.Key), std::move(Pair.Value));
			}

			OtherMap.Reset();
		}
		void Append(const TMap& OtherMap)
		{
			this->Reserve(this->Num() + OtherMap.Num());
			for (auto& Pair : OtherMap)
			{
				this->Add(Pair.Key, Pair.Value);
			}
		}

		FORCEINLINE       ValueType& operator[](const KeyType& Key) { return this->FindOrAdd(Key); }
		FORCEINLINE const ValueType& operator[](const KeyType& Key) const { return this->FindOrAdd(Key); }
	};
}

// Multi map
namespace Fuko
{
	template<typename KeyType, typename ValueType
		, template<typename> typename Alloc = TPmrAllocator
		, typename KeyFuncs = TDefaultMapKeyFuncs<KeyType, ValueType, true>>
	class TMultiMap : public TMapBase<KeyType, ValueType, Alloc, KeyFuncs>
	{
		static_assert(KeyFuncs::bAllowDuplicateKeys, "TMultiMap cannot be instantiated with a KeyFuncs which disallows duplicate keys");
	public:
		using Super = TMapBase<KeyType, ValueType, Alloc, KeyFuncs>;
		using typename Super::ElementType;
		using typename Super::SizeType;
		using typename Super::AllocType;

		// construct 
		FORCEINLINE TMultiMap(AllocType&& InAlloc) :Super(InAlloc) {}
		TMultiMap(std::initializer_list<ElementType> InitList, AllocType&& InAlloc) :Super(InAlloc)
		{
			this->Reserve((int32)InitList.size());
			for (const ElementType& Element : InitList)
			{
				this->Add(Element.Key, Element.Value);
			}
		}
		
		// move construct 
		TMultiMap(TMultiMap&&) = default;
		
		// copy construct 
		TMultiMap(const TMultiMap& Other, AllocType&& InAlloc = AllocType()) :Super(Other , InAlloc) {}

		// assign 
		TMultiMap& operator=(const TMultiMap&) = default;
		TMultiMap& operator=(std::initializer_list<ElementType> InitList)
		{
			this->Empty((int32)InitList.size());
			for (const ElementType& Element : InitList)
			{
				this->Add(Element.Key, Element.Value);
			}
			return *this;
		}

		// move assign
		TMultiMap& operator=(TMultiMap&&) = default;
		
		// mult find
		void MultiFind(const KeyType& Key, TArray<ValueType>& OutValues, bool bMaintainOrder = false) const
		{
			for (typename Super::ElementSetType::TConstKeyIterator It(Super::Pairs, Key); It; ++It)
			{
				new(OutValues) ValueType(It->Value);
			}

			if (bMaintainOrder)
			{
				Algo::Reverse(OutValues);
			}
		}
		void MultiFindPointer(const KeyType& Key, TArray<const ValueType*>& OutValues, bool bMaintainOrder = false) const
		{
			for (typename Super::ElementSetType::TConstKeyIterator It(Super::Pairs, Key); It; ++It)
			{
				OutValues.Add(&It->Value);
			}

			if (bMaintainOrder)
			{
				Algo::Reverse(OutValues);
			}
		}
		void MultiFindPointer(const KeyType& Key, TArray<ValueType*>& OutValues, bool bMaintainOrder = false)
		{
			for (typename Super::ElementSetType::TKeyIterator It(Super::Pairs, Key); It; ++It)
			{
				OutValues.Add(&It->Value);
			}

			if (bMaintainOrder)
			{
				Algo::Reverse(OutValues);
			}
		}

		// unique add 
		FORCEINLINE ValueType& AddUnique(const KeyType&  InKey, const ValueType&  InValue) { return EmplaceUnique(InKey, InValue); }
		FORCEINLINE ValueType& AddUnique(const KeyType&  InKey, ValueType&& InValue) { return EmplaceUnique(InKey, std::move(InValue)); }
		FORCEINLINE ValueType& AddUnique(KeyType&& InKey, const ValueType&  InValue) { return EmplaceUnique(std::move(InKey), InValue); }
		FORCEINLINE ValueType& AddUnique(KeyType&& InKey, ValueType&& InValue) { return EmplaceUnique(std::move(InKey), std::move(InValue)); }

		// unique emplace 
		template <typename InitKeyType, typename InitValueType>
		ValueType& EmplaceUnique(InitKeyType&& InKey, InitValueType&& InValue)
		{
			if (ValueType* Found = FindPair(InKey, InValue))
			{
				return *Found;
			}

			// If there's no existing association with the same key and value, create one.
			return Super::Add(std::forward<InitKeyType>(InKey), std::forward<InitValueType>(InValue));
		}

		// remove
		FORCEINLINE int32 Remove(const KeyType& InKey)
		{
			return Super::Remove(InKey);
		}
		int32 Remove(const KeyType& InKey, const ValueType& InValue)
		{
			// Iterate over pairs with a matching key.
			int32 NumRemovedPairs = 0;
			for (auto It = Super::m_Pairs.begin(); It; ++It)
			{
				// If this pair has a matching value as well, remove it.
				if (It->Value == InValue)
				{
					It.RemoveCurrent();
					++NumRemovedPairs;
				}
			}
			return NumRemovedPairs;
		}
		int32 RemoveSingle(const KeyType& InKey, const ValueType& InValue)
		{
			// Iterate over pairs with a matching key.
			int32 NumRemovedPairs = 0;
			for (auto It = Super::m_Pairs.begin(); It; ++It)
			{
				// If this pair has a matching value as well, remove it.
				if (It->Value == InValue)
				{
					It.RemoveCurrent();
					++NumRemovedPairs;

					// We were asked to remove only the first association, so bail out.
					break;
				}
			}
			return NumRemovedPairs;
		}

		// find 
		FORCEINLINE const ValueType* FindPair(const KeyType& Key, const ValueType& Value) const
		{
			return const_cast<TMultiMap*>(this)->FindPair(Key, Value);
		}
		ValueType* FindPair(const KeyType& Key, const ValueType& Value)
		{
			// Iterate over pairs with a matching key.
			for (auto It = Super::m_Pairs.begin(); It; ++It)
			{
				// If the pair's value matches, return a pointer to it.
				if (It->Value == Value)
				{
					return &It->Value;
				}
			}

			return nullptr;
		}

		// num 
		int32 Num(const KeyType& Key) const
		{
			// Iterate over pairs with a matching key.
			int32 NumMatchingPairs = 0;
			for (typename Super::ElementSetType::TConstKeyIterator It(Super::m_Pairs, Key); It; ++It)
			{
				++NumMatchingPairs;
			}
			return NumMatchingPairs;
		}
		FORCEINLINE int32 Num() const
		{
			return Super::Num();
		}

		// append 
		void Append(TMultiMap&& OtherMultiMap)
		{
			this->Reserve(this->Num() + OtherMultiMap.Num());
			for (auto& Pair : OtherMultiMap)
			{
				this->Add(std::move(Pair.Key), std::move(Pair.Value));
			}

			OtherMultiMap.Reset();
		}
		void Append(const TMultiMap& OtherMultiMap)
		{
			this->Reserve(this->Num() + OtherMultiMap.Num());
			for (auto& Pair : OtherMultiMap)
			{
				this->Add(Pair.Key, Pair.Value);
			}
		}
	};
}