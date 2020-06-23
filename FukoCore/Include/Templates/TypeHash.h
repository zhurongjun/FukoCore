#pragma once
#include "CoreType.h"
#include "CoreConfig.h"
#include "TypeTraits.h"
#include "UtilityTemp.h"
#include "CoreMinimal/Crc.h"

namespace Fuko
{
	inline uint32 HashCombine(uint32 A, uint32 C)
	{
		uint32 B = 0x9e3779b9;
		A += B;

		A -= B; A -= C; A ^= (C >> 13);
		B -= C; B -= A; B ^= (A << 8);
		C -= A; C -= B; C ^= (B >> 13);
		A -= B; A -= C; A ^= (C >> 12);
		B -= C; B -= A; B ^= (A << 16);
		C -= A; C -= B; C ^= (B >> 5);
		A -= B; A -= C; A ^= (C >> 3);
		B -= C; B -= A; B ^= (A << 10);
		C -= A; C -= B; C ^= (B >> 15);

		return C;
	}

	inline uint32 PointerHash(const void* Key, uint32 C = 0)
	{
		if constexpr (sizeof(void*) == 4)
		{
			uint32 PtrInt = reinterpret_cast<uint32>(Key);
			return HashCombine((uint32)PtrInt, C);
		}
		else
		{
			uint64 PtrInt = reinterpret_cast<uint64>(Key) >> 4;
			return HashCombine((uint32)PtrInt, C);
		}
	}

	inline uint32 GetTypeHash(const uint8 A)
	{
		return A;
	}

	inline uint32 GetTypeHash(const int8 A)
	{
		return A;
	}

	inline uint32 GetTypeHash(const uint16 A)
	{
		return A;
	}

	inline uint32 GetTypeHash(const int16 A)
	{
		return A;
	}

	inline uint32 GetTypeHash(const int32 A)
	{
		return A;
	}

	inline uint32 GetTypeHash(const uint32 A)
	{
		return A;
	}

	inline uint32 GetTypeHash(const uint64 A)
	{
		return (uint32)A + ((uint32)(A >> 32) * 23);
	}

	inline uint32 GetTypeHash(const int64 A)
	{
		return (uint32)A + ((uint32)(A >> 32) * 23);
	}

	inline uint32 GetTypeHash(float Value)
	{
		return *(uint32*)&Value;
	}

	inline uint32 GetTypeHash(double Value)
	{
		return GetTypeHash(*(uint64*)&Value);
	}

	inline uint32 GetTypeHash(const TCHAR* S)
	{
		return Fuko::Crc::StrCrc32(S);
	}

	inline uint32 GetTypeHash(const void* A)
	{
		return PointerHash(A);
	}

	inline uint32 GetTypeHash(void* A)
	{
		return PointerHash(A);
	}

	template <typename EnumType>
	FORCEINLINE  typename TEnableIf_t<TIsEnum_v<EnumType>, uint32> GetTypeHash(EnumType E)
	{
		return GetTypeHash((__underlying_type(EnumType))E);
	}
}
