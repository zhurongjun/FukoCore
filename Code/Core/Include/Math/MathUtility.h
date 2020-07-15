#pragma once
#include <CoreConfig.h>
#include <CoreType.h>
#include <intrin0.h>
#include <math.h>
#include <xmmintrin.h>
#include <float.h>

// constant
namespace Fuko::Math
{
	// PI
	inline constexpr float PI = 3.1415926535897932f;
	inline constexpr float INV_PI = 0.31830988618f;
	inline constexpr float HALF_PI = 1.57079632679f;

	// 极小数
	inline constexpr float SMALL_NUMBER = 1.e-8f;
	inline constexpr float KINDA_SMALL_NUMBER = 1.e-4f;

	// 极大数
	inline constexpr float BIG_NUMBER = 3.4e+38f;

	// 自然常数
	inline constexpr float EULERS_NUMBER = 2.71828182845904523536f;

	// float 最大值
	inline constexpr float MAX_FLT = 3.402823466e+38F;
}

// functions
namespace Fuko::Math
{
	// Trunc, 截断 
	FORCEINLINE int32 TruncToInt(float F) { return (int32)F; }
	FORCEINLINE float TruncToFloat(float F) { return truncf(F); }
	FORCEINLINE double TruncToDouble(float F) { return trunc(F); }

	// Floor, 向下取整 
	FORCEINLINE int32 FloorToInt(float F) { return (int32)floorf(F); }
	FORCEINLINE float FloorToFloat(float F) { return floorf(F); }
	FORCEINLINE double FloorToDouble(double F) { return floor(F); }

	// Ceil, 向上取整 
	FORCEINLINE int32 CeilToInt(float F) { return (int32)ceilf(F); }
	FORCEINLINE float CeilToFloat(float F) { return ceilf(F); }
	FORCEINLINE double CeilToDouble(double F) { return ceil(F); }

	// Round, 四舍五入 
	FORCEINLINE int32 RoundToInt(float F) { return FloorToInt(F + 0.5f); }
	FORCEINLINE float RoundToFloat(float F) { return FloorToFloat(F + 0.5f); }
	FORCEINLINE double RoundToDouble(double F) { return FloorToDouble(F + 0.5); }

	// 有符号小数位 
	FORCEINLINE float Fractional(float Value) { return Value - TruncToFloat(Value); }
	// 无符号小数位 
	FORCEINLINE float Frac(float Value) { return Value - FloorToFloat(Value); }

	// 浮点求余，返回小数部分 
	FORCEINLINE float Modf(const float InValue, float* OutIntPart) { return modff(InValue, OutIntPart); }
	FORCEINLINE double Modf(const double InValue, double* OutIntPart) { return modf(InValue, OutIntPart); }

	// 指数, 乘方, 开根 
	FORCEINLINE float Exp(float Value) { return expf(Value); }
	FORCEINLINE float Exp2(float Value) { return powf(2.f, Value); }
	FORCEINLINE float Pow(float A, float B) { return powf(A, B); }
	FORCEINLINE float Sqrt(float Value) { return sqrtf(Value); }

	// 平方根倒数
	FORCEINLINE float InvSqrt(float F);
	// 快速版本平方根倒数，但是会略微损失精度
	FORCEINLINE float InvSqrtEst(float F);

	// 对数 
	FORCEINLINE float Loge(float Value) { return logf(Value); }
	FORCEINLINE float Log2(float Value) { return Loge(Value) * 1.4426950f; }
	FORCEINLINE float LogX(float Base, float Value) { return Loge(Value) / Loge(Base); }

	// 三角函数 
	FORCEINLINE float Sin(float Value) { return sinf(Value); }
	FORCEINLINE float Asin(float Value) { return asinf((Value < -1.f) ? -1.f : ((Value < 1.f) ? Value : 1.f)); }
	FORCEINLINE float Sinh(float Value) { return sinhf(Value); }
	FORCEINLINE float Cos(float Value) { return cosf(Value); }
	FORCEINLINE float Acos(float Value) { return acosf((Value < -1.f) ? -1.f : ((Value < 1.f) ? Value : 1.f)); }
	FORCEINLINE float Tan(float Value) { return tanf(Value); }
	FORCEINLINE float Atan(float Value) { return atanf(Value); }
	float CORE_API Atan2(float Y, float X);

	// 浮点求余 
	FORCEINLINE float Fmod(float X, float Y);

	// 浮点是否是NaN(Not a Number)
	FORCEINLINE bool IsNaN(float A) { return ((*(uint32*)&A) & 0x7FFFFFFFU) > 0x7F800000U; }
	FORCEINLINE bool IsNaN(double A) { return ((*(uint64*)&A) & 0x7FFFFFFFFFFFFFFFULL) > 0x7FF0000000000000ULL; }
	// 浮点是否有限
	FORCEINLINE bool IsFinite(float A) { return ((*(uint32*)&A) & 0x7F800000U) != 0x7F800000U; }
	FORCEINLINE bool IsFinite(double A) { return ((*(uint64*)&A) & 0x7FF0000000000000ULL) != 0x7FF0000000000000ULL; }
	// 是否是负的浮点
	FORCEINLINE bool IsNegativeFloat(float A) { return ((*(uint32*)&A) >= (uint32)0x80000000); }
	FORCEINLINE bool IsNegativeFloat(double A) { return ((*(uint64*)&A) >= (uint64)0x8000000000000000); }

	// 计算头尾0位  
	FORCEINLINE uint32 CountLeadingZeros(uint32 Value);
	FORCEINLINE uint64 CountLeadingZeros(uint64 Value);
	FORCEINLINE uint32 CountTrailingZeros(uint32 Value);
	FORCEINLINE uint64 CountTrailingZeros(uint64 Value);

	// 向下取2的Log 
	FORCEINLINE uint32 FloorLog2(uint32 Value);
	FORCEINLINE uint64 FloorLog2(uint64 Value);

	// 向上取2的对数
	FORCEINLINE uint32 CeilLogTwo(uint32 Arg);
	FORCEINLINE uint64 CeilLogTwo(uint64 Arg);

	// 向上Round到2的次方
	FORCEINLINE uint32 RoundUpToPowerOfTwo(uint32 Arg);
	FORCEINLINE uint64 RoundUpToPowerOfTwo(uint64 V);

	// 莫顿码
	FORCEINLINE uint32 MortonCode2(uint32 x);
	FORCEINLINE uint32 ReverseMortonCode2(uint32 x);
	FORCEINLINE uint32 MortonCode3(uint32 x);
	FORCEINLINE uint32 ReverseMortonCode3(uint32 x);

	// 常用
	FORCEINLINE float Abs(const float A) { return fabsf(A); }
	template<class T> FORCEINLINE constexpr T Abs(const T A) { return (A >= (T)0) ? A : -A; }
	template<class T> FORCEINLINE constexpr T Sign(const T A) { return (A > (T)0) ? (T)1 : ((A < (T)0) ? (T)-1 : (T)0); }
	template<class T> FORCEINLINE constexpr T Max(const T A, const T B) { return (A >= B) ? A : B; }
	template<class T> FORCEINLINE constexpr T Min(const T A, const T B) { return (A <= B) ? A : B; }
	template<class T> FORCEINLINE constexpr T Max3(const T A, const T B, const T C) { return Max(Max(A, B), C); }
	template<class T> FORCEINLINE constexpr T Min3(const T A, const T B, const T C) { return Min(Min(A, B), C); }
	template<class T> FORCEINLINE constexpr bool IsPowerOfTwo(T Value) { return ((Value & (Value - 1)) == (T)0); }
	template<class T> FORCEINLINE constexpr T Square(const T A) { return A * A; }
	template<class T> FORCEINLINE constexpr T Clamp(const T X, const T Min, const T Max) { return X < Min ? Min : X < Max ? X : Max; }

	// 计算 bits
	FORCEINLINE int32 CountBits(uint64 Bits);

	// 浮点近似比较
	FORCEINLINE bool IsNearlyEqual(float A, float B, float ErrorTolerance = SMALL_NUMBER) { return Abs<float>(A - B) <= ErrorTolerance; }
	FORCEINLINE bool IsNearlyEqual(double A, double B, double ErrorTolerance = SMALL_NUMBER) { return Abs<double>(A - B) <= ErrorTolerance; }
	FORCEINLINE bool IsNearlyZero(float Value, float ErrorTolerance = SMALL_NUMBER) { return Abs<float>(Value) <= ErrorTolerance; }
	FORCEINLINE bool IsNearlyZero(double Value, double ErrorTolerance = SMALL_NUMBER) { return Abs<double>(Value) <= ErrorTolerance; }

	// 对齐到最近的一个Grid的整数倍  
	FORCEINLINE float GridSnap(float Location, float Grid);
	FORCEINLINE double GridSnap(double Location, double Grid);

	// 一般对整数使用 
	template<class T> FORCEINLINE T DivideAndRoundUp(T Dividend, T Divisor) { return (Dividend + Divisor - 1) / Divisor; }
	template<class T> FORCEINLINE T DivideAndRoundDown(T Dividend, T Divisor) { return Dividend / Divisor; }
	template<class T> FORCEINLINE T DivideAndRoundNearest(T Dividend, T Divisor) { return (Dividend >= 0) ? (Dividend + Divisor / 2) / Divisor : (Dividend - Divisor / 2 + 1) / Divisor; }

	// 弧度to角度
	template<class T> FORCEINLINE constexpr decltype(auto) RadiansToDegrees(T const& RadVal) { return RadVal * (180.f / PI); }
	// 角度to弧度
	template<class T> FORCEINLINE constexpr decltype(auto) DegreesToRadians(T const& DegVal) { return DegVal * (PI / 180.f); }

	// 得到角度差，在-180 - 180之间，有符号
	FORCEINLINE float FindDeltaAngleDegrees(float A1, float A2);
	// 得到弧度差，在-PI - PI之间，有符号
	FORCEINLINE float FindDeltaAngleRadians(float A1, float A2);

	// 将角度化归到-180 - 180之间
	FORCEINLINE float UnwindDegrees(float A);
	// 将弧度化归到-PI - PI之间
	FORCEINLINE float UnwindRadians(float A);
	// 将角0和角1的夹角化归到-180 - 180之间
	FORCEINLINE void WindRelativeAnglesDegrees(float InAngle0, float& InOutAngle1);
}

// generic implement 
namespace Fuko::Math
{
	FORCEINLINE float Fmod(float X, float Y)
	{
		if (fabsf(Y) <= 1.e-8f)
		{
			return 0.f;
		}
		const float Quotient = TruncToFloat(X / Y);
		float IntPortion = Y * Quotient;

		// 防止浮点误差导致取整部分大于原数
		if (fabsf(IntPortion) > fabsf(X))
		{
			IntPortion = X;
		}

		const float Result = X - IntPortion;
		return Result;
	}

	// 向上取2的对数
	FORCEINLINE uint32 CeilLogTwo(uint32 Arg)
	{
		uint32 Bitmask = ((uint32)(CountLeadingZeros(Arg) << 26)) >> 31;
		return (32 - CountLeadingZeros(Arg - 1)) & (~Bitmask);
	}
	FORCEINLINE uint64 CeilLogTwo(uint64 Arg)
	{
		uint64 Bitmask = ((uint64)(CountLeadingZeros(Arg) << 57llu)) >> 63;
		return (64 - CountLeadingZeros(Arg - 1)) & (~Bitmask);
	}

	// 向上Round到2的次方
	FORCEINLINE uint32 RoundUpToPowerOfTwo(uint32 Arg)
	{
		return 1 << CeilLogTwo(Arg);
	}
	FORCEINLINE uint64 RoundUpToPowerOfTwo(uint64 V)
	{
		return uint64(1) << CeilLogTwo(V);
	}

	// 莫顿码
	FORCEINLINE uint32 MortonCode2(uint32 x)
	{
		x &= 0x0000ffff;
		x = (x ^ (x << 8)) & 0x00ff00ff;
		x = (x ^ (x << 4)) & 0x0f0f0f0f;
		x = (x ^ (x << 2)) & 0x33333333;
		x = (x ^ (x << 1)) & 0x55555555;
		return x;
	}
	FORCEINLINE uint32 ReverseMortonCode2(uint32 x)
	{
		x &= 0x55555555;
		x = (x ^ (x >> 1)) & 0x33333333;
		x = (x ^ (x >> 2)) & 0x0f0f0f0f;
		x = (x ^ (x >> 4)) & 0x00ff00ff;
		x = (x ^ (x >> 8)) & 0x0000ffff;
		return x;
	}
	FORCEINLINE uint32 MortonCode3(uint32 x)
	{
		x &= 0x000003ff;
		x = (x ^ (x << 16)) & 0xff0000ff;
		x = (x ^ (x << 8)) & 0x0300f00f;
		x = (x ^ (x << 4)) & 0x030c30c3;
		x = (x ^ (x << 2)) & 0x09249249;
		return x;
	}
	FORCEINLINE uint32 ReverseMortonCode3(uint32 x)
	{
		x &= 0x09249249;
		x = (x ^ (x >> 2)) & 0x030c30c3;
		x = (x ^ (x >> 4)) & 0x0300f00f;
		x = (x ^ (x >> 8)) & 0xff0000ff;
		x = (x ^ (x >> 16)) & 0x000003ff;
		return x;
	}

	// 计算 bits
	FORCEINLINE int32 CountBits(uint64 Bits)
	{
		Bits -= (Bits >> 1) & 0x5555555555555555ull;
		Bits = (Bits & 0x3333333333333333ull) + ((Bits >> 2) & 0x3333333333333333ull);
		Bits = (Bits + (Bits >> 4)) & 0x0f0f0f0f0f0f0f0full;
		return (Bits * 0x0101010101010101) >> 56;
	}

	// 向下取2的对数
	FORCEINLINE uint32 FloorLog2(uint32 Value)
	{
		uint32 pos = 0;
		if (Value >= 1 << 16) { Value >>= 16; pos += 16; }
		if (Value >= 1 << 8) { Value >>= 8; pos += 8; }
		if (Value >= 1 << 4) { Value >>= 4; pos += 4; }
		if (Value >= 1 << 2) { Value >>= 2; pos += 2; }
		if (Value >= 1 << 1) { pos += 1; }
		return (Value == 0) ? 0 : pos;
	}
	FORCEINLINE uint64 FloorLog2(uint64 Value)
	{
		uint64 pos = 0;
		if (Value >= 1ull << 32) { Value >>= 32; pos += 32; }
		if (Value >= 1ull << 16) { Value >>= 16; pos += 16; }
		if (Value >= 1ull << 8) { Value >>= 8; pos += 8; }
		if (Value >= 1ull << 4) { Value >>= 4; pos += 4; }
		if (Value >= 1ull << 2) { Value >>= 2; pos += 2; }
		if (Value >= 1ull << 1) { pos += 1; }
		return (Value == 0) ? 0 : pos;
	}

	// 对齐到最近的一个Grid的整数倍  
	FORCEINLINE float GridSnap(float Location, float Grid)
	{
		if (Grid == 0.f) return Location;
		else return FloorToFloat((Location + 0.5f*Grid) / Grid)*Grid;
	}
	FORCEINLINE double GridSnap(double Location, double Grid)
	{
		if (Grid == 0.0)	return Location;
		else return FloorToDouble((Location + 0.5*Grid) / Grid)*Grid;
	}

	// 得到角度差，在-180 - 180之间，有符号
	FORCEINLINE float FindDeltaAngleDegrees(float A1, float A2)
	{
		float Delta = A2 - A1;
		if (Delta > 180.0f)
		{
			Delta = Delta - 360.0f;
		}
		else if (Delta < -180.0f)
		{
			Delta = Delta + 360.0f;
		}
		return Delta;
	}
	// 得到弧度差，在-PI - PI之间，有符号
	FORCEINLINE float FindDeltaAngleRadians(float A1, float A2)
	{
		float Delta = A2 - A1;
		if (Delta > PI)
		{
			Delta = Delta - (PI * 2.0f);
		}
		else if (Delta < -PI)
		{
			Delta = Delta + (PI * 2.0f);
		}
		return Delta;
	}

	// 将角度化归到-180 - 180之间
	FORCEINLINE float UnwindDegrees(float A)
	{
		while (A > 180.f) A -= 360.f;
		while (A < -180.f) A += 360.f;
		return A;
	}
	// 将弧度化归到-PI - PI之间
	FORCEINLINE float UnwindRadians(float A)
	{
		while (A > PI) A -= ((float)PI * 2.0f);
		while (A < -PI) A += ((float)PI * 2.0f);
		return A;
	}

	// 将角0和角1的夹角化归到-180 - 180之间
	FORCEINLINE void WindRelativeAnglesDegrees(float InAngle0, float& InOutAngle1)
	{
		const float Diff = InAngle0 - InOutAngle1;
		const float AbsDiff = Abs(Diff);
		if (AbsDiff > 180.0f)
		{
			InOutAngle1 += 360.0f * Sign(Diff) * FloorToFloat((AbsDiff / 360.0f) + 0.5f);
		}
	}
}

// platform math 
namespace Fuko::Math
{
#pragma intrinsic( _BitScanReverse )
#pragma intrinsic( _BitScanForward )
	FORCEINLINE uint32 CountLeadingZeros(uint32 Value)
	{
		unsigned long Log2;
		if (_BitScanReverse(&Log2, Value) != 0)
		{
			return 31 - Log2;
		}
		return 32;
	}
	FORCEINLINE uint64 CountLeadingZeros(uint64 Value)
	{
		unsigned long Log2;
		if (_BitScanReverse64(&Log2, Value) != 0)
		{
			return 63 - Log2;
		}
		return 64;
	}
	FORCEINLINE uint32 CountTrailingZeros(uint32 Value)
	{
		if (Value == 0)
		{
			return 32;
		}
		unsigned long BitIndex;	// 0-based, where the LSB is 0 and MSB is 31
		_BitScanForward(&BitIndex, Value);	// Scans from LSB to MSB
		return BitIndex;
	}
	FORCEINLINE uint64 CountTrailingZeros(uint64 Value)
	{
		if (Value == 0)
		{
			return 64;
		}
		unsigned long BitIndex;	// 0-based, where the LSB is 0 and MSB is 31
		_BitScanForward64(&BitIndex, Value);	// Scans from LSB to MSB
		return BitIndex;
	}
}

// sse impl
namespace Fuko::Math
{
	FORCEINLINE float InvSqrt(float F)
	{
		const __m128 fOneHalf = _mm_set_ss(0.5f);
		__m128 Y0, X0, X1, X2, FOver2;
		float temp;

		Y0 = _mm_set_ss(F);
		X0 = _mm_rsqrt_ss(Y0);	// 1/sqrt estimate (12 bits)
		FOver2 = _mm_mul_ss(Y0, fOneHalf);

		// 1st Newton-Raphson iteration
		X1 = _mm_mul_ss(X0, X0);
		X1 = _mm_sub_ss(fOneHalf, _mm_mul_ss(FOver2, X1));
		X1 = _mm_add_ss(X0, _mm_mul_ss(X0, X1));

		// 2nd Newton-Raphson iteration
		X2 = _mm_mul_ss(X1, X1);
		X2 = _mm_sub_ss(fOneHalf, _mm_mul_ss(FOver2, X2));
		X2 = _mm_add_ss(X1, _mm_mul_ss(X1, X2));

		_mm_store_ss(&temp, X2);
		return temp;
	}

	FORCEINLINE float InvSqrtEst(float F)
	{
		// Performs one pass of Newton-Raphson iteration on the hardware estimate
		const __m128 fOneHalf = _mm_set_ss(0.5f);
		__m128 Y0, X0, X1, FOver2;
		float temp;

		Y0 = _mm_set_ss(F);
		X0 = _mm_rsqrt_ss(Y0);	// 1/sqrt estimate (12 bits)
		FOver2 = _mm_mul_ss(Y0, fOneHalf);

		// 1st Newton-Raphson iteration
		X1 = _mm_mul_ss(X0, X0);
		X1 = _mm_sub_ss(fOneHalf, _mm_mul_ss(FOver2, X1));
		X1 = _mm_add_ss(X0, _mm_mul_ss(X0, X1));

		_mm_store_ss(&temp, X1);
		return temp;
	}
}