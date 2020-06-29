#pragma once
#include "CoreType.h"
#include "CoreConfig.h"
#include "Templates/Pair.h"
#include "Set.h"

// std::forward
namespace Fuko
{
	class FDefaultSetAllocator;
	
	template<typename KeyType, typename ValueType, bool bInAllowDuplicateKeys>
	struct TDefaultMapKeyFuncs;

	template<typename KeyType, typename ValueType, typename SetAllocator = FDefaultSetAllocator, typename KeyFuncs = TDefaultMapHashableKeyFuncs<KeyType,ValueType,false>>
	class TMap;
}

// KeyFunctions 
namespace Fuko
{
	template <typename KeyInitType, typename ValueInitType>
	class TPairInitializer
	{
	public:
		typename TRValueToLValueReference<KeyInitType  >::Type Key;
		typename TRValueToLValueReference<ValueInitType>::Type Value;

		FORCEINLINE TPairInitializer(KeyInitType InKey, ValueInitType InValue) : Key(InKey) , Value(InValue) { }

		/** Implicit conversion to pair initializer. */
		template <typename KeyType, typename ValueType>
		FORCEINLINE TPairInitializer(const TPair<KeyType, ValueType>& Pair) : Key(Pair.Key), Value(Pair.Value) { }

		template <typename KeyType, typename ValueType>
		operator TPair<KeyType, ValueType>() const
		{
			return TPair<KeyType, ValueType>(static_cast<KeyInitType>(Key), static_cast<ValueInitType>(Value));
		}
	};

	template <typename KeyInitType>
	class TKeyInitializer
	{
	public:
		typename TRValueToLValueReference<KeyInitType>::Type Key;
		FORCEINLINE explicit TKeyInitializer(KeyInitType InKey) : Key(InKey) { }

		template <typename KeyType, typename ValueType>
		operator TPair<KeyType, ValueType>() const
		{
			return TPair<KeyType, ValueType>(static_cast<KeyInitType>(Key), ValueType());
		}
	};

	template<typename KeyType, typename ValueType, bool bInAllowDuplicateKeys>
	struct TDefaultMapKeyFuncs : BaseKeyFuncs<TPair<KeyType, ValueType>, KeyType, bInAllowDuplicateKeys>
	{
		typedef typename TTypeTraits<KeyType>::ConstPointerType KeyInitType;
		typedef const TPairInitializer<typename TTypeTraits<KeyType>::ConstInitType, typename TTypeTraits<ValueType>::ConstInitType>& ElementInitType;

		static FORCEINLINE KeyInitType GetSetKey(ElementInitType Element)
		{
			return Element.Key;
		}

		static FORCEINLINE bool Matches(KeyInitType A, KeyInitType B)
		{
			return A == B;
		}

		template<typename ComparableKey>
		static FORCEINLINE bool Matches(KeyInitType A, ComparableKey B)
		{
			return A == B;
		}

		static FORCEINLINE uint32 GetKeyHash(KeyInitType Key)
		{
			return GetTypeHash(Key);
		}

		template<typename ComparableKey>
		static FORCEINLINE uint32 GetKeyHash(ComparableKey Key)
		{
			return GetTypeHash(Key);
		}
	};

	template<typename KeyType, typename ValueType, bool bInAllowDuplicateKeys>
	struct TDefaultMapHashableKeyFuncs : TDefaultMapKeyFuncs<KeyType, ValueType, bInAllowDuplicateKeys>
	{
		using HashabilityCheck = decltype(GetTypeHash(std::declval<const KeyType>()));
	};
}

// Base map
namespace Fuko
{
	template <typename KeyType, typename ValueType, typename SetAllocator, typename KeyFuncs>
	class TMapBase
	{
		template <typename OtherKeyType, typename OtherValueType, typename OtherSetAllocator, typename OtherKeyFuncs>
		friend class TMapBase;

		friend struct TContainerTraits<TMapBase>;
	public:
		typedef typename TTypeTraits<KeyType  >::ConstPointerType KeyConstPointerType;
		typedef typename TTypeTraits<KeyType  >::ConstInitType    KeyInitType;
		typedef typename TTypeTraits<ValueType>::ConstInitType    ValueInitType;
		typedef TPair<KeyType, ValueType> ElementType;

	protected:
		TMapBase() = default;
		TMapBase(TMapBase&&) = default;
		TMapBase(const TMapBase&) = default;
		TMapBase& operator=(TMapBase&&) = default;
		TMapBase& operator=(const TMapBase&) = default;

		/** Constructor for moving elements from a TMap with a different SetAllocator */
		template<typename OtherSetAllocator>
		TMapBase(TMapBase<KeyType, ValueType, OtherSetAllocator, KeyFuncs>&& Other)
			: Pairs(std::move(Other.Pairs))
		{ }

		/** Constructor for copying elements from a TMap with a different SetAllocator */
		template<typename OtherSetAllocator>
		TMapBase(const TMapBase<KeyType, ValueType, OtherSetAllocator, KeyFuncs>& Other)
			: Pairs(Other.Pairs)
		{ }

		/** Assignment operator for moving elements from a TMap with a different SetAllocator */
		template<typename OtherSetAllocator>
		TMapBase& operator=(TMapBase<KeyType, ValueType, OtherSetAllocator, KeyFuncs>&& Other)
		{
			Pairs = std::move(Other.Pairs);
			return *this;
		}

		/** Assignment operator for copying elements from a TMap with a different SetAllocator */
		template<typename OtherSetAllocator>
		TMapBase& operator=(const TMapBase<KeyType, ValueType, OtherSetAllocator, KeyFuncs>& Other)
		{
			Pairs = Other.Pairs;
			return *this;
		}

	public:
		// Legacy comparison operators.  Note that these also test whether the map's key-value pairs were added in the same order!
		friend bool LegacyCompareEqual(const TMapBase& A, const TMapBase& B)
		{
			return LegacyCompareEqual(A.Pairs, B.Pairs);
		}
		friend bool LegacyCompareNotEqual(const TMapBase& A, const TMapBase& B)
		{
			return LegacyCompareNotEqual(A.Pairs, B.Pairs);
		}

		/**
		 * Compare this map with another for equality. Does not make any assumptions about Key order.
		 * NOTE: this might be a candidate for operator== but it was decided to make it an explicit function
		 *  since it can potentially be quite slow.
		 *
		 * @param Other The other map to compare against
		 * @returns True if both this and Other contain the same keys with values that compare ==
		 */
		bool OrderIndependentCompareEqual(const TMapBase& Other) const
		{
			// first check counts (they should be the same obviously)
			if (Num() != Other.Num())
			{
				return false;
			}

			// since we know the counts are the same, we can just iterate one map and check for existence in the other
			for (typename ElementSetType::TConstIterator It(Pairs); It; ++It)
			{
				const ValueType* BVal = Other.Find(It->Key);
				if (BVal == nullptr)
				{
					return false;
				}
				if (!(*BVal == It->Value))
				{
					return false;
				}
			}

			// all fields in A match B and A and B's counts are the same (so there can be no fields in B not in A)
			return true;
		}

		/**
		 * Removes all elements from the map.
		 *
		 * This method potentially leaves space allocated for an expected
		 * number of elements about to be added.
		 *
		 * @param ExpectedNumElements The number of elements about to be added to the set.
		 */
		FORCEINLINE void Empty(int32 ExpectedNumElements = 0)
		{
			Pairs.Empty(ExpectedNumElements);
		}

		/** Efficiently empties out the map but preserves all allocations and capacities */
		FORCEINLINE void Reset()
		{
			Pairs.Reset();
		}

		/** Shrinks the pair set to avoid slack. */
		FORCEINLINE void Shrink()
		{
			Pairs.Shrink();
		}

		/** Compacts the pair set to remove holes */
		FORCEINLINE void Compact()
		{
			Pairs.Compact();
		}

		/** Compacts the pair set to remove holes. Does not change the iteration order of the elements. */
		FORCEINLINE void CompactStable()
		{
			Pairs.CompactStable();
		}

		/** Preallocates enough memory to contain Number elements */
		FORCEINLINE void Reserve(int32 Number)
		{
			Pairs.Reserve(Number);
		}

		/** @return The number of elements in the map. */
		FORCEINLINE int32 Num() const
		{
			return Pairs.Num();
		}

		/**
		 * Get the unique keys contained within this map.
		 *
		 * @param OutKeys Upon return, contains the set of unique keys in this map.
		 * @return The number of unique keys in the map.
		 */
		template<typename Allocator> int32 GetKeys(TArray<KeyType, Allocator>& OutKeys) const
		{
			TSet<KeyType> VisitedKeys;
			for (typename ElementSetType::TConstIterator It(Pairs); It; ++It)
			{
				if (!VisitedKeys.Contains(It->Key))
				{
					OutKeys.Add(It->Key);
					VisitedKeys.Add(It->Key);
				}
			}
			return OutKeys.Num();
		}

		/**
		 * Helper function to return the amount of memory allocated by this container .
		 * Only returns the size of allocations made directly by the container, not the elements themselves.
		 *
		 * @return Number of bytes allocated by this container.
		 * @see CountBytes
		 */
		FORCEINLINE uint32 GetAllocatedSize() const
		{
			return Pairs.GetAllocatedSize();
		}

		/**
		 * Set the value associated with a key.
		 *
		 * @param InKey The key to associate the value with.
		 * @param InValue The value to associate with the key.
		 * @return A reference to the value as stored in the map. The reference is only valid until the next change to any key in the map.
		 */
		FORCEINLINE ValueType& Add(const KeyType&  InKey, const ValueType&  InValue) { return Emplace(InKey, InValue); }
		FORCEINLINE ValueType& Add(const KeyType&  InKey, ValueType&& InValue) { return Emplace(InKey, std::move(InValue)); }
		FORCEINLINE ValueType& Add(KeyType&& InKey, const ValueType&  InValue) { return Emplace(std::move(InKey), InValue); }
		FORCEINLINE ValueType& Add(KeyType&& InKey, ValueType&& InValue) { return Emplace(std::move(InKey), std::move(InValue)); }

		/** See Add() and class documentation section on ByHash() functions */
		FORCEINLINE ValueType& AddByHash(uint32 KeyHash, const KeyType&  InKey, const ValueType&  InValue) { return EmplaceByHash(KeyHash, InKey, InValue); }
		FORCEINLINE ValueType& AddByHash(uint32 KeyHash, const KeyType&  InKey, ValueType&& InValue) { return EmplaceByHash(KeyHash, InKey, std::move(InValue)); }
		FORCEINLINE ValueType& AddByHash(uint32 KeyHash, KeyType&& InKey, const ValueType&  InValue) { return EmplaceByHash(KeyHash, std::move(InKey), InValue); }
		FORCEINLINE ValueType& AddByHash(uint32 KeyHash, KeyType&& InKey, ValueType&& InValue) { return EmplaceByHash(KeyHash, std::move(InKey), std::move(InValue)); }

		/**
		 * Set a default value associated with a key.
		 *
		 * @param InKey The key to associate the value with.
		 * @return A reference to the value as stored in the map. The reference is only valid until the next change to any key in the map.
		 */
		FORCEINLINE ValueType& Add(const KeyType&  InKey) { return Emplace(InKey); }
		FORCEINLINE ValueType& Add(KeyType&& InKey) { return Emplace(std::move(InKey)); }

		/** See Add() and class documentation section on ByHash() functions */
		FORCEINLINE ValueType& AddByHash(uint32 KeyHash, const KeyType&  InKey) { return EmplaceByHash(KeyHash, InKey); }
		FORCEINLINE ValueType& AddByHash(uint32 KeyHash, KeyType&& InKey) { return EmplaceByHash(KeyHash, std::move(InKey)); }

		/**
		 * Set the value associated with a key.
		 *
		 * @param InKeyValue A Tuple containing the Key and Value to associate together
		 * @return A reference to the value as stored in the map. The reference is only valid until the next change to any key in the map.
		 */
		FORCEINLINE ValueType& Add(const TTuple<KeyType, ValueType>&  InKeyValue) { return Emplace(InKeyValue.Key, InKeyValue.Value); }
		FORCEINLINE ValueType& Add(TTuple<KeyType, ValueType>&& InKeyValue) { return Emplace(std::move(InKeyValue.Key), std::move(InKeyValue.Value)); }

		/**
		 * Sets the value associated with a key.
		 *
		 * @param InKey The key to associate the value with.
		 * @param InValue The value to associate with the key.
		 * @return A reference to the value as stored in the map. The reference is only valid until the next change to any key in the map.	 */
		template <typename InitKeyType, typename InitValueType>
		ValueType& Emplace(InitKeyType&& InKey, InitValueType&& InValue)
		{
			const FSetElementId PairId = Pairs.Emplace(TPairInitializer<InitKeyType&&, InitValueType&&>(std::forward<InitKeyType>(InKey), std::forward<InitValueType>(InValue)));

			return Pairs[PairId].Value;
		}

		/** See Emplace() and class documentation section on ByHash() functions */
		template <typename InitKeyType, typename InitValueType>
		ValueType& EmplaceByHash(uint32 KeyHash, InitKeyType&& InKey, InitValueType&& InValue)
		{
			const FSetElementId PairId = Pairs.EmplaceByHash(KeyHash, TPairInitializer<InitKeyType&&, InitValueType&&>(std::forward<InitKeyType>(InKey), std::forward<InitValueType>(InValue)));

			return Pairs[PairId].Value;
		}

		/**
		 * Set a default value associated with a key.
		 *
		 * @param InKey The key to associate the value with.
		 * @return A reference to the value as stored in the map. The reference is only valid until the next change to any key in the map.
		 */
		template <typename InitKeyType>
		ValueType& Emplace(InitKeyType&& InKey)
		{
			const FSetElementId PairId = Pairs.Emplace(TKeyInitializer<InitKeyType&&>(std::forward<InitKeyType>(InKey)));

			return Pairs[PairId].Value;
		}

		/** See Emplace() and class documentation section on ByHash() functions */
		template <typename InitKeyType>
		ValueType& EmplaceByHash(uint32 KeyHash, InitKeyType&& InKey)
		{
			const FSetElementId PairId = Pairs.EmplaceByHash(KeyHash, TKeyInitializer<InitKeyType&&>(std::forward<InitKeyType>(InKey)));

			return Pairs[PairId].Value;
		}

		/**
		 * Remove all value associations for a key.
		 *
		 * @param InKey The key to remove associated values for.
		 * @return The number of values that were associated with the key.
		 */
		FORCEINLINE int32 Remove(KeyConstPointerType InKey)
		{
			const int32 NumRemovedPairs = Pairs.Remove(InKey);
			return NumRemovedPairs;
		}

		/** See Remove() and class documentation section on ByHash() functions */
		template<typename ComparableKey>
		FORCEINLINE int32 RemoveByHash(uint32 KeyHash, const ComparableKey& Key)
		{
			const int32 NumRemovedPairs = Pairs.RemoveByHash(KeyHash, Key);
			return NumRemovedPairs;
		}

		/**
		 * Find the key associated with the specified value.
		 *
		 * The time taken is O(N) in the number of pairs.
		 *
		 * @param Value The value to search for
		 * @return A pointer to the key associated with the specified value,
		 *     or nullptr if the value isn't contained in this map. The pointer
		 *     is only valid until the next change to any key in the map.
		 */
		const KeyType* FindKey(ValueInitType Value) const
		{
			for (typename ElementSetType::TConstIterator PairIt(Pairs); PairIt; ++PairIt)
			{
				if (PairIt->Value == Value)
				{
					return &PairIt->Key;
				}
			}
			return nullptr;
		}

		/**
		 * Find the value associated with a specified key.
		 *
		 * @param Key The key to search for.
		 * @return A pointer to the value associated with the specified key, or nullptr if the key isn't contained in this map.  The pointer
		 *			is only valid until the next change to any key in the map.
		 */
		FORCEINLINE ValueType* Find(KeyConstPointerType Key)
		{
			if (auto* Pair = Pairs.Find(Key))
			{
				return &Pair->Value;
			}

			return nullptr;
		}
		FORCEINLINE const ValueType* Find(KeyConstPointerType Key) const
		{
			return const_cast<TMapBase*>(this)->Find(Key);
		}

		/** See Find() and class documentation section on ByHash() functions */
		template<typename ComparableKey>
		FORCEINLINE ValueType* FindByHash(uint32 KeyHash, const ComparableKey& Key)
		{
			if (auto* Pair = Pairs.FindByHash(KeyHash, Key))
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

	private:
		FORCEINLINE static uint32 HashKey(const KeyType& Key)
		{
			return KeyFuncs::GetKeyHash(Key);
		}

		/**
		 * Find the value associated with a specified key, or if none exists,
		 * adds a value using the default constructor.
		 *
		 * @param Key The key to search for.
		 * @return A reference to the value associated with the specified key.
		 */
		template <typename InitKeyType>
		ValueType& FindOrAddImpl(uint32 KeyHash, InitKeyType&& Key)
		{
			if (auto* Pair = Pairs.FindByHash(KeyHash, Key))
			{
				return Pair->Value;
			}

			return AddByHash(KeyHash, std::forward<InitKeyType>(Key));
		}

		/**
		 * Find the value associated with a specified key, or if none exists,
		 * adds the value
		 *
		 * @param Key The key to search for.
		 * @param Value The value to associate with the key.
		 * @return A reference to the value associated with the specified key.
		 */
		template <typename InitKeyType, typename InitValueType>
		ValueType& FindOrAddImpl(uint32 KeyHash, InitKeyType&& Key, InitValueType&& Value)
		{
			if (auto* Pair = Pairs.FindByHash(KeyHash, Key))
			{
				return Pair->Value;
			}

			return AddByHash(KeyHash, std::forward<InitKeyType>(Key), std::forward<InitValueType>(Value));
		}

	public:

		/**
		 * Find the value associated with a specified key, or if none exists,
		 * adds a value using the default constructor.
		 *
		 * @param Key The key to search for.
		 * @return A reference to the value associated with the specified key.
		 */
		FORCEINLINE ValueType& FindOrAdd(const KeyType&  Key) { return FindOrAddImpl(HashKey(Key), Key); }
		FORCEINLINE ValueType& FindOrAdd(KeyType&& Key) { return FindOrAddImpl(HashKey(Key), std::moveIfPossible(Key)); }

		/** See FindOrAdd() and class documentation section on ByHash() functions */
		FORCEINLINE ValueType& FindOrAddByHash(uint32 KeyHash, const KeyType&  Key) { return FindOrAddImpl(KeyHash, Key); }
		FORCEINLINE ValueType& FindOrAddByHash(uint32 KeyHash, KeyType&& Key) { return FindOrAddImpl(KeyHash, std::moveIfPossible(Key)); }

		/**
		 * Find the value associated with a specified key, or if none exists,
		 * adds a value using the default constructor.
		 *
		 * @param Key The key to search for.
		 * @param Value The value to associate with the key.
		 * @return A reference to the value associated with the specified key.
		 */
		FORCEINLINE ValueType& FindOrAdd(const KeyType&  Key, const ValueType&  Value) { return FindOrAddImpl(HashKey(Key), Key, Value); }
		FORCEINLINE ValueType& FindOrAdd(const KeyType&  Key, ValueType&&       Value) { return FindOrAddImpl(HashKey(Key), Key, std::move(Value)); }
		FORCEINLINE ValueType& FindOrAdd(KeyType&& Key, const ValueType&  Value) { return FindOrAddImpl(HashKey(Key), std::move(Key), Value); }
		FORCEINLINE ValueType& FindOrAdd(KeyType&& Key, ValueType&&       Value) { return FindOrAddImpl(HashKey(Key), std::move(Key), std::move(Value)); }

		/** See FindOrAdd() and class documentation section on ByHash() functions */
		FORCEINLINE ValueType& FindOrAddByHash(uint32 KeyHash, const KeyType&  Key, const ValueType&  Value) { return FindOrAddImpl(KeyHash, Key, Value); }
		FORCEINLINE ValueType& FindOrAddByHash(uint32 KeyHash, const KeyType&  Key, ValueType&& Value) { return FindOrAddImpl(KeyHash, Key, std::move(Value)); }
		FORCEINLINE ValueType& FindOrAddByHash(uint32 KeyHash, KeyType&& Key, const ValueType&  Value) { return FindOrAddImpl(KeyHash, std::move(Key), Value); }
		FORCEINLINE ValueType& FindOrAddByHash(uint32 KeyHash, KeyType&& Key, ValueType&& Value) { return FindOrAddImpl(KeyHash, std::move(Key), std::move(Value)); }

		/**
		 * Find a reference to the value associated with a specified key.
		 *
		 * @param Key The key to search for.
		 * @return The value associated with the specified key, or triggers an assertion if the key does not exist.
		 */
		FORCEINLINE const ValueType& FindChecked(KeyConstPointerType Key) const
		{
			const auto* Pair = Pairs.Find(Key);
			check(Pair != nullptr);
			return Pair->Value;
		}

		/**
		 * Find a reference to the value associated with a specified key.
		 *
		 * @param Key The key to search for.
		 * @return The value associated with the specified key, or triggers an assertion if the key does not exist.
		 */
		FORCEINLINE ValueType& FindChecked(KeyConstPointerType Key)
		{
			auto* Pair = Pairs.Find(Key);
			check(Pair != nullptr);
			return Pair->Value;
		}

		/**
		 * Find the value associated with a specified key.
		 *
		 * @param Key The key to search for.
		 * @return The value associated with the specified key, or the default value for the ValueType if the key isn't contained in this map.
		 */
		FORCEINLINE ValueType FindRef(KeyConstPointerType Key) const
		{
			if (const auto* Pair = Pairs.Find(Key))
			{
				return Pair->Value;
			}

			return ValueType();
		}

		/**
		 * Check if map contains the specified key.
		 *
		 * @param Key The key to check for.
		 * @return true if the map contains the key.
		 */
		FORCEINLINE bool Contains(KeyConstPointerType Key) const
		{
			return Pairs.Contains(Key);
		}

		/** See Contains() and class documentation section on ByHash() functions */
		template<typename ComparableKey>
		FORCEINLINE bool ContainsByHash(uint32 KeyHash, const ComparableKey& Key) const
		{
			return Pairs.ContainsByHash(KeyHash, Key);
		}

		/**
		 * Generate an array from the keys in this map.
		 *
		 * @param OutArray Will contain the collection of keys.
		 */
		template<typename Allocator> void GenerateKeyArray(TArray<KeyType, Allocator>& OutArray) const
		{
			OutArray.Empty(Pairs.Num());
			for (typename ElementSetType::TConstIterator PairIt(Pairs); PairIt; ++PairIt)
			{
				new(OutArray) KeyType(PairIt->Key);
			}
		}

		/**
		 * Generate an array from the values in this map.
		 *
		 * @param OutArray Will contain the collection of values.
		 */
		template<typename Allocator> void GenerateValueArray(TArray<ValueType, Allocator>& OutArray) const
		{
			OutArray.Empty(Pairs.Num());
			for (typename ElementSetType::TConstIterator PairIt(Pairs); PairIt; ++PairIt)
			{
				new(OutArray) ValueType(PairIt->Value);
			}
		}

	protected:
		typedef TSet<ElementType, KeyFuncs, SetAllocator> ElementSetType;

		/** The base of TMapBase iterators. */
		template<bool bConst, bool bRangedFor = false>
		class TBaseIterator
		{
		public:
			typedef typename std::conditional_t<
				bConst,
				typename std::conditional_t<bRangedFor, typename ElementSetType::TRangedForConstIterator, typename ElementSetType::TConstIterator>,
				typename std::conditional_t<bRangedFor, typename ElementSetType::TRangedForIterator, typename ElementSetType::TIterator     >
			> PairItType;
		private:
			typedef typename std::conditional_t<bConst, const TMapBase, TMapBase> MapType;
			typedef typename std::conditional_t<bConst, const KeyType, KeyType> ItKeyType;
			typedef typename std::conditional_t<bConst, const ValueType, ValueType> ItValueType;
			typedef typename std::conditional_t<bConst, const typename ElementSetType::ElementType, typename ElementSetType::ElementType> PairType;

		public:
			FORCEINLINE TBaseIterator(const PairItType& InElementIt)
				: PairIt(InElementIt)
			{
			}

			FORCEINLINE TBaseIterator& operator++()
			{
				++PairIt;
				return *this;
			}

			/** conversion to "bool" returning true if the iterator is valid. */
			FORCEINLINE explicit operator bool() const
			{
				return !!PairIt;
			}
			/** inverse of the "bool" operator */
			FORCEINLINE bool operator !() const
			{
				return !(bool)*this;
			}

			FORCEINLINE friend bool operator==(const TBaseIterator& Lhs, const TBaseIterator& Rhs) { return Lhs.PairIt == Rhs.PairIt; }
			FORCEINLINE friend bool operator!=(const TBaseIterator& Lhs, const TBaseIterator& Rhs) { return Lhs.PairIt != Rhs.PairIt; }

			FORCEINLINE ItKeyType&   Key()   const { return PairIt->Key; }
			FORCEINLINE ItValueType& Value() const { return PairIt->Value; }

			FORCEINLINE PairType& operator* () const { return  *PairIt; }
			FORCEINLINE PairType* operator->() const { return &*PairIt; }

		protected:
			PairItType PairIt;
		};

		/** The base type of iterators that iterate over the values associated with a specified key. */
		template<bool bConst>
		class TBaseKeyIterator
		{
		private:
			typedef typename std::conditional_t<bConst, typename ElementSetType::TConstKeyIterator, typename ElementSetType::TKeyIterator> SetItType;
			typedef typename std::conditional_t<bConst, const KeyType, KeyType> ItKeyType;
			typedef typename std::conditional_t<bConst, const ValueType, ValueType> ItValueType;

		public:
			/** Initialization constructor. */
			FORCEINLINE TBaseKeyIterator(const SetItType& InSetIt)
				: SetIt(InSetIt)
			{
			}

			FORCEINLINE TBaseKeyIterator& operator++()
			{
				++SetIt;
				return *this;
			}

			/** conversion to "bool" returning true if the iterator is valid. */
			FORCEINLINE explicit operator bool() const
			{
				return !!SetIt;
			}
			/** inverse of the "bool" operator */
			FORCEINLINE bool operator !() const
			{
				return !(bool)*this;
			}

			FORCEINLINE ItKeyType&   Key() const { return SetIt->Key; }
			FORCEINLINE ItValueType& Value() const { return SetIt->Value; }

		protected:
			SetItType SetIt;
		};

		/** A set of the key-value pairs in the map. */
		ElementSetType Pairs;

	public:
		/** Map iterator. */
		class TIterator : public TBaseIterator<false>
		{
		public:

			/** Initialization constructor. */
			FORCEINLINE TIterator(TMapBase& InMap, bool bInRequiresRehashOnRemoval = false)
				: TBaseIterator<false>(InMap.Pairs.CreateIterator())
				, Map(InMap)
				, bElementsHaveBeenRemoved(false)
				, bRequiresRehashOnRemoval(bInRequiresRehashOnRemoval)
			{
			}

			/** Destructor. */
			FORCEINLINE ~TIterator()
			{
				if (bElementsHaveBeenRemoved && bRequiresRehashOnRemoval)
				{
					Map.Pairs.Relax();
				}
			}

			/** Removes the current pair from the map. */
			FORCEINLINE void RemoveCurrent()
			{
				TBaseIterator<false>::PairIt.RemoveCurrent();
				bElementsHaveBeenRemoved = true;
			}

		private:
			TMapBase& Map;
			bool      bElementsHaveBeenRemoved;
			bool      bRequiresRehashOnRemoval;
		};

		/** Const map iterator. */
		class TConstIterator : public TBaseIterator<true>
		{
		public:
			FORCEINLINE TConstIterator(const TMapBase& InMap)
				: TBaseIterator<true>(InMap.Pairs.CreateConstIterator())
			{
			}
		};

		using TRangedForIterator = TBaseIterator<false, true>;
		using TRangedForConstIterator = TBaseIterator<true, true>;

		/** Iterates over values associated with a specified key in a const map. */
		class TConstKeyIterator : public TBaseKeyIterator<true>
		{
		public:
			FORCEINLINE TConstKeyIterator(const TMapBase& InMap, KeyInitType InKey)
				: TBaseKeyIterator<true>(typename ElementSetType::TConstKeyIterator(InMap.Pairs, InKey))
			{}
		};

		/** Iterates over values associated with a specified key in a map. */
		class TKeyIterator : public TBaseKeyIterator<false>
		{
		public:
			FORCEINLINE TKeyIterator(TMapBase& InMap, KeyInitType InKey)
				: TBaseKeyIterator<false>(typename ElementSetType::TKeyIterator(InMap.Pairs, InKey))
			{}

			/** Removes the current key-value pair from the map. */
			FORCEINLINE void RemoveCurrent()
			{
				TBaseKeyIterator<false>::SetIt.RemoveCurrent();
			}
		};

		/** Creates an iterator over all the pairs in this map */
		FORCEINLINE TIterator CreateIterator()
		{
			return TIterator(*this);
		}

		/** Creates a const iterator over all the pairs in this map */
		FORCEINLINE TConstIterator CreateConstIterator() const
		{
			return TConstIterator(*this);
		}

		/** Creates an iterator over the values associated with a specified key in a map */
		FORCEINLINE TKeyIterator CreateKeyIterator(KeyInitType InKey)
		{
			return TKeyIterator(*this, InKey);
		}

		/** Creates a const iterator over the values associated with a specified key in a map */
		FORCEINLINE TConstKeyIterator CreateConstKeyIterator(KeyInitType InKey) const
		{
			return TConstKeyIterator(*this, InKey);
		}

	public:
		// Support foreach 
		FORCEINLINE TRangedForIterator      begin() { return TRangedForIterator(Pairs.begin()); }
		FORCEINLINE TRangedForConstIterator begin() const { return TRangedForConstIterator(Pairs.begin()); }
		FORCEINLINE TRangedForIterator      end() { return TRangedForIterator(Pairs.end()); }
		FORCEINLINE TRangedForConstIterator end() const { return TRangedForConstIterator(Pairs.end()); }
	};
}

// SortableMap
namespace Fuko
{
	template <typename KeyType, typename ValueType, typename SetAllocator, typename KeyFuncs>
	class TSortableMapBase : public TMapBase<KeyType, ValueType, SetAllocator, KeyFuncs>
	{
		friend struct TContainerTraits<TSortableMapBase>;

	protected:
		typedef TMapBase<KeyType, ValueType, SetAllocator, KeyFuncs> Super;

		TSortableMapBase() = default;
		TSortableMapBase(TSortableMapBase&&) = default;
		TSortableMapBase(const TSortableMapBase&) = default;
		TSortableMapBase& operator=(TSortableMapBase&&) = default;
		TSortableMapBase& operator=(const TSortableMapBase&) = default;

		/** Constructor for moving elements from a TMap with a different SetAllocator */
		template<typename OtherSetAllocator>
		TSortableMapBase(TSortableMapBase<KeyType, ValueType, OtherSetAllocator, KeyFuncs>&& Other)
			: Super(std::move(Other))
		{
		}

		/** Constructor for copying elements from a TMap with a different SetAllocator */
		template<typename OtherSetAllocator>
		TSortableMapBase(const TSortableMapBase<KeyType, ValueType, OtherSetAllocator, KeyFuncs>& Other)
			: Super(Other)
		{
		}

		/** Assignment operator for moving elements from a TMap with a different SetAllocator */
		template<typename OtherSetAllocator>
		TSortableMapBase& operator=(TSortableMapBase<KeyType, ValueType, OtherSetAllocator, KeyFuncs>&& Other)
		{
			(Super&)*this = std::move(Other);
			return *this;
		}

		/** Assignment operator for copying elements from a TMap with a different SetAllocator */
		template<typename OtherSetAllocator>
		TSortableMapBase& operator=(const TSortableMapBase<KeyType, ValueType, OtherSetAllocator, KeyFuncs>& Other)
		{
			(Super&)*this = Other;
			return *this;
		}

	public:
		/**
		 * Sorts the pairs array using each pair's Key as the sort criteria, then rebuilds the map's hash.
		 * Invoked using "MyMapVar.KeySort( PREDICATE_CLASS() );"
		 */
		template<typename PREDICATE_CLASS>
		FORCEINLINE void KeySort(const PREDICATE_CLASS& Predicate)
		{
			Super::Pairs.Sort(FKeyComparisonClass<PREDICATE_CLASS>(Predicate));
		}

		/**
		 * Stable sorts the pairs array using each pair's Key as the sort criteria, then rebuilds the map's hash.
		 * Invoked using "MyMapVar.KeySort( PREDICATE_CLASS() );"
		 */
		template<typename PREDICATE_CLASS>
		FORCEINLINE void KeyStableSort(const PREDICATE_CLASS& Predicate)
		{
			Super::Pairs.StableSort(FKeyComparisonClass<PREDICATE_CLASS>(Predicate));
		}

		/**
		 * Sorts the pairs array using each pair's Value as the sort criteria, then rebuilds the map's hash.
		 * Invoked using "MyMapVar.ValueSort( PREDICATE_CLASS() );"
		 */
		template<typename PREDICATE_CLASS>
		FORCEINLINE void ValueSort(const PREDICATE_CLASS& Predicate)
		{
			Super::Pairs.Sort(FValueComparisonClass<PREDICATE_CLASS>(Predicate));
		}

		/**
		 * Stable sorts the pairs array using each pair's Value as the sort criteria, then rebuilds the map's hash.
		 * Invoked using "MyMapVar.ValueSort( PREDICATE_CLASS() );"
		 */
		template<typename PREDICATE_CLASS>
		FORCEINLINE void ValueStableSort(const PREDICATE_CLASS& Predicate)
		{
			Super::Pairs.StableSort(FValueComparisonClass<PREDICATE_CLASS>(Predicate));
		}


	private:

		/** Extracts the pair's key from the map's pair structure and passes it to the user provided comparison class. */
		template<typename PREDICATE_CLASS>
		class FKeyComparisonClass
		{
			TDereferenceWrapper< KeyType, PREDICATE_CLASS> Predicate;

		public:

			FORCEINLINE FKeyComparisonClass(const PREDICATE_CLASS& InPredicate)
				: Predicate(InPredicate)
			{}

			FORCEINLINE bool operator()(const typename Super::ElementType& A, const typename Super::ElementType& B) const
			{
				return Predicate(A.Key, B.Key);
			}
		};

		/** Extracts the pair's value from the map's pair structure and passes it to the user provided comparison class. */
		template<typename PREDICATE_CLASS>
		class FValueComparisonClass
		{
			TDereferenceWrapper< ValueType, PREDICATE_CLASS> Predicate;

		public:

			FORCEINLINE FValueComparisonClass(const PREDICATE_CLASS& InPredicate)
				: Predicate(InPredicate)
			{}

			FORCEINLINE bool operator()(const typename Super::ElementType& A, const typename Super::ElementType& B) const
			{
				return Predicate(A.Value, B.Value);
			}
		};
	};
}

// Signal map
namespace Fuko
{
	template<typename KeyType, typename ValueType, typename SetAllocator, typename KeyFuncs>
	class TMap : public TSortableMapBase<KeyType, ValueType, SetAllocator, KeyFuncs>
	{
		friend struct TContainerTraits<TMap>;

		template <typename, typename>
		friend class TScriptMap;

		static_assert(!KeyFuncs::bAllowDuplicateKeys, "TMap cannot be instantiated with a KeyFuncs which allows duplicate keys");

	public:
		typedef TSortableMapBase<KeyType, ValueType, SetAllocator, KeyFuncs> Super;
		typedef typename Super::KeyInitType KeyInitType;
		typedef typename Super::KeyConstPointerType KeyConstPointerType;

		TMap() = default;
		TMap(TMap&&) = default;
		TMap(const TMap&) = default;
		TMap& operator=(TMap&&) = default;
		TMap& operator=(const TMap&) = default;

		/** Constructor for moving elements from a TMap with a different SetAllocator */
		template<typename OtherSetAllocator>
		TMap(TMap<KeyType, ValueType, OtherSetAllocator, KeyFuncs>&& Other)
			: Super(std::move(Other))
		{
		}

		/** Constructor for copying elements from a TMap with a different SetAllocator */
		template<typename OtherSetAllocator>
		TMap(const TMap<KeyType, ValueType, OtherSetAllocator, KeyFuncs>& Other)
			: Super(Other)
		{
		}

		/** Constructor which gets its elements from a native initializer list */
		TMap(std::initializer_list<TPairInitializer<const KeyType&, const ValueType&>> InitList)
		{
			this->Reserve((int32)InitList.size());
			for (const TPairInitializer<const KeyType&, const ValueType&>& Element : InitList)
			{
				this->Add(Element.Key, Element.Value);
			}
		}

		/** Assignment operator for moving elements from a TMap with a different SetAllocator */
		template<typename OtherSetAllocator>
		TMap& operator=(TMap<KeyType, ValueType, OtherSetAllocator, KeyFuncs>&& Other)
		{
			(Super&)*this = std::move(Other);
			return *this;
		}

		/** Assignment operator for copying elements from a TMap with a different SetAllocator */
		template<typename OtherSetAllocator>
		TMap& operator=(const TMap<KeyType, ValueType, OtherSetAllocator, KeyFuncs>& Other)
		{
			(Super&)*this = Other;
			return *this;
		}

		/** Assignment operator which gets its elements from a native initializer list */
		TMap& operator=(std::initializer_list<TPairInitializer<const KeyType&, const ValueType&>> InitList)
		{
			this->Empty((int32)InitList.size());
			for (const TPairInitializer<const KeyType&, const ValueType&>& Element : InitList)
			{
				this->Add(Element.Key, Element.Value);
			}
			return *this;
		}

		/**
		 * Remove the pair with the specified key and copies the value
		 * that was removed to the ref parameter
		 *
		 * @param Key The key to search for
		 * @param OutRemovedValue If found, the value that was removed (not modified if the key was not found)
		 * @return whether or not the key was found
		 */
		FORCEINLINE bool RemoveAndCopyValue(KeyInitType Key, ValueType& OutRemovedValue)
		{
			const FSetElementId PairId = Super::Pairs.FindId(Key);
			if (!PairId.IsValidId())
				return false;

			OutRemovedValue = std::moveIfPossible(Super::Pairs[PairId].Value);
			Super::Pairs.Remove(PairId);
			return true;
		}

		/**
		 * Find a pair with the specified key, removes it from the map, and returns the value part of the pair.
		 *
		 * If no pair was found, an exception is thrown.
		 *
		 * @param Key the key to search for
		 * @return whether or not the key was found
		 */
		FORCEINLINE ValueType FindAndRemoveChecked(KeyConstPointerType Key)
		{
			const FSetElementId PairId = Super::Pairs.FindId(Key);
			check(PairId.IsValidId());
			ValueType Result = std::moveIfPossible(Super::Pairs[PairId].Value);
			Super::Pairs.Remove(PairId);
			return Result;
		}

		/**
		 * Move all items from another map into our map (if any keys are in both,
		 * the value from the other map wins) and empty the other map.
		 *
		 * @param OtherMap The other map of items to move the elements from.
		 */
		template<typename OtherSetAllocator>
		void Append(TMap<KeyType, ValueType, OtherSetAllocator, KeyFuncs>&& OtherMap)
		{
			this->Reserve(this->Num() + OtherMap.Num());
			for (auto& Pair : OtherMap)
			{
				this->Add(std::moveIfPossible(Pair.Key), std::moveIfPossible(Pair.Value));
			}

			OtherMap.Reset();
		}

		/**
		 * Add all items from another map to our map (if any keys are in both,
		 * the value from the other map wins).
		 *
		 * @param OtherMap The other map of items to add.
		 */
		template<typename OtherSetAllocator>
		void Append(const TMap<KeyType, ValueType, OtherSetAllocator, KeyFuncs>& OtherMap)
		{
			this->Reserve(this->Num() + OtherMap.Num());
			for (auto& Pair : OtherMap)
			{
				this->Add(Pair.Key, Pair.Value);
			}
		}

		FORCEINLINE       ValueType& operator[](KeyConstPointerType Key) { return this->FindChecked(Key); }
		FORCEINLINE const ValueType& operator[](KeyConstPointerType Key) const { return this->FindChecked(Key); }
	};
}

// Multi map
namespace Fuko
{
	template<typename KeyType, typename ValueType, typename SetAllocator /* = FDefaultSetAllocator */, typename KeyFuncs /*= TDefaultMapHashableKeyFuncs<KeyType,ValueType,true>*/>
	class TMultiMap : public TSortableMapBase<KeyType, ValueType, SetAllocator, KeyFuncs>
	{
		friend struct TContainerTraits<TMultiMap>;

		static_assert(KeyFuncs::bAllowDuplicateKeys, "TMultiMap cannot be instantiated with a KeyFuncs which disallows duplicate keys");

	public:
		typedef TSortableMapBase<KeyType, ValueType, SetAllocator, KeyFuncs> Super;
		typedef typename Super::KeyConstPointerType KeyConstPointerType;
		typedef typename Super::KeyInitType KeyInitType;
		typedef typename Super::ValueInitType ValueInitType;

		TMultiMap() = default;
		TMultiMap(TMultiMap&&) = default;
		TMultiMap(const TMultiMap&) = default;
		TMultiMap& operator=(TMultiMap&&) = default;
		TMultiMap& operator=(const TMultiMap&) = default;

		/** Constructor for moving elements from a TMap with a different SetAllocator */
		template<typename OtherSetAllocator>
		TMultiMap(TMultiMap<KeyType, ValueType, OtherSetAllocator, KeyFuncs>&& Other)
			: Super(std::move(Other))
		{
		}

		/** Constructor for copying elements from a TMap with a different SetAllocator */
		template<typename OtherSetAllocator>
		TMultiMap(const TMultiMap<KeyType, ValueType, OtherSetAllocator, KeyFuncs>& Other)
			: Super(Other)
		{
		}

		/** Constructor which gets its elements from a native initializer list */
		TMultiMap(std::initializer_list<TPairInitializer<const KeyType&, const ValueType&>> InitList)
		{
			this->Reserve((int32)InitList.size());
			for (const TPairInitializer<const KeyType&, const ValueType&>& Element : InitList)
			{
				this->Add(Element.Key, Element.Value);
			}
		}

		/** Assignment operator for moving elements from a TMap with a different SetAllocator */
		template<typename OtherSetAllocator>
		TMultiMap& operator=(TMultiMap<KeyType, ValueType, OtherSetAllocator, KeyFuncs>&& Other)
		{
			(Super&)*this = std::move(Other);
			return *this;
		}

		/** Assignment operator for copying elements from a TMap with a different SetAllocator */
		template<typename OtherSetAllocator>
		TMultiMap& operator=(const TMultiMap<KeyType, ValueType, OtherSetAllocator, KeyFuncs>& Other)
		{
			(Super&)*this = Other;
			return *this;
		}

		/** Assignment operator which gets its elements from a native initializer list */
		TMultiMap& operator=(std::initializer_list<TPairInitializer<const KeyType&, const ValueType&>> InitList)
		{
			this->Empty((int32)InitList.size());
			for (const TPairInitializer<const KeyType&, const ValueType&>& Element : InitList)
			{
				this->Add(Element.Key, Element.Value);
			}
			return *this;
		}

		/**
		 * Finds all values associated with the specified key.
		 *
		 * @param Key The key to find associated values for.
		 * @param OutValues Upon return, contains the values associated with the key.
		 * @param bMaintainOrder true if the Values array should be in the same order as the map's pairs.
		 */
		template<typename Allocator> void MultiFind(KeyInitType Key, TArray<ValueType, Allocator>& OutValues, bool bMaintainOrder = false) const
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

		/**
		 * Finds all values associated with the specified key.
		 *
		 * @param Key The key to find associated values for.
		 * @param OutValues Upon return, contains pointers to the values associated with the key.
		 *					Pointers are only valid until the next change to any key in the map.
		 * @param bMaintainOrder true if the Values array should be in the same order as the map's pairs.
		 */
		template<typename Allocator> void MultiFindPointer(KeyInitType Key, TArray<const ValueType*, Allocator>& OutValues, bool bMaintainOrder = false) const
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
		template<typename Allocator> void MultiFindPointer(KeyInitType Key, TArray<ValueType*, Allocator>& OutValues, bool bMaintainOrder = false)
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

		/**
		 * Add a key-value association to the map.  The association doesn't replace any of the key's existing associations.
		 * However, if both the key and value match an existing association in the map, no new association is made and the existing association's
		 * value is returned.
		 *
		 * @param InKey The key to associate.
		 * @param InValue The value to associate.
		 * @return A reference to the value as stored in the map; the reference is only valid until the next change to any key in the map.
		 */
		FORCEINLINE ValueType& AddUnique(const KeyType&  InKey, const ValueType&  InValue) { return EmplaceUnique(InKey, InValue); }
		FORCEINLINE ValueType& AddUnique(const KeyType&  InKey, ValueType&& InValue) { return EmplaceUnique(InKey, std::moveIfPossible(InValue)); }
		FORCEINLINE ValueType& AddUnique(KeyType&& InKey, const ValueType&  InValue) { return EmplaceUnique(std::moveIfPossible(InKey), InValue); }
		FORCEINLINE ValueType& AddUnique(KeyType&& InKey, ValueType&& InValue) { return EmplaceUnique(std::moveIfPossible(InKey), std::moveIfPossible(InValue)); }

		/**
		 * Add a key-value association to the map.
		 *
		 * The association doesn't replace any of the key's existing associations.
		 * However, if both key and value match an existing association in the map,
		 * no new association is made and the existing association's value is returned.
		 *
		 * @param InKey The key to associate.
		 * @param InValue The value to associate.
		 * @return A reference to the value as stored in the map; the reference is only valid until the next change to any key in the map.
		 */
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

		/**
		 * Remove all value associations for a key.
		 *
		 * @param InKey The key to remove associated values for.
		 * @return The number of values that were associated with the key.
		 */
		FORCEINLINE int32 Remove(KeyConstPointerType InKey)
		{
			return Super::Remove(InKey);
		}

		/**
		 * Remove associations between the specified key and value from the map.
		 *
		 * @param InKey The key part of the pair to remove.
		 * @param InValue The value part of the pair to remove.
		 * @return The number of associations removed.
		 */
		int32 Remove(KeyInitType InKey, ValueInitType InValue)
		{
			// Iterate over pairs with a matching key.
			int32 NumRemovedPairs = 0;
			for (typename Super::ElementSetType::TKeyIterator It(Super::Pairs, InKey); It; ++It)
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

		/**
		 * Remove the first association between the specified key and value from the map.
		 *
		 * @param InKey The key part of the pair to remove.
		 * @param InValue The value part of the pair to remove.
		 * @return The number of associations removed.
		 */
		int32 RemoveSingle(KeyInitType InKey, ValueInitType InValue)
		{
			// Iterate over pairs with a matching key.
			int32 NumRemovedPairs = 0;
			for (typename Super::ElementSetType::TKeyIterator It(Super::Pairs, InKey); It; ++It)
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

		/**
		 * Find an association between a specified key and value. (const)
		 *
		 * @param Key The key to find.
		 * @param Value The value to find.
		 * @return If the map contains a matching association, a pointer to the value in the map is returned.  Otherwise nullptr is returned.
		 *			The pointer is only valid as long as the map isn't changed.
		 */
		FORCEINLINE const ValueType* FindPair(KeyInitType Key, ValueInitType Value) const
		{
			return const_cast<TMultiMap*>(this)->FindPair(Key, Value);
		}

		/**
		 * Find an association between a specified key and value.
		 *
		 * @param Key The key to find.
		 * @param Value The value to find.
		 * @return If the map contains a matching association, a pointer to the value in the map is returned.  Otherwise nullptr is returned.
		 *			The pointer is only valid as long as the map isn't changed.
		 */
		ValueType* FindPair(KeyInitType Key, ValueInitType Value)
		{
			// Iterate over pairs with a matching key.
			for (typename Super::ElementSetType::TKeyIterator It(Super::Pairs, Key); It; ++It)
			{
				// If the pair's value matches, return a pointer to it.
				if (It->Value == Value)
				{
					return &It->Value;
				}
			}

			return nullptr;
		}

		/** Returns the number of values within this map associated with the specified key */
		int32 Num(KeyInitType Key) const
		{
			// Iterate over pairs with a matching key.
			int32 NumMatchingPairs = 0;
			for (typename Super::ElementSetType::TConstKeyIterator It(Super::Pairs, Key); It; ++It)
			{
				++NumMatchingPairs;
			}
			return NumMatchingPairs;
		}

		// Since we implement an overloaded Num() function in TMultiMap, we need to reimplement TMapBase::Num to make it visible.
		FORCEINLINE int32 Num() const
		{
			return Super::Num();
		}

		/**
		 * Move all items from another map into our map (if any keys are in both,
		 * the value from the other map wins) and empty the other map.
		 *
		 * @param OtherMultiMap The other map of items to move the elements from.
		 */
		template<typename OtherSetAllocator>
		void Append(TMultiMap<KeyType, ValueType, OtherSetAllocator, KeyFuncs>&& OtherMultiMap)
		{
			this->Reserve(this->Num() + OtherMultiMap.Num());
			for (auto& Pair : OtherMultiMap)
			{
				this->Add(std::moveIfPossible(Pair.Key), std::moveIfPossible(Pair.Value));
			}

			OtherMultiMap.Reset();
		}

		/**
		 * Add all items from another map to our map (if any keys are in both,
		 * the value from the other map wins).
		 *
		 * @param OtherMultiMap The other map of items to add.
		 */
		template<typename OtherSetAllocator>
		void Append(const TMultiMap<KeyType, ValueType, OtherSetAllocator, KeyFuncs>& OtherMultiMap)
		{
			this->Reserve(this->Num() + OtherMultiMap.Num());
			for (auto& Pair : OtherMultiMap)
			{
				this->Add(Pair.Key, Pair.Value);
			}
		}
	};

}

// ContainerTraits 
namespace Fuko
{
	template <typename KeyType, typename ValueType, typename SetAllocator, typename KeyFuncs>
	struct TContainerTraits<TMap<KeyType, ValueType, SetAllocator, KeyFuncs>> : public TContainerTraitsBase<TMap<KeyType, ValueType, SetAllocator, KeyFuncs>>
	{
		enum { MoveWillEmptyContainer = TContainerTraits<typename TMap<KeyType, ValueType, SetAllocator, KeyFuncs>::ElementSetType>::MoveWillEmptyContainer };
	};


	template <typename KeyType, typename ValueType, typename SetAllocator, typename KeyFuncs>
	struct TContainerTraits<TMultiMap<KeyType, ValueType, SetAllocator, KeyFuncs>> : public TContainerTraitsBase<TMultiMap<KeyType, ValueType, SetAllocator, KeyFuncs>>
	{
		enum { MoveWillEmptyContainer = TContainerTraits<typename TMultiMap<KeyType, ValueType, SetAllocator, KeyFuncs>::ElementSetType>::MoveWillEmptyContainer };
	};

}