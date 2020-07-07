#pragma once
#include <type_traits>
#include <stdint.h>
#include <xtr1common>
#include "CoreType.h"

// 是否是强类型枚举
template<typename T>
struct TIsEnumClass
{
private:
	static char(&Fun(int))[2];
	static char Fun(...);
public:
	static constexpr bool value = std::is_enum_v<T> && !(sizeof(Fun(T())) - 1);
};
template<typename T>
inline constexpr bool TIsEnumClass_v = TIsEnumClass<T>::value;

// 是否为可变参函数的有效参数
template <typename T>
struct TIsValidVariadicFunctionArg
{
	static constexpr bool value =
		std::is_same_v<T, uint32_t> ||
		std::is_same_v<T, uint8_t> ||
		std::is_same_v<T, int32_t> ||
		std::is_same_v<T, uint64_t> ||
		std::is_same_v<T, int64_t> ||
		std::is_same_v<T, double> ||
		std::is_same_v<T, long> ||
		std::is_same_v<T, unsigned long> ||
		std::is_same_v<T, char> ||
		std::is_same_v<T, wchar_t> ||
		std::is_same_v<T, bool> ||
		std::is_same_v<T, const void*>;
};
template <typename T>
inline constexpr bool TIsValidVariadicFunctionArg_v = TIsValidVariadicFunctionArg<T>::value;

// 是否可以使用0来构造的类型 
template <typename T>
struct TIsZeroConstructType
{
	static constexpr bool value = std::is_enum_v<T> || std::is_arithmetic_v<T> || std::is_pointer_v<T>;
};
template <typename T>
inline constexpr bool TIsZeroConstructType_v = TIsZeroConstructType<T>::value;

// 是否可以直接memecpy 
template <typename T, typename Arg>
struct TIsBitwiseConstructible
{
	static_assert(
		!std::is_lvalue_reference_v<T> &&
		!std::is_lvalue_reference_v<Arg>,
		"TIsBitwiseConstructible is not designed to accept reference types");

	static_assert(
		std::is_same_v<T,std::remove_cv_t<T>> &&
		std::is_same_v<Arg,std::remove_cv_t<Arg>>,
		"TIsBitwiseConstructible is not designed to accept qualified types");

	static constexpr bool value = false;
};
template <typename T>
struct TIsBitwiseConstructible<T, T>
{
	// 对于平凡构造的对象，可以按位拷贝  
	static constexpr bool value = std::is_trivially_copy_constructible_v<T>;
};
template <typename T, typename U>
struct TIsBitwiseConstructible<const T, U> : TIsBitwiseConstructible<T, U>
{
	// 从非const到const对象的拷贝等同于非const互相拷贝  
};
template <typename T>
struct TIsBitwiseConstructible<const T*, T*>
{
	// 从非const指针向const指针的拷贝是必定允许的 
	static constexpr bool value = true;
};
// unsigned和signed拷贝的特化
template <> struct TIsBitwiseConstructible< uint8, int8>	{ static constexpr bool value = true; };
template <> struct TIsBitwiseConstructible<  int8, uint8>	{ static constexpr bool value = true; };
template <> struct TIsBitwiseConstructible<uint16, int16>	{ static constexpr bool value = true; };
template <> struct TIsBitwiseConstructible< int16, uint16>	{ static constexpr bool value = true; };
template <> struct TIsBitwiseConstructible<uint32, int32>	{ static constexpr bool value = true; };
template <> struct TIsBitwiseConstructible< int32, uint32>	{ static constexpr bool value = true; };
template <> struct TIsBitwiseConstructible<uint64, int64>	{ static constexpr bool value = true; };
template <> struct TIsBitwiseConstructible< int64, uint64>	{ static constexpr bool value = true; };
template<typename T, typename Arg>
inline constexpr bool TIsBitwiseConstructible_v = TIsBitwiseConstructible<T, Arg>::value;

// 是否可以直接realloc，不调用构造和析构 
template <typename DestElementType, typename SrcElementType>
struct TCanBitwiseRelocate
{
	static constexpr bool value = (TIsBitwiseConstructible_v<DestElementType, SrcElementType> &&
								std::is_trivially_destructible_v<SrcElementType>) ||
								std::is_same_v<DestElementType, SrcElementType>;
};
template <typename DestElementType, typename SrcElementType>
inline constexpr bool TCanBitwiseRelocate_v = TCanBitwiseRelocate<DestElementType, SrcElementType>::value;

// 是否可以直接比内存  
template<typename T>
struct TCanBitwiseCompare
{
	static constexpr bool value = std::is_enum_v<T> || std::is_arithmetic_v<T> || std::is_pointer_v<T>;
};
template<typename T>
inline constexpr bool TCanBitwiseCompare_v = TCanBitwiseCompare<T>::value;

// 是否是数组或者引用 
template <typename T, typename ArrType>
struct TIsArrayOrRefOfType
{
	enum { Value = false };
};

template <typename ArrType> struct TIsArrayOrRefOfType<               ArrType[], ArrType> { enum { Value = true }; };
template <typename ArrType> struct TIsArrayOrRefOfType<const          ArrType[], ArrType> { enum { Value = true }; };
template <typename ArrType> struct TIsArrayOrRefOfType<      volatile ArrType[], ArrType> { enum { Value = true }; };
template <typename ArrType> struct TIsArrayOrRefOfType<const volatile ArrType[], ArrType> { enum { Value = true }; };

template <typename ArrType, unsigned int N> struct TIsArrayOrRefOfType<               ArrType[N], ArrType> { enum { Value = true }; };
template <typename ArrType, unsigned int N> struct TIsArrayOrRefOfType<const          ArrType[N], ArrType> { enum { Value = true }; };
template <typename ArrType, unsigned int N> struct TIsArrayOrRefOfType<      volatile ArrType[N], ArrType> { enum { Value = true }; };
template <typename ArrType, unsigned int N> struct TIsArrayOrRefOfType<const volatile ArrType[N], ArrType> { enum { Value = true }; };

template <typename ArrType, unsigned int N> struct TIsArrayOrRefOfType<               ArrType(&)[N], ArrType> { enum { Value = true }; };
template <typename ArrType, unsigned int N> struct TIsArrayOrRefOfType<const          ArrType(&)[N], ArrType> { enum { Value = true }; };
template <typename ArrType, unsigned int N> struct TIsArrayOrRefOfType<      volatile ArrType(&)[N], ArrType> { enum { Value = true }; };
template <typename ArrType, unsigned int N> struct TIsArrayOrRefOfType<const volatile ArrType(&)[N], ArrType> { enum { Value = true }; };
template <typename T, typename ArrType>
inline constexpr bool TIsArrayOrRefOfType_v = TIsArrayOrRefOfType<T, ArrType>::Value;

// 是否可以直接使用按位交换
template <typename T>
struct TUseBitwiseSwap
{
	static constexpr bool Value = !(__is_enum(T) || std::is_pointer_v<T> || std::is_arithmetic_v<T>);
};
template <typename T>
inline constexpr bool TUseBitwiseSwap_v = TUseBitwiseSwap<T>::Value;

// 从Byte数到IntType
template <int NumBytes>
struct TSignedIntType {};
template <> struct TSignedIntType<1> { using Type = int8; };
template <> struct TSignedIntType<2> { using Type = int16; };
template <> struct TSignedIntType<4> { using Type = int32; };
template <> struct TSignedIntType<8> { using Type = int64; };
template <int NumBytes>
using TSignedIntType_T = typename TSignedIntType<NumBytes>::Type;

// 从Size到UIntType
template <int NumBytes>
struct TUnsignedIntType {};
template <> struct TUnsignedIntType<1> { using Type = uint8; };
template <> struct TUnsignedIntType<2> { using Type = uint16; };
template <> struct TUnsignedIntType<4> { using Type = uint32; };
template <> struct TUnsignedIntType<8> { using Type = uint64; };
template <int NumBytes>
using TUnsignedIntType_T = typename TUnsignedIntType<NumBytes>::Type;
