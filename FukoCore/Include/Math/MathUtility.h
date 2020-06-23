#pragma once
#include <new>
#include <wchar.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <stdint.h>
#include <xmmintrin.h>
#include <intrin0.h>
#include "../CoreConfig.h"

namespace Fuko::Math
{


}

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

// 通用数学算法
struct FMathGeneric
{
	// 截断小数
	static CONSTEXPR FORCEINLINE int32_t TruncToInt(float F)
	{
		return (int32_t)F;
	}
	static CONSTEXPR FORCEINLINE float TruncToFloat(float F)
	{
		return (float)TruncToInt(F);
	}
	static CONSTEXPR FORCEINLINE double TruncToDouble(float F)
	{
		return (double)TruncToInt(F);
	}

	// 向下取整
	static FORCEINLINE int32_t FloorToInt(float F)
	{
		return TruncToInt(floorf(F));
	}
	static FORCEINLINE float FloorToFloat(float F)
	{
		return floorf(F);
	}
	static FORCEINLINE double FloorToDouble(double F)
	{
		return floor(F);
	}

	// 四舍五入
	static FORCEINLINE int32_t RoundToInt(float F)
	{
		return FloorToInt(F + 0.5f);
	}
	static FORCEINLINE float RoundToFloat(float F)
	{
		return FloorToFloat(F + 0.5f);
	}
	static FORCEINLINE double RoundToDouble(double F)
	{
		return FloorToDouble(F + 0.5);
	}

	// 向上取整
	static FORCEINLINE int32_t CeilToInt(float F)
	{
		return TruncToInt(ceilf(F));
	}
	static FORCEINLINE float CeilToFloat(float F)
	{
		return ceilf(F);
	}
	static FORCEINLINE double CeilToDouble(double F)
	{
		return ceil(F);
	}

	// 带符号的截取小数位
	static FORCEINLINE float Fractional(float Value)
	{
		return Value - TruncToFloat(Value);
	}
	// 无符号截取小数位
	static FORCEINLINE float Frac(float Value)
	{
		return Value - FloorToFloat(Value);
	}

	// 取整数部分和小数部分
	static FORCEINLINE float Modf(const float InValue, float* OutIntPart)
	{
		return modff(InValue, OutIntPart);
	}
	static FORCEINLINE double Modf(const double InValue, double* OutIntPart)
	{
		return modf(InValue, OutIntPart);
	}

	// 自然常数e的指数
	static FORCEINLINE float Exp(float Value) { return expf(Value); }
	// 2的指数
	static FORCEINLINE float Exp2(float Value) { return powf(2.f, Value); /*exp2f(Value);*/ }
	// 自然对数
	static FORCEINLINE float Loge(float Value) { return logf(Value); }
	// 自定义对数
	static FORCEINLINE float LogX(float Base, float Value) { return Loge(Value) / Loge(Base); }
	// 2为底的对数
	static FORCEINLINE float Log2(float Value) { return Loge(Value) * 1.4426950f; }

	// 浮点取余，返回趋向于0的余数
	static FORCEINLINE float Fmod(float X, float Y)
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

	// 三角函数运算
	static FORCEINLINE float Sin(float Value) { return sinf(Value); }
	static FORCEINLINE float Asin(float Value) { return asinf((Value < -1.f) ? -1.f : ((Value < 1.f) ? Value : 1.f)); }
	static FORCEINLINE float Sinh(float Value) { return sinhf(Value); }
	static FORCEINLINE float Cos(float Value) { return cosf(Value); }
	static FORCEINLINE float Acos(float Value) { return acosf((Value < -1.f) ? -1.f : ((Value < 1.f) ? Value : 1.f)); }
	static FORCEINLINE float Tan(float Value) { return tanf(Value); }
	static FORCEINLINE float Atan(float Value) { return atanf(Value); }
	static float Atan2(float Y, float X);

	// 开方
	static FORCEINLINE float Sqrt(float Value) { return sqrtf(Value); }
	// 次方
	static FORCEINLINE float Pow(float A, float B) { return powf(A, B); }

	// 平方根倒数
	static FORCEINLINE float InvSqrt(float F)
	{
		return 1.0f / sqrtf(F);
	}
	// 快速版本平方根倒数，但是会略微损失精度
	static FORCEINLINE float InvSqrtEst(float F)
	{
		return InvSqrt(F);
	}

	// 浮点是否是NaN(Not a Number)
	static FORCEINLINE bool IsNaN(float A)
	{
		return ((*(uint32_t*)&A) & 0x7FFFFFFF) > 0x7F800000;
	}
	// 浮点是否有限
	static FORCEINLINE bool IsFinite(float A)
	{
		return ((*(uint32_t*)&A) & 0x7F800000) != 0x7F800000;
	}
	// 是否是负的浮点
	static FORCEINLINE bool IsNegativeFloat(const float& A)
	{
		return ((*(uint32_t*)&A) >= (uint32_t)0x80000000); 
	}

	// 随机
	static FORCEINLINE void RandInit(int32_t Seed) { srand(Seed); }
	static FORCEINLINE int32_t Rand() { return rand(); }
	static FORCEINLINE float FRand() { return Rand() / (float)RAND_MAX; }

	// 向下取2的对数
	static FORCEINLINE uint32_t FloorLog2(uint32_t Value)
	{
		// 二分取对数
		uint32_t pos = 0;
		if (Value >= 1 << 16) { Value >>= 16; pos += 16; }
		if (Value >= 1 << 8) { Value >>= 8; pos += 8; }
		if (Value >= 1 << 4) { Value >>= 4; pos += 4; }
		if (Value >= 1 << 2) { Value >>= 2; pos += 2; }
		if (Value >= 1 << 1) { pos += 1; }
		return (Value == 0) ? 0 : pos;
	}
	static FORCEINLINE uint64_t FloorLog2_64(uint64_t Value)
	{
		uint64_t pos = 0;
		if (Value >= 1ull << 32) { Value >>= 32; pos += 32; }
		if (Value >= 1ull << 16) { Value >>= 16; pos += 16; }
		if (Value >= 1ull << 8) { Value >>= 8; pos += 8; }
		if (Value >= 1ull << 4) { Value >>= 4; pos += 4; }
		if (Value >= 1ull << 2) { Value >>= 2; pos += 2; }
		if (Value >= 1ull << 1) { pos += 1; }
		return (Value == 0) ? 0 : pos;
	}

	// 向上取2的对数
	static FORCEINLINE uint32_t CeilLogTwo(uint32_t Arg)
	{
		uint32_t Bitmask = ((uint32_t)(CountLeadingZeros(Arg) << 26)) >> 31;
		return (32 - CountLeadingZeros(Arg - 1)) & (~Bitmask);
	}
	static FORCEINLINE uint64_t CeilLogTwo64(uint64_t Arg)
	{
		uint64_t Bitmask = ((uint64_t)(CountLeadingZeros64(Arg) << 57)) >> 63;
		return (64 - CountLeadingZeros64(Arg - 1)) & (~Bitmask);
	}

	// 计算头部有几个0位
	static FORCEINLINE uint32_t CountLeadingZeros(uint32_t Value)
	{
		if (Value == 0) return 32;
		return 31 - FloorLog2(Value);
	}
	static FORCEINLINE uint64_t CountLeadingZeros64(uint64_t Value)
	{
		if (Value == 0) return 64;
		return 63 - FloorLog2_64(Value);
	}

	// 计算尾部有几个0位
	static FORCEINLINE uint32_t CountTrailingZeros(uint32_t Value)
	{
		if (Value == 0)
		{
			return 32;
		}
		uint32_t Result = 0;
		while ((Value & 1) == 0)
		{
			Value >>= 1;
			++Result;
		}
		return Result;
	}
	static FORCEINLINE uint64_t CountTrailingZeros64(uint64_t Value)
	{
		if (Value == 0)
		{
			return 64;
		}
		uint64_t Result = 0;
		while ((Value & 1) == 0)
		{
			Value >>= 1;
			++Result;
		}
		return Result;
	}

	// 向上Round到2的次方
	static FORCEINLINE uint32_t RoundUpToPowerOfTwo(uint32_t Arg)
	{
		return 1 << CeilLogTwo(Arg);
	}
	static FORCEINLINE uint64_t RoundUpToPowerOfTwo64(uint64_t V)
	{
		return uint64_t(1) << CeilLogTwo64(V);
	}

	// 莫顿码
	static FORCEINLINE uint32_t MortonCode2(uint32_t x)
	{
		x &= 0x0000ffff;
		x = (x ^ (x << 8)) & 0x00ff00ff;
		x = (x ^ (x << 4)) & 0x0f0f0f0f;
		x = (x ^ (x << 2)) & 0x33333333;
		x = (x ^ (x << 1)) & 0x55555555;
		return x;
	}
	static FORCEINLINE uint32_t ReverseMortonCode2(uint32_t x)
	{
		x &= 0x55555555;
		x = (x ^ (x >> 1)) & 0x33333333;
		x = (x ^ (x >> 2)) & 0x0f0f0f0f;
		x = (x ^ (x >> 4)) & 0x00ff00ff;
		x = (x ^ (x >> 8)) & 0x0000ffff;
		return x;
	}
	static FORCEINLINE uint32_t MortonCode3(uint32_t x)
	{
		x &= 0x000003ff;
		x = (x ^ (x << 16)) & 0xff0000ff;
		x = (x ^ (x << 8)) & 0x0300f00f;
		x = (x ^ (x << 4)) & 0x030c30c3;
		x = (x ^ (x << 2)) & 0x09249249;
		return x;
	}
	static FORCEINLINE uint32_t ReverseMortonCode3(uint32_t x)
	{
		x &= 0x09249249;
		x = (x ^ (x >> 2)) & 0x030c30c3;
		x = (x ^ (x >> 4)) & 0x0300f00f;
		x = (x ^ (x >> 8)) & 0xff0000ff;
		x = (x ^ (x >> 16)) & 0x000003ff;
		return x;
	}

	// Select
	static CONSTEXPR FORCEINLINE float FloatSelect(float Comparand, float ValueGEZero, float ValueLTZero)
	{
		return Comparand >= 0.f ? ValueGEZero : ValueLTZero;
	}
	static CONSTEXPR FORCEINLINE double FloatSelect(double Comparand, double ValueGEZero, double ValueLTZero)
	{
		return Comparand >= 0.f ? ValueGEZero : ValueLTZero;
	}

	// 常用
	template< class T >
	static CONSTEXPR FORCEINLINE T Abs(const T A)
	{
		return (A >= (T)0) ? A : -A;
	}
	static FORCEINLINE float Abs(const float A)
	{
		return fabsf(A);
	}
	template< class T >
	static CONSTEXPR FORCEINLINE T Sign(const T A)
	{
		return (A > (T)0) ? (T)1 : ((A < (T)0) ? (T)-1 : (T)0);
	}
	template< class T >
	static CONSTEXPR FORCEINLINE T Max(const T A, const T B)
	{
		return (A >= B) ? A : B;
	}
	template< class T >
	static CONSTEXPR FORCEINLINE T Min(const T A, const T B)
	{
		return (A <= B) ? A : B;
	}

	static FORCEINLINE int32_t CountBits(uint64_t Bits)
	{
		Bits -= (Bits >> 1) & 0x5555555555555555ull;
		Bits = (Bits & 0x3333333333333333ull) + ((Bits >> 2) & 0x3333333333333333ull);
		Bits = (Bits + (Bits >> 4)) & 0x0f0f0f0f0f0f0f0full;
		return (Bits * 0x0101010101010101) >> 56;
	}
};

// Window特化的高性能算法
struct FMathWindows : public FMathGeneric
{
	static FORCEINLINE int32_t TruncToInt(float F)
	{
		return _mm_cvtt_ss2si(_mm_set_ss(F));
	}
	static FORCEINLINE float TruncToFloat(float F)
	{
		return (float)TruncToInt(F); // same as generic implementation, but this will call the faster trunc
	}
	static FORCEINLINE int32_t RoundToInt(float F)
	{
		// Note: the x2 is to workaround the rounding-to-nearest-even-number issue when the fraction is .5
		return _mm_cvt_ss2si(_mm_set_ss(F + F + 0.5f)) >> 1;
	}
	static FORCEINLINE float RoundToFloat(float F)
	{
		return (float)RoundToInt(F);
	}
	static FORCEINLINE int32_t FloorToInt(float F)
	{
		return _mm_cvt_ss2si(_mm_set_ss(F + F - 0.5f)) >> 1;
	}
	static FORCEINLINE float FloorToFloat(float F)
	{
		return (float)FloorToInt(F);
	}
	static FORCEINLINE int32_t CeilToInt(float F)
	{
		// Note: the x2 is to workaround the rounding-to-nearest-even-number issue when the fraction is .5
		return -(_mm_cvt_ss2si(_mm_set_ss(-0.5f - (F + F))) >> 1);
	}
	static FORCEINLINE float CeilToFloat(float F)
	{
		// Note: the x2 is to workaround the rounding-to-nearest-even-number issue when the fraction is .5
		return (float)CeilToInt(F);
	}
	static FORCEINLINE bool IsNaN(float A) { return _isnan(A) != 0; }
	static FORCEINLINE bool IsFinite(float A) { return _finite(A) != 0; }

	static FORCEINLINE float InvSqrt(float F)
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
	static FORCEINLINE float InvSqrtEst(float F)
	{
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

#pragma intrinsic( _BitScanReverse )
	static FORCEINLINE uint32_t FloorLog2(uint32_t Value)
	{
		// 得到最后一个有效位，就是Log向下Log的结果
		unsigned long Log2;
		if (_BitScanReverse(&Log2, Value) != 0)
		{
			return Log2;
		}

		return 0;
	}
	static FORCEINLINE uint32_t CountLeadingZeros(uint32_t Value)
	{
		// 得到最后一个有效位，使用31减去，就是头部有效位
		unsigned long Log2;
		if (_BitScanReverse(&Log2, Value) != 0)
		{
			return 31 - Log2;
		}

		return 32;
	}
	static FORCEINLINE uint32_t CountTrailingZeros(uint32_t Value)
	{
		// 得到第一个有效位就是尾部0的数量
		if (Value == 0)
		{
			return 32;
		}
		unsigned long BitIndex;
		_BitScanForward(&BitIndex, Value);
		return BitIndex;
	}
	static FORCEINLINE uint32_t CeilLogTwo(uint32_t Arg)
	{
		uint32_t Bitmask = ((uint32_t)(CountLeadingZeros(Arg) << 26)) >> 31;
		return (32 - CountLeadingZeros(Arg - 1)) & (~Bitmask);
	}
	static FORCEINLINE uint32_t RoundUpToPowerOfTwo(uint32_t Arg)
	{
		return 1 << CeilLogTwo(Arg);
	}
	static FORCEINLINE uint64_t RoundUpToPowerOfTwo64(uint64_t Arg)
	{
		return uint64_t(1) << CeilLogTwo64(Arg);
	}

#if PLATFORM_64BITS
	static FORCEINLINE uint64 CeilLogTwo64(uint64 Arg)
	{
		int64 Bitmask = ((int64)(CountLeadingZeros64(Arg) << 57)) >> 63;
		return (64 - CountLeadingZeros64(Arg - 1)) & (~Bitmask);
	}
	static FORCEINLINE uint64 CountLeadingZeros64(uint64 Value)
	{
		// Use BSR to return the log2 of the integer
		unsigned long Log2;
		if (_BitScanReverse64(&Log2, Value) != 0)
		{
			return 63 - Log2;
		}

		return 64;
	}
#endif

};

// 最终的Math类
struct FMath : public FMathWindows
{
	// 随机数
	static FORCEINLINE int32_t RandHelper(int32_t A)
	{
		return A > 0 ? Min(TruncToInt(FRand() * A), A - 1) : 0;
	}
	static FORCEINLINE int32_t RandRange(int32_t Min, int32_t Max)
	{
		const int32_t Range = (Max - Min) + 1;
		return Min + RandHelper(Range);
	}
	static FORCEINLINE float RandRange(float InMin, float InMax)
	{
		return FRandRange(InMin, InMax);
	}
	static FORCEINLINE float FRandRange(float InMin, float InMax)
	{
		return InMin + (InMax - InMin) * FRand();
	}
	static FORCEINLINE bool RandBool()
	{
		return (RandRange(0, 1) == 1) ? true : false;
	}

	// TODO: 向量随机


	// 是否在范围内，前闭后开
	template< class U >
	static FORCEINLINE bool IsWithin(const U& TestValue, const U& MinValue, const U& MaxValue)
	{
		return ((TestValue >= MinValue) && (TestValue < MaxValue));
	}
	// 是否在范围内，闭区间
	template< class U >
	static FORCEINLINE bool IsWithinInclusive(const U& TestValue, const U& MinValue, const U& MaxValue)
	{
		return ((TestValue >= MinValue) && (TestValue <= MaxValue));
	}

	// 浮点近似比较
	static FORCEINLINE bool IsNearlyEqual(float A, float B, float ErrorTolerance = SMALL_NUMBER)
	{
		return Abs<float>(A - B) <= ErrorTolerance;
	}
	static FORCEINLINE bool IsNearlyEqual(double A, double B, double ErrorTolerance = SMALL_NUMBER)
	{
		return Abs<double>(A - B) <= ErrorTolerance;
	}
	static FORCEINLINE bool IsNearlyZero(float Value, float ErrorTolerance = SMALL_NUMBER)
	{
		return Abs<float>(Value) <= ErrorTolerance;
	}
	static FORCEINLINE bool IsNearlyZero(double Value, double ErrorTolerance = SMALL_NUMBER)
	{
		return Abs<double>(Value) <= ErrorTolerance;
	}

	// 是否是2的次方
	template <typename T>
	static FORCEINLINE bool IsPowerOfTwo(T Value)
	{
		return ((Value & (Value - 1)) == (T)0);
	}

	// 3数最值
	template< class T >
	static FORCEINLINE T Max3(const T A, const T B, const T C)
	{
		return Max(Max(A, B), C);
	}
	template< class T >
	static FORCEINLINE T Min3(const T A, const T B, const T C)
	{
		return Min(Min(A, B), C);
	}

	// 平方
	template< class T >
	static FORCEINLINE T Square(const T A)
	{
		return A * A;
	}

	// 夹取
	template< class T >
	static FORCEINLINE T Clamp(const T X, const T Min, const T Max)
	{
		return X < Min ? Min : X < Max ? X : Max;
	}

	// 网格式截断，即Floor的泛化
	static FORCEINLINE float GridSnap(float Location, float Grid)
	{
		if (Grid == 0.f) return Location;
		else
		{
			return FloorToFloat((Location + 0.5f*Grid) / Grid)*Grid;
		}
	}
	static FORCEINLINE double GridSnap(double Location, double Grid)
	{
		if (Grid == 0.0)	return Location;
		else
		{
			return FloorToDouble((Location + 0.5*Grid) / Grid)*Grid;
		}
	}

	// Floor,Ceil,Round的泛化版本
	template <class T>
	static FORCEINLINE T DivideAndRoundUp(T Dividend, T Divisor)
	{
		return (Dividend + Divisor - 1) / Divisor;
	}
	template <class T>
	static FORCEINLINE T DivideAndRoundDown(T Dividend, T Divisor)
	{
		return Dividend / Divisor;
	}
	template <class T>
	static FORCEINLINE T DivideAndRoundNearest(T Dividend, T Divisor)
	{
		return (Dividend >= 0)
			? (Dividend + Divisor / 2) / Divisor
			: (Dividend - Divisor / 2 + 1) / Divisor;
	}

	// 同时求Sin和Cos
	static FORCEINLINE void SinCos(float* ScalarSin, float* ScalarCos, float Value)
	{
		// Map Value to y in [-pi,pi], x = 2*pi*quotient + remainder.
		float quotient = (INV_PI*0.5f)*Value;
		if (Value >= 0.0f)
		{
			quotient = (float)((int)(quotient + 0.5f));
		}
		else
		{
			quotient = (float)((int)(quotient - 0.5f));
		}
		float y = Value - (2.0f*PI)*quotient;

		// Map y to [-pi/2,pi/2] with sin(y) = sin(Value).
		float sign;
		if (y > HALF_PI)
		{
			y = PI - y;
			sign = -1.0f;
		}
		else if (y < -HALF_PI)
		{
			y = -PI - y;
			sign = -1.0f;
		}
		else
		{
			sign = +1.0f;
		}

		float y2 = y * y;

		// 11-degree minimax approximation
		*ScalarSin = (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) * y2 + 0.0083333310f) * y2 - 0.16666667f) * y2 + 1.0f) * y;

		// 10-degree minimax approximation
		float p = ((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) * y2 + 0.041666638f) * y2 - 0.5f) * y2 + 1.0f;
		*ScalarCos = sign * p;
	}

	// 快速ArcSin
#define FASTASIN_HALF_PI (1.5707963050f)
	static FORCEINLINE float FastAsin(float Value)
	{
		// Clamp input to [-1,1].
		bool nonnegative = (Value >= 0.0f);
		float x = FMath::Abs(Value);
		float omx = 1.0f - x;
		if (omx < 0.0f)
		{
			omx = 0.0f;
		}
		float root = FMath::Sqrt(omx);
		// 7-degree minimax approximation
		float result = ((((((-0.0012624911f * x + 0.0066700901f) * x - 0.0170881256f) * x + 0.0308918810f) * x - 0.0501743046f) * x + 0.0889789874f) * x - 0.2145988016f) * x + FASTASIN_HALF_PI;
		result *= root;  // acos(|x|)
		// acos(x) = pi - acos(-x) when x < 0, asin(x) = pi/2 - acos(x)
		return (nonnegative ? FASTASIN_HALF_PI - result : result - FASTASIN_HALF_PI);
	}
#undef FASTASIN_HALF_PI

	// 弧度to角度
	template<class T>
	static CONSTEXPR FORCEINLINE auto RadiansToDegrees(T const& RadVal) -> decltype(RadVal * (180.f / PI))
	{
		return RadVal * (180.f / PI);
	}
	// 角度to弧度
	template<class T>
	static CONSTEXPR FORCEINLINE auto DegreesToRadians(T const& DegVal) -> decltype(DegVal * (PI / 180.f))
	{
		return DegVal * (PI / 180.f);
	}

	// 得到角度差，在-180 - 180之间，有符号
	static float FindDeltaAngleDegrees(float A1, float A2)
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
	static float FindDeltaAngleRadians(float A1, float A2)
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
	static float UnwindDegrees(float A)
	{
		while (A > 180.f) A -= 360.f;
		while (A < -180.f) A += 360.f;
		return A;
	}
	// 将弧度化归到-PI - PI之间
	static float UnwindRadians(float A)
	{
		while (A > PI) A -= ((float)PI * 2.0f);
		while (A < -PI) A += ((float)PI * 2.0f);
		return A;
	}

	// 将角0和角1的夹角化归到-180 - 180之间
	void WindRelativeAnglesDegrees(float InAngle0, float& InOutAngle1)
	{
		const float Diff = InAngle0 - InOutAngle1;
		const float AbsDiff = Abs(Diff);
		if (AbsDiff > 180.0f)
		{
			InOutAngle1 += 360.0f * Sign(Diff) * FloorToFloat((AbsDiff / 360.0f) + 0.5f);
		}
	}
};