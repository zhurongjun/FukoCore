#pragma once
#include <CoreType.h>
#include <CoreConfig.h>

namespace Fuko
{
	template<typename...Ts>
	struct TypeList {};

	template<int N, typename T, typename...Ts>
	struct TNthType { using Type = typename TNthType<N - 1, Ts...>::Type; };
	template<typename T, typename...Ts>
	struct TNthType<0, T, Ts...> { using Type = T; };
	template<int N, typename...Ts>
	using TNthType_t = typename TNthType<N, Ts...>::Type;

	template<int N,typename TList>
	struct TTypeAt;
	template<int N,typename...Ts>
	struct TTypeAt<N, TypeList<Ts...>> { using Type = TNthType_t<N, Ts...>; };
	template<int N, typename TList>
	using TTypeAt_t = typename TTypeAt<N, TList>::Type;

	template<typename T,typename TList>
	struct TContainType;
	template<typename T, typename...Ts>
	struct TContainType <T, TypeList<Ts...>> { static constexpr bool Value = std::disjunction_v<std::is_same<T, Ts>...>; };
	template<typename T, typename TList>
	inline constexpr bool TContainType_v = TContainType<T, TList>::Value;

	template<int N,typename TList>
	struct TRightTypes;
	template<int N, typename T, typename...Ts>
	struct TRightTypes<N, TypeList<T, Ts...>> { using Type = typename TRightTypes<N - 1, TypeList<Ts...>>::Type; };
	template<typename T, typename...Ts>
	struct TRightTypes<0, TypeList<T, Ts...>> { using Type = TypeList<T, Ts...>; };
	template<>
	struct TRightTypes<0,TypeList<>> { using Type = TypeList<>; };
	template<int N, typename TList>
	using TRightTypes_t = typename TRightTypes<N, TList>::Type;
}