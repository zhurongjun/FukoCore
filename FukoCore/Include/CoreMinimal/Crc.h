#pragma once
#include "CoreType.h"
#include "Assert.h"
#include <String/Char.h>

namespace Fuko::Crc
{
	/** lookup table with precalculated CRC values - slicing by 8 implementation */
	static uint32 CRCTablesSB8[8][256];

	/** initializes the CRC lookup table. Must be called before any of the
		CRC functions are used. */
	static void Init();

	/** generates CRC hash of the memory area */
	static uint32 MemCrc32(const void* Data, int32 Length, uint32 CRC = 0);

	/** generates CRC hash of the element */
	template <typename T>
	static uint32 TypeCrc32(const T& Data, uint32 CRC = 0)
	{
		return MemCrc32(&Data, sizeof(T), CRC);
	}

	/** String CRC. */
	template <typename CharType>
	static uint32 StrCrc32(const CharType* Data, uint32 CRC = 0)
	{
		if constexpr (sizeof(CharType) == 1)
		{
			CRC = ~CRC;
			while (CharType Ch = *Data++)
			{
				CRC = (CRC >> 8) ^ CRCTablesSB8[0][(CRC ^ Ch) & 0xFF];
				CRC = (CRC >> 8) ^ CRCTablesSB8[0][(CRC) & 0xFF];
				CRC = (CRC >> 8) ^ CRCTablesSB8[0][(CRC) & 0xFF];
				CRC = (CRC >> 8) ^ CRCTablesSB8[0][(CRC) & 0xFF];
			}
			return ~CRC;
		}
		else
		{
			static_assert(sizeof(CharType) <= 4, "StrCrc32 only works with CharType up to 32 bits.");

			CRC = ~CRC;
			while (CharType Ch = *Data++)
			{
				CRC = (CRC >> 8) ^ CRCTablesSB8[0][(CRC ^ Ch) & 0xFF];
				Ch >>= 8;
				CRC = (CRC >> 8) ^ CRCTablesSB8[0][(CRC ^ Ch) & 0xFF];
				Ch >>= 8;
				CRC = (CRC >> 8) ^ CRCTablesSB8[0][(CRC ^ Ch) & 0xFF];
				Ch >>= 8;
				CRC = (CRC >> 8) ^ CRCTablesSB8[0][(CRC ^ Ch) & 0xFF];
			}
			return ~CRC;
		}
	}
};