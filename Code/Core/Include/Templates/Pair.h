#pragma once
#include <CoreType.h>
#include <CoreConfig.h>
#include "Tuple.h"

// implement TPair
namespace Fuko
{
	template<typename KeyType, typename ValueType>
	struct TPair
	{
		// construct
		TPair() = default;
		TPair(KeyType&& InKey, ValueType&& InValue) :Key(std::forward<KeyType>(InKey)), Value(std::forward<ValueType>(InValue)) {}
		TPair(const KeyType& InKey, const ValueType& InValue) :Key(InKey), Value(InValue) {}
		TPair(KeyType&& InKey) : Key(std::forward<KeyType>(InKey)), Value() {}
		TPair(const KeyType& InKey) : Key(InKey), Value() {}

		// copy & move
		TPair(const TPair&) = default;
		TPair(TPair&&) = default;
		
		// assign
		TPair& operator=(const TPair&) = default;
		TPair& operator=(TPair&&) = default;

		// compare
		bool operator==(const TPair& Other) const { return Key == Other.Key && Value == Other.Value; }
		bool operator!=(const TPair& Other) const { return !(*this == Other); }
		bool operator<(const TPair& Other) const { return Key < Other.Key && Value < Other.Value; }
		bool operator<=(const TPair& Other) const { return !(Other < *this); }
		bool operator>(const TPair& Other) const { return Other < *this; }
		bool operator>=(const TPair& Other) const { return !(*this < Other); }

		// tuple
		operator TTuple<KeyType, ValueType>()
		{
			return TTuple<KeyType, ValueType>(Key, Value);
		}

		KeyType			Key;
		ValueType		Value;
	};
}

// implement MakePair()
namespace Fuko
{
	template<typename TKey,typename TValue>
	FORCEINLINE TPair<TKey, TValue> MakePair(TKey&& Key, TValue&& Value)
	{
		return TPair<TKey, TValue>(std::forward<TKey>(Key), std::forward<TValue>(Value));
	}
}

// support structure binding
namespace std
{
	template<size_t Index,typename TKey,typename TValue>
	struct tuple_element<Index, Fuko::TPair<TKey, TValue>>
	{
		static_assert(Index == 0 || Index == 1, "TPair out of range");
	};

	template<typename TKey,typename TValue>
	struct tuple_element<0, Fuko::TPair<TKey, TValue>>
	{
		using type = TKey;
	};

	template<typename TKey, typename TValue>
	struct tuple_element<1, Fuko::TPair<TKey, TValue>>
	{
		using type = TValue;
	};

	template<typename TKey, typename TValue>
	struct tuple_size<Fuko::TPair<TKey, TValue>>
	{
		static constexpr size_t value = 2;
	};

	template<size_t Index, typename TKey, typename TValue>
	FORCEINLINE decltype(auto) get(const Fuko::TPair<TKey, TValue>& Pair)
	{
		if constexpr (Index == 0)
		{
			return static_cast<const TKey&>(Pair.Key);
		}
		else if constexpr (Index == 1)
		{
			return static_cast<const TValue&>(Pair.Value);
		}
		else
		{
			static_assert(false, "TPair out of range!!!");
		}
	}

	template<size_t Index, typename TKey, typename TValue>
	FORCEINLINE decltype(auto) get(Fuko::TPair<TKey, TValue>& Pair)
	{
		if constexpr (Index == 0)
		{
			return static_cast<TKey&>(Pair.Key);
		}
		else if constexpr (Index == 1)
		{
			return static_cast<TValue&>(Pair.Value);
		}
		else
		{
			static_assert(false, "TPair out of range!!!");
		}
	}

	template<size_t Index, typename TKey, typename TValue>
	FORCEINLINE decltype(auto) get(Fuko::TPair<TKey, TValue>&& Pair)
	{
		if constexpr (Index == 0)
		{
			return std::move(static_cast<TKey&>(Pair.Key));
		}
		else if constexpr (Index == 1)
		{
			return std::move(static_cast<TValue&>(Pair.Value));
		}
		else
		{
			static_assert(false, "TPair out of range!!!");
		}
	}
}

// support transform
namespace Fuko
{
	template <typename FuncType, typename TKey, typename TValue>
	FORCEINLINE decltype(auto) TransformTuple(TPair<TKey,TValue>&& Pair, FuncType&& Func)
	{
		return MakeTuple(
			Func(Pair.Key),
			Func(Pair.Value)
		);
	}

	template <typename FuncType, typename TKey, typename TValue>
	FORCEINLINE decltype(auto) TransformPair(TPair<TKey, TValue>&& Pair, FuncType&& Func)
	{
		return MakePair(
			Func(Pair.Key),
			Func(Pair.Value)
		);
	}
}
