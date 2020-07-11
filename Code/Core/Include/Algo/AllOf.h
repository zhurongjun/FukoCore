#pragma once
#include <CoreConfig.h>
#include <CoreType.h>

namespace Fuko::Algo
{
	// 所有元素都是true 
	template <typename TRang, typename TProj = NoMap>
	FORCEINLINE bool AllOf(const TRang& Input, TProj&& Proj = TProj())
	{
		for (const auto& Elem : Input)
		{
			if (!(bool)Proj(Elem))
			{
				return false;
			}
		}
		return true;
	}

	// 所有元素都是false 
	template <typename TRang, typename TProj = NoMap>
	FORCEINLINE bool NoneOf(const TRang& Input, TProj&& Proj = TProj())
	{
		for (const auto& Elem : Input)
		{
			if ((bool)Proj(Elem))
			{
				return false;
			}
		}

		return true;
	}

	// 任意一元素是true
	template <typename TRang, typename TProj = NoMap>
	FORCEINLINE bool AnyOf(const TRang& Input, TProj&& Proj = TProj()) 
	{ 
		return !NoneOf(Range,std::forward<TProj>(Proj)); 
	}
}