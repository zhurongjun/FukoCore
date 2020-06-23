#pragma once
#include "CoreType.h"
#include "CoreConfig.h"

// 用于实现concept 
template <typename Concept, typename... Args>
struct TModels
{
	template <typename... Ts>
	static char(&Resolve(decltype(&Concept::template Requires<Ts...>)*))[2];

	template <typename... Ts>
	static char(&Resolve(...))[1];

	static constexpr bool Value = sizeof(Resolve<Args...>(0)) == 2;
};
template <typename Concept, typename... Args>
inline constexpr bool TModels_V = TModels<Concept, Args...>::Value;

// 细化concept条件
template <typename Concept, typename... Args>
auto Refines() -> int(&)[!!TModels<Concept, Args...>::Value * 2 - 1];

