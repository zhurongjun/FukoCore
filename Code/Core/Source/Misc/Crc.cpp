#include "Misc/Crc.h"
#include "Templates/UtilityTemp.h"
#include "Misc/ByteSwap.h"

enum { Crc32Poly = 0x04c11db7 };

FORCEINLINE uint32 ReverseBits(uint32 Bits)
{
	Bits = (Bits << 16) | (Bits >> 16);
	Bits = ((Bits & 0x00ff00ff) << 8) | ((Bits & 0xff00ff00) >> 8);
	Bits = ((Bits & 0x0f0f0f0f) << 4) | ((Bits & 0xf0f0f0f0) >> 4);
	Bits = ((Bits & 0x33333333) << 2) | ((Bits & 0xcccccccc) >> 2);
	Bits = ((Bits & 0x55555555) << 1) | ((Bits & 0xaaaaaaaa) >> 1);
	return Bits;
}

uint32 Fuko::Crc::MemCrc32(const void* InData, int32 Length, uint32 CRC)
{
	CRC = ~CRC;

	const uint8* __restrict Data = (uint8*)InData;

	// First we need to align to 32-bits
	int32 InitBytes = static_cast<int32>(Align(Data, 4) - Data);

	if (Length > InitBytes)
	{
		Length -= InitBytes;

		for (; InitBytes; --InitBytes)
		{
			CRC = (CRC >> 8) ^ CRCTablesSB8[0][(CRC & 0xFF) ^ *Data++];
		}

		auto Data4 = (const uint32*)Data;
		for (uint32 Repeat = Length / 8; Repeat; --Repeat)
		{
			uint32 V1 = *Data4++ ^ CRC;
			uint32 V2 = *Data4++;
			CRC =
				CRCTablesSB8[7][V1 & 0xFF] ^
				CRCTablesSB8[6][(V1 >> 8) & 0xFF] ^
				CRCTablesSB8[5][(V1 >> 16) & 0xFF] ^
				CRCTablesSB8[4][V1 >> 24] ^
				CRCTablesSB8[3][V2 & 0xFF] ^
				CRCTablesSB8[2][(V2 >> 8) & 0xFF] ^
				CRCTablesSB8[1][(V2 >> 16) & 0xFF] ^
				CRCTablesSB8[0][V2 >> 24];
		}
		Data = (const uint8*)Data4;

		Length %= 8;
	}

	for (; Length; --Length)
	{
		CRC = (CRC >> 8) ^ CRCTablesSB8[0][(CRC & 0xFF) ^ *Data++];
	}

	return ~CRC;
}
#