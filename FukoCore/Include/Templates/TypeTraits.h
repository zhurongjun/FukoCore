#pragma once
#include <type_traits>
#include <stdint.h>
#include <xtr1common>
#include "CoreType.h"

// 是否是抽象类
template<typename T>
using TIsAbstract = std::is_abstract<T>;
template<typename T>
inline constexpr bool TIsAbstract_v = TIsAbstract<T>::value;

// 是否是算数类型
template<typename T>
using TIsArithmetic = std::is_arithmetic<T>;
template<typename T>
inline constexpr bool TIsArithmetic_v = TIsArithmetic<T>::value;

// 是否是数组
template<typename T>
using TIsArray = std::is_array<T>;
template<typename T>
inline constexpr bool TIsArray_v = TIsArray<T>::value;

// 是否是类
template<typename T>
using TIsClass = std::is_class<T>;
template<typename T>
inline constexpr bool TIsClass_v = TIsClass<T>::value;

// 是否是可构造的
template<typename T>
using TIsConstructible = std::is_constructible<T>;
template<typename T>
inline constexpr bool TIsConstructible_v = TIsConstructible<T>::value;

// 是否是枚举
template<typename T>
using TIsEnum = std::is_enum<T>;
template<typename T>
inline constexpr bool TIsEnum_v = TIsEnum<T>::value;

// 是否是强类型枚举
template<typename T>
struct TIsEnumClass
{
private:
	static char(&Fun(int))[2];
	static char Fun(...);
public:
	static constexpr bool value = TIsEnum_v<T> && !(sizeof(Fun(T())) - 1);
};
template<typename T>
inline constexpr bool TIsEnumClass_v = TIsEnumClass<T>::value;

// 是否是浮点
template<typename T>
using TIsFloatingPoint = std::is_floating_point<T>;
template<typename T>
inline constexpr bool TIsFloatingPoint_v = TIsFloatingPoint<T>::value;

// 是否是整型
template<typename T>
using TIsIntegral = std::is_integral<T>;
template<typename T>
inline constexpr bool TIsIntegral_v = TIsIntegral<T>::value;

// 是否是可调用的
template<typename T>
using TIsInvocable = std::is_invocable<T>;
template<typename T>
inline constexpr bool TIsInvocable_v = TIsInvocable<T>::value;

// 是否是POD类型
template<typename T>
using TIsPODType = std::is_pod<T>;
template<typename T>
inline constexpr bool TIsPODType_v = TIsPODType<T>::value;

// 是否是指针
template<typename T>
using TIsPointer = std::is_pointer<T>;
template<typename T>
inline constexpr bool TIsPointer_v = TIsPointer<T>::value;

// 是否是引用类型
template<typename T>
using TIsReferenceType = std::is_reference<T>;
template<typename T>
inline constexpr bool TIsReferenceType_v = TIsReferenceType<T>::value;

// 是否是有符号的
template<typename T>
using TIsSigned = std::is_signed<T>;
template<typename T>
inline constexpr bool TIsSigned_v = TIsSigned<T>::value;

// 是否是平凡的赋值运算符
template<typename T>
struct TIsTriviallyCopyAssignable
{
	static constexpr bool value = __has_trivial_assign(T) || TIsPODType_v<T>;
};
template<typename T>
inline constexpr bool TIsTriviallyCopyAssignable_v = TIsTriviallyCopyAssignable<T>::value;

// 是否具有平凡的复制构造
template<typename T>
struct TIsTriviallyCopyConstructible
{
	static constexpr bool value = __has_trivial_copy(T) || TIsPODType_v<T>;
};
template<typename T>
inline constexpr bool TIsTriviallyCopyConstructible_v = TIsTriviallyCopyConstructible<T>::value;

// 是否具有平凡的析构
template<typename T>
struct TIsTriviallyDestructible
{
	static constexpr bool value = __has_trivial_destructor(T) || __is_enum(T);
};
template<typename T>
inline constexpr bool TIsTriviallyDestructible_v = TIsTriviallyDestructible<T>::value;

// 是否是平凡类型
template<typename T>
using TIsTrivial = std::is_trivial<T>;
template<typename T>
inline constexpr bool TIsTrivial_v = TIsTrivial<T>::value;

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
	static constexpr bool value = TIsEnum_v<T> || TIsArithmetic_v<T> || TIsPointer_v<T>;
};
template <typename T>
inline constexpr bool TIsZeroConstructType_v = TIsZeroConstructType<T>::value;

// 是否可以直接memecpy 
template <typename T, typename Arg>
struct TIsBitwiseConstructible
{
	static_assert(
		!TIsReferenceType_v<T> &&
		!TIsReferenceType_v<Arg>,
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
	static constexpr bool value = TIsTriviallyCopyConstructible_v<T>;
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
// 硬写的unsigned和signed的Construct  
template <> struct TIsBitwiseConstructible< uint8_t, int8_t>	{ static constexpr bool value = true; };
template <> struct TIsBitwiseConstructible<  int8_t, uint8_t>	{ static constexpr bool value = true; };
template <> struct TIsBitwiseConstructible<uint16_t, int16_t>	{ static constexpr bool value = true; };
template <> struct TIsBitwiseConstructible< int16_t, uint16_t>	{ static constexpr bool value = true; };
template <> struct TIsBitwiseConstructible<uint32_t, int32_t>	{ static constexpr bool value = true; };
template <> struct TIsBitwiseConstructible< int32_t, uint32_t>	{ static constexpr bool value = true; };
template <> struct TIsBitwiseConstructible<uint64_t, int64_t>	{ static constexpr bool value = true; };
template <> struct TIsBitwiseConstructible< int64_t, uint64_t>	{ static constexpr bool value = true; };
template<typename T, typename Arg>
inline constexpr bool TIsBitwiseConstructible_v = TIsBitwiseConstructible<T, Arg>::value;

// 是否可以直接realloc，不调用构造和析构 
template <typename DestElementType, typename SrcElementType>
struct TCanBitwiseRelocate
{
	static constexpr bool value = (TIsBitwiseConstructible_v<DestElementType, SrcElementType> &&
							  TIsTriviallyDestructible_v<DestElementType, SrcElementType>) ||
								std::is_same_v<DestElementType, SrcElementType>;
};
template <typename DestElementType, typename SrcElementType>
inline constexpr bool TCanBitwiseRelocate_v = TCanBitwiseRelocate<DestElementType, SrcElementType>::value;

// 是否可以直接比内存  
template<typename T>
struct TCanBitwiseCompare
{
	static constexpr bool value = TIsEnum_v<T> || TIsArithmetic_v<T> || TIsPointer_v<T>;
};
template<typename T>
inline constexpr bool TCanBitwiseCompare_v = TCanBitwiseCompare<T>::value;

// Array的一些特性萃取  
template<typename T> struct TContainerTraitsBase
{
	enum { MoveWillEmptyContainer = false };
};
template<typename T> struct TContainerTraits : public TContainerTraitsBase<T> {};

// 是否是左值 
template<typename T> struct TIsLValueReferenceType     { static constexpr bool value = false; };
template<typename T> struct TIsLValueReferenceType<T&> { static constexpr bool value = true; };
template<typename T>
inline constexpr bool TIsLValueReferenceType_v = TIsLValueReferenceType<T>::value;

// 是否是右值 
template<typename T> struct TIsRValueReferenceType      { static constexpr bool value = false; };
template<typename T> struct TIsRValueReferenceType<T&&> { static constexpr bool value = true; };
template<typename T>
inline constexpr bool TIsRValueReferenceType_v = TIsRValueReferenceType<T>::value;

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
	static constexpr bool Value = !(__is_enum(T) || TIsPointer_v<T> || TIsArithmetic_v<T>);
};
template <typename T>
inline constexpr bool TUseBitwiseSwap_v = TUseBitwiseSwap<T>::Value;

// 从Byte数到IntType
template <int NumBytes>
struct TSignedIntType
{
};

template <> struct TSignedIntType<1> { using Type = int8; };
template <> struct TSignedIntType<2> { using Type = int16; };
template <> struct TSignedIntType<4> { using Type = int32; };
template <> struct TSignedIntType<8> { using Type = int64; };

template <int NumBytes>
using TSignedIntType_T = typename TSignedIntType<NumBytes>::Type;

template <int NumBytes>
struct TUnsignedIntType
{
};

template <> struct TUnsignedIntType<1> { using Type = uint8; };
template <> struct TUnsignedIntType<2> { using Type = uint16; };
template <> struct TUnsignedIntType<4> { using Type = uint32; };
template <> struct TUnsignedIntType<8> { using Type = uint64; };
template <int NumBytes>
using TUnsignedIntType_T = typename TUnsignedIntType<NumBytes>::Type;