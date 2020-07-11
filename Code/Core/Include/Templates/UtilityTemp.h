#pragma once
#include "TypeTraits.h"
#include "CoreType.h"
#include <initializer_list>
#include "Align.h"

// 用来防止模板特化版本调用非特化版本造成无限递归的TypeWrapper 
template <typename T> struct TTypeWrapper;
template <typename T> struct TUnwrapType					{ typedef T Type; };
template <typename T> struct TUnwrapType<TTypeWrapper<T>>	{ typedef T Type; };
template <typename T> using  TUnwrapType_t = typename TUnwrapType<T>::Type;

// Maxsize of 
template <typename...>
struct TMaxSizeof;
template <>
struct TMaxSizeof<>
{
	static const uint32_t value = 0;
};
template <typename T, typename... TRest>
struct TMaxSizeof<T, TRest...>
{
	static const uint32_t value = sizeof(T) > TMaxSizeof<TRest...>::value ? sizeof(T) : TMaxSizeof<TRest...>::value;
};
template<typename T, typename... TRest>
inline constexpr uint32 TMaxSizeof_v = TMaxSizeof<T, TRest...>::value;

// 上面的一个特化版本
template <bool Predicate, typename Func> struct TLazyEnableIf;
template <typename Func> struct TLazyEnableIf<true, Func> { typedef typename Func::Type Type; };
template <typename Func>struct TLazyEnableIf<false, Func> { };

template<bool Predicate, typename Func>
using TLazyEnableIf_t = typename TLazyEnableIf<Predicate, Func>::Type;

// 是否是顺序容器
template <typename T>
struct TIsContiguousContainer
{
	static constexpr bool value = false;
};

template <typename T> struct TIsContiguousContainer<             T& > : TIsContiguousContainer<T> {};
template <typename T> struct TIsContiguousContainer<             T&&> : TIsContiguousContainer<T> {};
template <typename T> struct TIsContiguousContainer<const          T> : TIsContiguousContainer<T> {};
template <typename T> struct TIsContiguousContainer<      volatile T> : TIsContiguousContainer<T> {};
template <typename T> struct TIsContiguousContainer<const volatile T> : TIsContiguousContainer<T> {};

template <typename T, size_t N> struct TIsContiguousContainer<               T[N]> { static constexpr bool value = true; };
template <typename T, size_t N> struct TIsContiguousContainer<const          T[N]> { static constexpr bool value = true; };
template <typename T, size_t N> struct TIsContiguousContainer<      volatile T[N]> { static constexpr bool value = true; };
template <typename T, size_t N> struct TIsContiguousContainer<const volatile T[N]> { static constexpr bool value = true; };

template <typename T>
struct TIsContiguousContainer<std::initializer_list<T>>
{
	static constexpr bool value = true;
};

template<typename T>
inline constexpr bool TIsContiguousContainer_v = TIsContiguousContainer<T>::value;

// 交换 
template <typename T>
inline typename void Swap(T& A, T& B)
{
	if constexpr (TUseBitwiseSwap_v<T>)
	{
		if (&A != &B)
		{
			TStorage<T> Temp;
			::Fuko::Memcpy(&Temp, &A, sizeof(T));
			::Fuko::Memcpy(&A, &B, sizeof(T));
			::Fuko::Memcpy(&B, &Temp, sizeof(T));
		}
	}
	else
	{
		T Temp = std::move(A);
		A = std::move(B);
		B = std::move(Temp);
	}
}

// 把CV限定符从From拷贝到To 
template <typename From, typename To> struct TCopyQualifiersFromTo                          { typedef                To Type; };
template <typename From, typename To> struct TCopyQualifiersFromTo<const          From, To> { typedef const          To Type; };
template <typename From, typename To> struct TCopyQualifiersFromTo<      volatile From, To> { typedef       volatile To Type; };
template <typename From, typename To> struct TCopyQualifiersFromTo<const volatile From, To> { typedef const volatile To Type; };
template <typename From, typename To>
using TCopyQualifiersFromTo_t = typename TCopyQualifiersFromTo<From, To>::Type;

// From到To是否丢失了CV限定符   
template <typename From, typename To>
struct TLosesQualifiersFromTo
{
	static constexpr bool Value = !std::is_same_v<TCopyQualifiersFromTo_t<From, To>, To>;
};
template <typename From, typename To>
inline constexpr bool TLosesQualifiersFromTo_v = TLosesQualifiersFromTo<From,To>::Value;

// 把谓语置反
template <typename PredicateType>
class TReversePredicate
{
	const PredicateType& Predicate;

public:
	TReversePredicate(const PredicateType& InPredicate)
		: Predicate(InPredicate)
	{
	}

	template <typename T>
	FORCEINLINE bool operator()(T&& A, T&& B) const
	{
		return std::invoke(Predicate, std::forward<T>(B), std::forward<T>(A));
	}
};

// 得到数据
template<typename T>
auto GetData(T&& Container) -> decltype(Container.GetData())
{
	static_assert(TIsContiguousContainer_v<T>, "T must be a Contiguous container");
	return Container.GetData();
}
template <typename T, size_t N> constexpr       T* GetData(T(&Container)[N]) { return Container; }
template <typename T, size_t N> constexpr       T* GetData(T(&&Container)[N]) { return Container; }
template <typename T, size_t N> constexpr const T* GetData(const T(&Container)[N]) { return Container; }
template <typename T, size_t N> constexpr const T* GetData(const T(&&Container)[N]) { return Container; }
template <typename T> constexpr const T* GetData(std::initializer_list<T> List) { return List.begin(); }

// 得到长度
template<typename T>
auto GetNum(T&& Container) -> decltype(Container.Num()) 
{
	static_assert(TIsContiguousContainer_v<std::decay_t<T>>, "T must be a Contiguous container");
	return Container.Num(); 
}
template <typename T, size_t N> constexpr size_t GetNum(T(&Container)[N]) { return N; }
template <typename T, size_t N> constexpr size_t GetNum(T(&&Container)[N]) { return N; }
template <typename T, size_t N> constexpr size_t GetNum(const T(&Container)[N]) { return N; }
template <typename T, size_t N> constexpr size_t GetNum(const T(&&Container)[N]) { return N; }
template <typename T> constexpr size_t GetNum(std::initializer_list<T> List) { return List.size(); }

// 得到元素指针类型 
template <typename RangeType>
struct TRangePointerType
{
	using Type = decltype(&*std::declval<RangeType&>().begin());
};

template <typename T, unsigned int N>
struct TRangePointerType<T[N]>
{
	using Type = T * ;
};

// 右值转换为左值 
template <typename T> struct TRValueToLValueReference { typedef T  Type; };
template <typename T> struct TRValueToLValueReference<T&&> { typedef T& Type; };