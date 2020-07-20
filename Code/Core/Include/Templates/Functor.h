#pragma once
#include <CoreType.h>
#include <CoreConfig.h>
#include "Templates/UtilityTemp.h"

// 元素map方式
struct NoMap
{
	template<typename T>
	FORCEINLINE constexpr decltype(auto) operator()(T&& Val) const { return std::forward<T>(Val); }
};
struct MapDeref
{
	template<typename T>
	FORCEINLINE constexpr auto& operator()(T&& Val) const
	{
		if constexpr (std::is_pointer_v<T>)
			return *Val;
		else
			return Val;
	}
};
struct MapKey
{
	template<typename T>
	FORCEINLINE constexpr auto& operator()(T&& Val) const { return Val.Key; }
};
struct MapValue
{
	template<typename T>
	FORCEINLINE constexpr auto& operator()(T&& Val) const { return Val.Value; }
};
struct MapKeyDeref
{
	template<typename T>
	FORCEINLINE constexpr auto& operator()(T&& Val) const 
	{
		if constexpr (std::is_pointer_v<decltype(Val.Key)>)
			return *Val.Key;
		else
			return Val.Key;
	}
};
struct MapValueDeref
{
	template<typename T>
	FORCEINLINE constexpr auto& operator()(T&& Val) const
	{
		if constexpr (std::is_pointer_v<decltype(Val.Value)>)
			return *Val.Value;
		else
			return Val.Value;
	}
};

// Less
template <typename T = void>
struct TLess
{
	FORCEINLINE constexpr bool operator()(const T& A, const T& B) const
	{
		return A < B;
	}
};
template <>
struct TLess<void>
{
	template <typename T, typename U>
	FORCEINLINE constexpr bool operator()(T&& A, U&& B) const
	{
		return std::forward<T>(A) < std::forward<U>(B);
	}
};

// Greater
template <typename T = void>
struct TGreater
{
	FORCEINLINE constexpr bool operator()(const T& A, const T& B) const
	{
		return A > B;
	}
};
template <>
struct TGreater<void>
{
	template <typename T, typename U>
	FORCEINLINE constexpr bool operator()(T&& A, U&& B) const
	{
		return std::forward<T>(A) > std::forward<U>(B);
	}
};

// 加法
template<typename T = void>
struct TPlus
{
	FORCEINLINE constexpr T operator()(const T& A, const T& B) { return A + B; }
};
template<>
struct TPlus<void>
{
	template<typename U, typename V>
	FORCEINLINE constexpr auto operator()(U&& A, V&& B) -> decltype(A + B) { return A + B; }
};

// 相等
template <typename T = void>
struct TEqualTo
{
	constexpr auto operator()(const T& Lhs, const T& Rhs) const -> decltype(Lhs == Rhs)
	{
		return Lhs == Rhs;
	}
};

template <>
struct TEqualTo<void>
{
	template <typename T, typename U>
	constexpr auto operator()(T&& Lhs, U&& Rhs) const -> decltype(std::forward<T>(Lhs) == std::forward<U>(Rhs))
	{
		return std::forward<T>(Lhs) == std::forward<U>(Rhs);
	}
};


