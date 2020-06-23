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

// Remove CV 
template<typename T> struct TRemoveCV					{ typedef T Type; };
template<typename T> struct TRemoveCV<const T>			{ typedef T Type; };
template<typename T> struct TRemoveCV<volatile T>		{ typedef T Type; };
template<typename T> struct TRemoveCV<const volatile T> { typedef T Type; };
template<typename T> struct TRemoveC					{ typedef T Type; };
template<typename T> struct TRemoveC<const T>			{ typedef T Type; };
template<typename T> struct TRemoveV					{ typedef T Type; };
template<typename T> struct TRemoveV<volatile T>		{ typedef T Type; };
template<typename T> using  TRemoveCV_t = typename TRemoveCV<T>::Type;
template<typename T> using  TRemoveC_t = typename TRemoveC<T>::Type;
template<typename T> using  TRemoveV_t = typename TRemoveV<T>::Type;

// Remove pointer
template<typename T> struct TRemovePointer                    { typedef T Type; };
template<typename T> struct TRemovePointer<T*>                { typedef T Type; };
template<typename T> struct TRemovePointer<T* const>          { typedef T Type; };
template<typename T> struct TRemovePointer<T* volatile>       { typedef T Type; };
template<typename T> struct TRemovePointer<T* const volatile> { typedef T Type; };
template<typename T> using  TRemovePointer_t = typename TRemovePointer<T>::Type;

// Remove reference 
template<typename T> struct TRemoveReference	  { typedef T Type; };
template<typename T> struct TRemoveReference<T&>  { typedef T Type; };
template<typename T> struct TRemoveReference<T&&> { typedef T Type; };
template<typename T> using  TRemoveReference_t = typename TRemoveReference<T>::Type;

// Conditional 
template<bool B, typename T, typename F>
struct TConditional { typedef T Type; };
template<typename T, typename F>
struct TConditional<false, T, F> { typedef F Type; };
template<bool B, typename T, typename F>
using TConditional_t = typename TConditional<B, T, F>::Type;

// Remove extent
template<typename T>			struct TRemoveExtent		{ typedef T Type; };
template<typename T>			struct TRemoveExtent<T[]>	{ typedef T Type; };
template<typename T, size_t N>	struct TRemoveExtent<T[N]>	{ typedef T Type; };
template<typename T>			using  TRemoveExtent_t = typename TRemoveExtent<T>::Type;

// Decay
template<typename T>
struct TDecay 
{
	using Type = std::decay_t<T>;
};
template<typename T>
using TDecay_t = std::decay_t<T>;

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

// Are type equal
template<typename A, typename B>
struct TAreTypesEqual
{
	static constexpr bool value = false;
};
template<typename A>
struct TAreTypesEqual<A, A>
{
	static constexpr bool value = true;
};
template<typename A, typename B>
inline constexpr bool TAreTypesEqual_v = TAreTypesEqual<A, B>::value;

// Move 
template <typename T>
FORCEINLINE TRemoveReference_t<T>&& MoveTemp(T&& Obj)
{
	typedef TRemoveReference_t<T> CastType;

	// 对右值使用是多余的，对const值使用几乎都是错误的情况
	static_assert(TIsLValueReferenceType_v<T>, "MoveTemp called on an rvalue");
	static_assert(!TAreTypesEqual_v<CastType&, const CastType&>, "MoveTemp called on a const object");

	return (CastType&&)Obj;
}
template <typename T>
FORCEINLINE TRemoveReference_t<T>&& MoveTempIfPossible(T&& Obj)
{
	typedef TRemoveReference_t<T> CastType;
	return (CastType&&)Obj;
}

template <typename T>
FORCEINLINE T&& Forward(typename TRemoveReference_t<T>& Obj)
{
	return static_cast<T&&>(Obj);
}
template <typename T>
FORCEINLINE T&& Forward(typename TRemoveReference_t<T>&& Obj)
{
	return static_cast<T&&>(Obj);
}

// Enable if 
template <bool Predicate, typename T = void> struct TEnableIf;
template <typename T> struct TEnableIf<true, T> { typedef T Type; };
template <typename T> struct TEnableIf<false, T> {};

template<bool Predicate, typename T = void>
using TEnableIf_t = typename TEnableIf<Predicate, T>::Type;

// 上面的一个特化版本
template <bool Predicate, typename Func> struct TLazyEnableIf;
template <typename Func> struct TLazyEnableIf<true, Func> { typedef typename Func::Type Type; };
template <typename Func>struct TLazyEnableIf<false, Func> { };

template<bool Predicate, typename Func>
using TLazyEnableIf_t = typename TLazyEnableIf<Predicate, Func>::Type;


// Choose class
template<bool Predicate, typename TrueClass, typename FalseClass>
class TChooseClass;

template<typename TrueClass, typename FalseClass>
class TChooseClass<true, TrueClass, FalseClass>
{
public:
	typedef TrueClass Type;
};

template<typename TrueClass, typename FalseClass>
class TChooseClass<false, TrueClass, FalseClass>
{
public:
	typedef FalseClass Type;
};

template<bool Predicate, typename TrueClass, typename FalseClass>
using TChooseClass_t = typename TChooseClass<Predicate, TrueClass, FalseClass>::Type;

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
		T Temp = MoveTemp(A);
		A = MoveTemp(B);
		B = MoveTemp(Temp);
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
	static constexpr bool Value = !TAreTypesEqual_v<TCopyQualifiersFromTo_t<From, To>, To>;
};
template <typename From, typename To>
inline constexpr bool TLosesQualifiersFromTo_v = TLosesQualifiersFromTo<From,To>::Value;

// 指针转换是否合法 
namespace PointerIsConvertibleFromTo_Private
{
	template <typename From, typename To, typename NoCVFrom = typename TRemoveCV_t<From>, typename NoCVTo = typename TRemoveCV_t<To>>
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
		return Invoke(Predicate, Forward<T>(B), Forward<T>(A));
	}
};

// 得到数据
template<typename T, typename = typename TEnableIf_t<TIsContiguousContainer_v<T>>>
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
template<typename T, typename = typename TEnableIf_t<TIsContiguousContainer_v<T>>>
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

// 声明一个值用于一些测试 
template <typename T>
T&& DeclVal();

// 得到元素指针类型 
template <typename RangeType>
struct TRangePointerType
{
	using Type = decltype(&*DeclVal<RangeType&>().begin());
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
	static_assert(TIsIntegral_v<T>);
	constexpr size_t Num()
	{
		return sizeof...(Indices);
	}
};

template <typename T, T N>
using TMakeIntegerSequence = __make_integer_seq<TIntegerSequence, T, N>;

// 置反位 
template <typename T>
FORCEINLINE typename TEnableIf_t<TAreTypesEqual_v<T, uint32>, T> ReverseBits(T Bits)
{
	Bits = (Bits << 16) | (Bits >> 16);
	Bits = ((Bits & 0x00ff00ff) << 8) | ((Bits & 0xff00ff00) >> 8);
	Bits = ((Bits & 0x0f0f0f0f) << 4) | ((Bits & 0xf0f0f0f0) >> 4);
	Bits = ((Bits & 0x33333333) << 2) | ((Bits & 0xcccccccc) >> 2);
	Bits = ((Bits & 0x55555555) << 1) | ((Bits & 0xaaaaaaaa) >> 1);
	return Bits;
}