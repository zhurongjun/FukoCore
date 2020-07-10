#pragma once
#include <CoreConfig.h>
#include <CoreType.h>

namespace Fuko::Algo
{
	template<typename SizeType>
	FORCEINLINE SizeType CalculateNumWords(SizeType NumBits)
	{
		check(NumBits >= 0);
		return FMath::DivideAndRoundUp(NumBits, NumBitsPerDWORD);
	}

	template<typename SizeType>
	FORCEINLINE uint32 GetLastWordMask(SizeType NumBits)
	{
		check(NumBits >= 0);
		const uint32 UnusedBits = (NumBitsPerDWORD - NumBits % NumBitsPerDWORD) % NumBitsPerDWORD;
		return FullMask >> UnusedBits;
	}

	template<typename SizeType>
	FORCEINLINE void SetWords(uint32* Words, SizeType NumWords, bool Value)
	{
		if (NumWords > 8)
		{
			Memset(Words, Value ? 0xff : 0, NumWords * sizeof(uint32));
		}
		else
		{
			uint32 Word = Value ? FullMask : EmptyMask;
			for (int32 Idx = 0; Idx < NumWords; ++Idx)
			{
				Words[Idx] = Word;
			}
		}
	}

	template<typename SizeType>
	FORCEINLINE SizeType FindBit(const uint32* Data, SizeType Num, bool Value)
	{
		const SizeType DwordCount = CalculateNumWords(Num);
		SizeType DwordIndex = 0;

		// test to skip head 
		const uint32 Test = Value ? EmptyMask : FullMask;

		// skip head 
		while (DwordIndex < DwordCount && Data[DwordIndex] == Test) ++DwordIndex;

		// now find bit 
		if (DwordIndex < DwordCount)
		{
			// reserve for CountTrailingZeros
			const uint32 Bits = Value ? (Data[DwordIndex]) : ~(Data[DwordIndex]);
			check(Bits != 0);
			const SizeType LowestBitIndex = FMath::CountTrailingZeros(Bits) + (DwordIndex << NumBitsPerDWORDLogTwo);

			if (LowestBitIndex < Num) return LowestBitIndex;
		}
		return INDEX_NONE;
	}

	template<typename SizeType>
	FORCEINLINE SizeType FindAndSetFirstZeroBit(uint32* Data, SizeType Num, SizeType ConservativeStartIndex)
	{
		const SizeType DwordCount = Algo::CalculateNumWords(Num);
		SizeType DwordIndex = FMath::DivideAndRoundDown(ConservativeStartIndex, NumBitsPerDWORD);
		while (DwordIndex < DwordCount && Data[DwordIndex] == (uint32)-1)
		{
			++DwordIndex;
		}

		if (DwordIndex < DwordCount)
		{
			const uint32 Bits = ~(Data[DwordIndex]);
			check(Bits != 0);
			const uint32 LowestBit = (Bits) & (-(int32)Bits);
			const SizeType LowestBitIndex = FMath::CountTrailingZeros(Bits) + (DwordIndex << NumBitsPerDWORDLogTwo);
			if (LowestBitIndex < Num)
			{
				Data[DwordIndex] |= LowestBit;
				return LowestBitIndex;
			}
		}

		return INDEX_NONE;
	}

	template<typename SizeType>
	FORCEINLINE SizeType FindLastBit(const uint32* Data, SizeType Num, bool Value)
	{
		SizeType DwordIndex = Algo::CalculateNumWords(Num) - 1;
		uint32 Mask = FullMask >> Algo::GetLastWordMask(Num);

		const uint32 Test = Value ? EmptyMask : FullMask;

		// skip tail 
		if ((Data[DwordIndex] & Mask) == (Test & Mask))
		{
			--DwordIndex;
			Mask = FullMask;
			while (DwordIndex >= 0 && Data[DwordIndex] == Test) --DwordIndex;
		}

		// now find bit index 
		if (DwordIndex >= 0)
		{
			const uint32 Bits = (Value ? Data[DwordIndex] : ~Data[DwordIndex]) & Mask;
			check(Bits != 0);
			uint32 BitIndex = (NumBitsPerDWORD - 1) - FMath::CountLeadingZeros(Bits);
			SizeType Result = BitIndex + (DwordIndex << NumBitsPerDWORDLogTwo);
			return Result;
		}
		return INDEX_NONE;
	}

	template<typename SizeType>
	FORCEINLINE SizeType FindAndSetLastZeroBit(uint32* Data, SizeType Num)
	{
		// Get the correct mask for the last word
		uint32 SlackIndex = ((Num - 1) % NumBitsPerDWORD) + 1;
		uint32 Mask = FullMask >> (NumBitsPerDWORD - SlackIndex);

		// Iterate over the array until we see a word with a zero bit.
		SizeType DwordIndex = Algo::CalculateNumWords(Num);
		for (;;)
		{
			if (DwordIndex == 0)
			{
				return INDEX_NONE;
			}
			--DwordIndex;
			if ((Data[DwordIndex] & Mask) != Mask)
			{
				break;
			}
			Mask = FullMask;
		}

		// Flip the bits, then we only need to find the first one bit -- easy.
		const uint32 Bits = ~Data[DwordIndex] & Mask;
		check(Bits != 0);

		uint32 BitIndex = (NumBitsPerDWORD - 1) - FMath::CountLeadingZeros(Bits);
		Data[DwordIndex] |= 1u << BitIndex;

		SizeType Result = BitIndex + (DwordIndex << NumBitsPerDWORDLogTwo);
		return Result;
	}

	template<typename SizeType>
	FORCEINLINE void SetBitRange(uint32* InData, SizeType Index, SizeType Num, bool Value)
	{
		if (Num == 0) return;

		// calculate dword index and count  
		SizeType StartIndex = Index / NumBitsPerDWORD;
		SizeType Count = (Index + Num + (NumBitsPerDWORD - 1)) / NumBitsPerDWORD - StartIndex;

		// calculate mask 
		uint32 StartMask = FullMask << (Index % NumBitsPerDWORD);
		uint32 EndMask = FullMask >> (NumBitsPerDWORD - (Index + Num) % NumBitsPerDWORD) % NumBitsPerDWORD;

		uint32* Data = InData + StartIndex;
		if (Value)
		{
			// set to true 
			if (Count == 1)
			{
				*Data |= StartMask & EndMask;
			}
			else
			{
				*Data++ |= StartMask;
				Count -= 2;	// exclude start and end 
				while (Count != 0)
				{
					*Data++ = FullMask;
					--Count;
				}
				*Data |= EndMask;
			}
		}
		else
		{
			// set to false 
			if (Count == 1)
			{
				*Data &= ~(StartMask & EndMask);
			}
			else
			{
				*Data++ &= ~StartMask;
				Count -= 2;	// exclude start and end 
				while (Count != 0)
				{
					*Data++ = 0;
					--Count;
				}
				*Data &= ~EndMask;
			}
		}
	}

	template<typename SizeType>
	FORCEINLINE void SetBit(uint32* InData, SizeType Index, bool Value)
	{
		if (Value)
		{
			InData[Index >> NumBitsPerDWORDLogTwo] |= (1 << (Index & PerDWORDMask));
		}
		else
		{
			InData[Index >> NumBitsPerDWORDLogTwo] &= ~(1 << (Index & PerDWORDMask));
		}
	}

	template<typename SizeType>
	FORCEINLINE bool GetBit(uint32* InData, SizeType Index)
	{
		return (InData[Index >> NumBitsPerDWORDLogTwo] & (1 << (Index & PerDWORDMask))) != 0;
	}
}