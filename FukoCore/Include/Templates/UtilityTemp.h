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
	static constexpr bool Value = true;
};

template<typename T>
inline constexpr bool TIsContiguousContainer_v = TIsContiguousContainer<T>::Value;

// 交换 
template <typename T>
inline typename void Swap(T& A, T& B)
{
	if constexpr (TUseBitwiseSwap_v<T>)
	{
		if (&A != &B)
		{
			TTypeCompatibleBytes<T> Temp;
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

// 指针转换是否合法 
namespace PointerIsConvertibleFromTo_Private
{
	template <typename From, typename To, typename NoCVFrom = std::remove_cv_t<From>, typename NoCVTo = std::remove_cv_t<To>>
	struct TImpl
	{
	private:
		static uint8  Test(...);
		static uint16 Test(To*);

	public:
		enum { Value = sizeof(Test((From*)nullptr)) - 1 };
	};

	template <typename From, typename To, typename NoCVFrom>
	struct TImpl<From, To, NoCVFrom, NoCVFrom>
	{
		// cv T* to cv T* conversions are always allowed as long as no CVs are lost
		enum { Value = !TLosesQualifiersFromTo<From, To>::Value };
	};

	template <typename From, typename To, typename NoCVFrom>
	struct TImpl<From, To, NoCVFrom, void>
	{
		// cv T* to cv void* conversions are always allowed as long as no CVs are lost
		enum { Value = !TLosesQualifiersFromTo<From, To>::Value };
	};

	template <typename From, typename To>
	struct TImpl<From, To, void, void>
	{
		// cv void* to cv void* conversions are always allowed as long as no CVs are lost
		enum { Value = !TLosesQualifiersFromTo<From, To>::Value };
	};

	template <typename From, typename To, typename NoCVTo>
	struct TImpl<From, To, void, NoCVTo>
	{
		// cv void* to cv not_void* conversions are never legal
		enum { Value = false };
	};
}

template <typename From, typename To>
struct TPointerIsConvertibleFromTo : PointerIsConvertibleFromTo_Private::TImpl<From, To>
{
};

// Invoke
namespace Invoke_Private 
{
	template <class>
	constexpr bool is_reference_wrapper_v = false;
	template <class U>
	constexpr bool is_reference_wrapper_v<std::reference_wrapper<U>> = true;

	template <class T, class Type, class T1, class... Args>
	constexpr decltype(auto) INVOKE(Type T::* f, T1&& t1, Args&&... args)
	{
		if constexpr (std::is_member_function_pointer_v<decltype(f)>) 
		{
			if constexpr (std::is_base_of_v<T, std::decay_t<T1>>)
				return (std::forward<T1>(t1).*f)(std::forward<Args>(args)...);
			else if constexpr (is_reference_wrapper_v<std::decay_t<T1>>)
				return (t1.get().*f)(std::forward<Args>(args)...);
			else
				return ((*std::forward<T1>(t1)).*f)(std::forward<Args>(args)...);
		}
		else 
		{
			static_assert(std::is_member_object_pointer_v<decltype(f)>);
			static_assert(sizeof...(args) == 0);
			if constexpr (std::is_base_of_v<T, std::decay_t<T1>>)
				return std::forward<T1>(t1).*f;
			else if constexpr (is_reference_wrapper_v<std::decay_t<T1>>)
				return t1.get().*f;
			else
				return (*std::forward<T1>(t1)).*f;
		}
	}

	template <class F, class... Args>
	decltype(auto) INVOKE(F&& f, Args&&... args)
	{
		return std::forward<F>(f)(std::forward<Args>(args)...);
	}
} 
template< class F, class... Args>
constexpr std::invoke_result_t<F, Args...> Invoke(F&& f, Args&&... args)
{
	return Invoke_Private::INVOKE(std::forward<F>(f), std::forward<Args>(args)...);
}

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
		return Invoke(Predicate, std::forward<T>(B), std::forward<T>(A));
	}
};

// 得到数据
template<typename T, typename = typename std::enable_if_t<TIsContiguousContainer_v<T>>>
auto GetData(T&& Container) -> decltype(Container.GetData())
{
	return Container.GetData();
}

template <typename T, size_t N> CONSTEXPR       T* GetData(T(&Container)[N]) { return Container; }
template <typename T, size_t N> CONSTEXPR       T* GetData(T(&&Container)[N]) { return Container; }
template <typename T, size_t N> CONSTEXPR const T* GetData(const T(&Container)[N]) { return Container; }
template <typename T, size_t N> CONSTEXPR const T* GetData(const T(&&Container)[N]) { return Container; }

template <typename T>
CONSTEXPR const T* GetData(std::initializer_list<T> List)
{
	return List.begin();
}

// 得到长度
template<typename T, typename = typename std::enable_if_t<TIsContiguousContainer_v<T>>>
auto GetNum(T&& Container) -> decltype(Container.Num())
{
	return Container.Num();
}

template <typename T, size_t N> CONSTEXPR size_t GetNum(T(&Container)[N]) { return N; }
template <typename T, size_t N> CONSTEXPR size_t GetNum(T(&&Container)[N]) { return N; }
template <typename T, size_t N> CONSTEXPR size_t GetNum(const T(&Container)[N]) { return N; }
template <typename T, size_t N> CONSTEXPR size_t GetNum(const T(&&Container)[N]) { return N; }

template <typename T>
CONSTEXPR size_t GetNum(std::initializer_list<T> List)
{
	return List.size();
}

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

// 得到参包第N项 
template <int32 N, typename... Types>
struct TNthTypeFromParameterPack;

template <int32 N, typename T, typename... OtherTypes>
struct TNthTypeFromParameterPack<N, T, OtherTypes...>
{
	using Type = typename TNthTypeFromParameterPack<N - 1, OtherTypes...>::Type;
};

template <typename T, typename... OtherTypes>
struct TNthTypeFromParameterPack<0, T, OtherTypes...>
{
	using Type = T;
};

template <typename... OtherTypes>
struct TNthTypeFromParameterPack<0, OtherTypes...>
{
	using Type = void;
};


// 整数序列
template <typename T, T... Indices>
struct TIntegerSequence
{
	static_assert(std::is_integral_v<T>);
	constexpr size_t Num()
	{
		return sizeof...(Indices);
	}
};

template <typename T, T N>
using TMakeIntegerSequence = __make_integer_seq<TIntegerSequence, T, N>;

// 置反位 
template <typename T>
FORCEINLINE typename std::enable_if_t<std::is_same_v<T, uint32>, T> ReverseBits(T Bits)
{
	Bits = (Bits << 16) | (Bits >> 16);
	Bits = ((Bits & 0x00ff00ff) << 8) | ((Bits & 0xff00ff00) >> 8);
	Bits = ((Bits & 0x0f0f0f0f) << 4) | ((Bits & 0xf0f0f0f0) >> 4);
	Bits = ((Bits & 0x33333333) << 2) | ((Bits & 0xcccccccc) >> 2);
	Bits = ((Bits & 0x55555555) << 1) | ((Bits & 0xaaaaaaaa) >> 1);
	return Bits;
}

// 右值转换为左值 
template <typename T> struct TRValueToLValueReference { typedef T  Type; };
template <typename T> struct TRValueToLValueReference<T&&> { typedef T& Type; };