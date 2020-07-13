#pragma once
#include <CoreConfig.h>
#include <CoreType.h>

namespace Fuko::Algo
{
	// 最大公约数 
	template<typename T>
	FORCEINLINE T GCD(T A, T B)
	{
		while (B != 0)
		{
			T Temp = B;
			B = A % B;
			A = Temp;
		}
		return A;
	}

	// 最小公倍数
	template<typename T>
	FORCEINLINE T LCM(T A, T B)
	{
		T LocGCD = GCD(A, B);
		return A * B / LocGCD;
	}
}