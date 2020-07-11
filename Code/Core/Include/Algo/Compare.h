#pragma once
#include <CoreConfig.h>
#include <CoreType.h>

namespace Fuko::Algo
{
	// 比较 
	template <typename TA, typename TB, typename TPred = TEqualTo<>>
	constexpr bool Compare(TA&& InputA, TB&& InputB, TPred&& Pred = TPred())
	{
		const auto SizeA = GetNum(InputA);
		const auto SizeB = GetNum(InputB);

		if (SizeA != SizeB) return false;

		auto A = GetData(InputA);
		auto B = GetData(InputB);

		for (auto i = SizeA; i; --i)
		{
			if (!Pred(*A++, *B++)) return false;
		}

		return true;
	}
}