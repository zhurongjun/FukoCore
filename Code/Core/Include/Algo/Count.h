#pragma once
#include <CoreConfig.h>
#include <CoreType.h>

namespace Fuko::Algo
{
	// 计数 
	template <typename TR, typename TV>
	FORCEINLINE size_t Count(const TR& Input, const TV& InValue)
	{
		size_t Result = 0;
		for (const auto& Value : Input)
		{
			if (Value == InValue) ++Result; 
		}
		return Result;
	}

	// 条件计数 
	template <typename InT, typename TPred>
	FORCEINLINE size_t CountIf(const InT& Input, TPred&& Pred)
	{
		size_t Result = 0;
		for (const auto& Value : Input)
		{
			if (Pred(Value)) ++Result;
		}
		return Result;
	}
}