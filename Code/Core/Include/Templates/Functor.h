#pragma once
#include "CoreConfig.h"
#include "Templates/UtilityTemp.h"

// 直接返回输入值的Functor用于默认的Projection 
struct FIdentityFunctor
{
	template <typename T>
	FORCEINLINE T&& operator()(T&& Val) const
	{
		return (T&&)Val;
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


