#pragma once
#include <CoreConfig.h>
#include <CoreType.h>
#include <Templates/UtilityTemp.h>
#include <Templates/Functor.h>

namespace Fuko::Algo
{
	template <typename T, typename TS, typename TV, typename TProj = NoMap>
	FORCEINLINE T* Find(T* Begin, TS Num, const TV& Value, TProj&& Proj = TProj())
	{
		for (auto End = Begin + Num; Begin != End; ++Begin)
		{
			if (Proj(*Begin) == Value) return Begin;
		}
		return nullptr;
	}

	template <typename T, typename TS, typename TProj = NoMap, typename TPred>
	FORCEINLINE T* FindBy(T* Begin, TS Num, TPred&& Pred, TProj&& Proj = TProj())
	{
		for (auto End = Begin + Num; Begin != End; ++Begin)
		{
			if (Pred(Proj(*Begin))) return Begin;
		}
		return nullptr;
	}
	
	template <typename T, typename TS, typename TV, typename TProj = NoMap>
	FORCEINLINE T* FindLast(T* Begin, TS Num, const TV& Value, TProj&& Proj = TProj())
	{
		--Begin;
		for (auto End = Begin + Num; Begin != End; --End)
		{
			if (Proj(*End) == Value) return End;
		}
		return nullptr;
	}

	template <typename T, typename TS, typename TProj = NoMap, typename TPred>
	FORCEINLINE T* FindLastBy(T* Begin, TS Num, TPred&& Pred, TProj&& Proj = TProj())
	{
		--Begin;
		for (auto End = Begin + Num; Begin < End; --End)
		{
			if (Pred(Proj(*End))) return End;
		}
		return nullptr;
	}
}