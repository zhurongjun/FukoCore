#pragma once
#include "CoreConfig.h"
#include "CoreType.h"
#include <Math/MathUtility.h>
#include "CoreMinimal/Assert.h"
#include <Containers/Allocators.h>

// Help function
namespace Fuko::BitArrayHelper
{
	static FORCEINLINE uint32 GetAndClearNextBit(uint32& Mask)
	{
		const uint32 LowestBitMask = (Mask) & (-(int32)Mask);
		const uint32 BitIndex = FMath::FloorLog2(LowestBitMask);
		Mask ^= LowestBitMask;
		return BitIndex;
	}
	static constexpr uint32 BitsPerWord = NumBitsPerDWORD;

	FORCEINLINE static uint32 CalculateNumWords(int32 NumBits)
	{
		check(NumBits >= 0);
		return FMath::DivideAndRoundUp(static_cast<uint32>(NumBits), BitsPerWord);
	}
}

// std::forward declaration
namespace Fuko
{
	template<typename Allocator = FDefaultBitArrayAllocator>
	class TBitArray;

	template<typename Allocator = FDefaultBitArrayAllocator>
	class TConstSetBitIterator;

	template<typename Allocator = FDefaultBitArrayAllocator, typename OtherAllocator = FDefaultBitArrayAllocator>
	class TConstDualSetBitIterator;
}

// Bit reference
namespace Fuko
{
	class FBitReference
	{
	public:

		FORCEINLINE FBitReference(uint32& InData, uint32 InMask)
			: Data(InData)
			, Mask(InMask)
		{}

		FORCEINLINE operator bool() const
		{
			return (Data & Mask) != 0;
		}
		FORCEINLINE void operator=(const bool NewValue)
		{
			if (NewValue)
			{
				Data |= Mask;
			}
			else
			{
				Data &= ~Mask;
			}
		}
		FORCEINLINE void operator|=(const bool NewValue)
		{
			if (NewValue)
			{
				Data |= Mask;
			}
		}
		FORCEINLINE void operator&=(const bool NewValue)
		{
			if (!NewValue)
			{
				Data &= ~Mask;
			}
		}
		FORCEINLINE void AtomicSet(const bool NewValue)
		{
			checkNoEntry();
// 			if (NewValue)
// 			{
// 				if (!(Data & Mask))
// 				{
// 					while (1)
// 					{
// 						uint32 Current = Data;
// 						uint32 Desired = Current | Mask;
// 						if (Current == Desired || FPlatformAtomics::InterlockedCompareExchange((volatile int32*)&Data, (int32)Desired, (int32)Current) == (int32)Current)
// 						{
// 							return;
// 						}
// 					}
// 				}
// 			}
// 			else
// 			{
// 				if (Data & Mask)
// 				{
// 					while (1)
// 					{
// 						uint32 Current = Data;
// 						uint32 Desired = Current & ~Mask;
// 						if (Current == Desired || FPlatformAtomics::InterlockedCompareExchange((volatile int32*)&Data, (int32)Desired, (int32)Current) == (int32)Current)
// 						{
// 							return;
// 						}
// 					}
// 				}
// 			}
		}
		FORCEINLINE FBitReference& operator=(const FBitReference& Copy)
		{
			*this = (bool)Copy;
			return *this;
		}

	private:
		uint32& Data;
		uint32 Mask;
	};

	class FConstBitReference
	{
	public:

		FORCEINLINE FConstBitReference(const uint32& InData, uint32 InMask)
			: Data(InData)
			, Mask(InMask)
		{}

		FORCEINLINE operator bool() const
		{
			return (Data & Mask) != 0;
		}

	private:
		const uint32& Data;
		uint32 Mask;
	};

	class FRelativeBitReference
	{
	public:
		FORCEINLINE explicit FRelativeBitReference(int32 BitIndex)
			: DWORDIndex(BitIndex >> NumBitsPerDWORDLogTwo)
			, Mask(1 << (BitIndex & (NumBitsPerDWORD - 1)))
		{
		}

		int32  DWORDIndex;
		uint32 Mask;
	};
}

// BitArray
namespace Fuko
{
	template<typename Allocator /*= FDefaultBitArrayAllocator*/>
	class TBitArray
	{
		template <typename, typename>
		friend class TScriptBitArray;
	public:
		typedef typename Allocator::template ForElementType<uint32> AllocatorType;

		template<typename>
		friend class TConstSetBitIterator;

		template<typename, typename>
		friend class TConstDualSetBitIterator;

		TBitArray()
			: NumBits(0)
			, MaxBits(AllocatorInstance.GetInitialCapacity() * NumBitsPerDWORD)
		{
			SetWords(GetData(), AllocatorInstance.GetInitialCapacity(), false);
		}

		FORCEINLINE explicit TBitArray(bool bValue, int32 InNumBits)
			: MaxBits(AllocatorInstance.GetInitialCapacity() * NumBitsPerDWORD)
		{
			Init(bValue, InNumBits);
		}

		FORCEINLINE TBitArray(TBitArray&& Other)
		{
			MoveOrCopy(*this, Other);
		}

		FORCEINLINE TBitArray(const TBitArray& Copy)
			: NumBits(0)
			, MaxBits(0)
		{
			*this = Copy;
		}

		FORCEINLINE TBitArray& operator=(TBitArray&& Other)
		{
			if (this != &Other)
			{
				MoveOrCopy(*this, Other);
			}

			return *this;
		}

		FORCEINLINE TBitArray& operator=(const TBitArray& Copy)
		{
			if (this == &Copy)
			{
				return *this;
			}

			Empty(Copy.Num());
			NumBits = Copy.NumBits;
			if (NumBits)
			{
				Memcpy(GetData(), Copy.GetData(), GetNumWords() * sizeof(uint32));
			}
			return *this;
		}

		FORCEINLINE bool operator==(const TBitArray<Allocator>& Other) const
		{
			if (Num() != Other.Num())
			{
				return false;
			}

			return Memcmp(GetData(), Other.GetData(), GetNumWords() * sizeof(uint32)) == 0;
		}

		FORCEINLINE bool operator<(const TBitArray<Allocator>& Other) const
		{
			if (Num() != Other.Num())
			{
				return Num() < Other.Num();
			}

			uint32 NumWords = GetNumWords();
			const uint32* Data0 = GetData();
			const uint32* Data1 = Other.GetData();

			for (uint32 i = 0; i < NumWords; i++)
			{
				if (Data0[i] != Data1[i])
				{
					return Data0[i] < Data1[i];
				}
			}
			return false;
		}

		FORCEINLINE bool operator!=(const TBitArray<Allocator>& Other)
		{
			return !(*this == Other);
		}

	private:
		FORCEINLINE uint32 GetNumWords() const
		{
			return BitArrayHelper::CalculateNumWords(NumBits);
		}

		FORCEINLINE uint32 GetMaxWords() const
		{
			return BitArrayHelper::CalculateNumWords(MaxBits);
		}

		FORCEINLINE uint32 GetLastWordMask() const
		{
			const uint32 UnusedBits = (BitArrayHelper::BitsPerWord - static_cast<uint32>(NumBits) % BitArrayHelper::BitsPerWord) % BitArrayHelper::BitsPerWord;
			return ~0u >> UnusedBits;
		}

		FORCEINLINE static void SetWords(uint32* Words, int32 NumWords, bool bValue)
		{
			if (NumWords > 8)
			{
				Memset(Words, bValue ? 0xff : 0, NumWords * sizeof(uint32));
			}
			else
			{
				uint32 Word = bValue ? ~0u : 0u;
				for (int32 Idx = 0; Idx < NumWords; ++Idx)
				{
					Words[Idx] = Word;
				}
			}
		}

		template <typename BitArrayType>
		static FORCEINLINE void MoveOrCopy(BitArrayType& ToArray, BitArrayType& FromArray)
		{
			ToArray.AllocatorInstance.MoveToEmpty(FromArray.AllocatorInstance);

			ToArray.NumBits = FromArray.NumBits;
			ToArray.MaxBits = FromArray.MaxBits;
			FromArray.NumBits = 0;
			FromArray.MaxBits = 0;
		}

	public:
		int32 Add(const bool Value)
		{
			const int32 Index = NumBits;

			Reserve(Index + 1);
			++NumBits;
			(*this)[Index] = Value;

			return Index;
		}

		int32 Add(const bool Value, int32 NumToAdd)
		{
			const int32 Index = NumBits;

			if (NumToAdd > 0)
			{
				Reserve(Index + NumToAdd);
				NumBits += NumToAdd;
				for (int32 It = Index, End = It + NumToAdd; It != End; ++It)
				{
					(*this)[It] = Value;
				}
			}

			return Index;
		}

		void Empty(int32 ExpectedNumBits = 0)
		{
			ExpectedNumBits = static_cast<int32>(BitArrayHelper::CalculateNumWords(ExpectedNumBits)) * NumBitsPerDWORD;
			const int32 InitialMaxBits = AllocatorInstance.GetInitialCapacity() * NumBitsPerDWORD;

			if (ExpectedNumBits > MaxBits || MaxBits > InitialMaxBits)
			{
				MaxBits = FMath::Max(ExpectedNumBits, InitialMaxBits);
				Realloc(0);
			}
			else
			{
				SetWords(GetData(), GetNumWords(), false);
			}

			NumBits = 0;
		}

		void Reserve(int32 Number)
		{
			if (Number > MaxBits)
			{
				const uint32 MaxDWORDs = AllocatorInstance.CalculateSlackGrow(
					BitArrayHelper::CalculateNumWords(Number),
					GetMaxWords(),
					sizeof(uint32)
				);
				MaxBits = MaxDWORDs * NumBitsPerDWORD;
				Realloc(NumBits);
			}
		}

		void Reset()
		{
			SetWords(GetData(), GetNumWords(), false);

			NumBits = 0;
		}

		FORCEINLINE void Init(bool bValue, int32 InNumBits)
		{
			NumBits = InNumBits;

			const uint32 NumWords = GetNumWords();
			const uint32 MaxWords = GetMaxWords();

			if (NumWords > MaxWords)
			{
				AllocatorInstance.ResizeAllocation(0, NumWords, sizeof(uint32));
				MaxBits = NumWords * NumBitsPerDWORD;

				uint32* Words = GetData();
				SetWords(Words, NumWords, bValue);
				Words[NumWords - 1] &= GetLastWordMask();
			}
			else if (bValue & (NumWords > 0))
			{
				uint32* Words = GetData();
				SetWords(Words, NumWords - 1, true);
				Words[NumWords - 1] = GetLastWordMask();
				SetWords(Words + NumWords, MaxWords - NumWords, false);
			}
			else
			{
				SetWords(GetData(), MaxWords, false);
			}
		}

		void SetNumUninitialized(int32 InNumBits)
		{
			int32 PreviousNumBits = NumBits;
			NumBits = InNumBits;

			if (InNumBits > MaxBits)
			{
				const int32 PreviousNumDWORDs = BitArrayHelper::CalculateNumWords(PreviousNumBits);
				const uint32 MaxDWORDs = AllocatorInstance.CalculateSlackReserve(
					BitArrayHelper::CalculateNumWords(InNumBits), sizeof(uint32));

				AllocatorInstance.ResizeAllocation(PreviousNumDWORDs, MaxDWORDs, sizeof(uint32));

				MaxBits = MaxDWORDs * NumBitsPerDWORD;
			}
		}

		FORCENOINLINE void SetRange(int32 Index, int32 Num, bool Value)
		{
			check(Index >= 0 && Num >= 0 && Index + Num <= NumBits);

			if (Num == 0)
			{
				return;
			}

			// 计算设置uint32的数量
			uint32 StartIndex = Index / NumBitsPerDWORD;
			uint32 Count = (Index + Num + (NumBitsPerDWORD - 1)) / NumBitsPerDWORD - StartIndex;

			// 计算首位的Mask 
			uint32 StartMask = 0xFFFFFFFFu << (Index % NumBitsPerDWORD);
			uint32 EndMask = 0xFFFFFFFFu >> (NumBitsPerDWORD - (Index + Num) % NumBitsPerDWORD) % NumBitsPerDWORD;

			uint32* Data = GetData() + StartIndex;
			if (Value)
			{
				if (Count == 1)
				{
					*Data |= StartMask & EndMask;
				}
				else
				{
					*Data++ |= StartMask;
					Count -= 2;
					while (Count != 0)
					{
						*Data++ = ~0;
						--Count;
					}
					*Data |= EndMask;
				}
			}
			else
			{
				if (Count == 1)
				{
					*Data &= ~(StartMask & EndMask);
				}
				else
				{
					*Data++ &= ~StartMask;
					Count -= 2;
					while (Count != 0)
					{
						*Data++ = 0;
						--Count;
					}
					*Data &= ~EndMask;
				}
			}
		}

		void RemoveAt(int32 BaseIndex, int32 NumBitsToRemove = 1)
		{
			check(BaseIndex >= 0 && NumBitsToRemove >= 0 && BaseIndex + NumBitsToRemove <= NumBits);

			if (BaseIndex + NumBitsToRemove != NumBits)
			{
				// Until otherwise necessary, this is an obviously correct implementation rather than an efficient implementation.
				FIterator WriteIt(*this);
				for (FConstIterator ReadIt(*this); ReadIt; ++ReadIt)
				{
					// 赛跑。。。。 
					// |            B----Num-----
					// |------------xxxxxxxxxxxxx-----------|
					// r									|
					// w									|
					//  --COND1 O---r						|
					//  --COND2 X---w						|
					//				w--COND1 X--     		|
					//				 -----------r			|
					//				 --COND1 O--w           |
					//	                         --COND2 O--r

					// COND1
					if (ReadIt.GetIndex() < BaseIndex || ReadIt.GetIndex() >= BaseIndex + NumBitsToRemove)
					{
						// COND2
						if (WriteIt.GetIndex() != ReadIt.GetIndex())
						{
							WriteIt.GetValue() = (bool)ReadIt.GetValue();
						}
						++WriteIt;
					}
				}
			}
			NumBits -= NumBitsToRemove;
		}

		void RemoveAtSwap(int32 BaseIndex, int32 NumBitsToRemove = 1)
		{
			check(BaseIndex >= 0 && NumBitsToRemove >= 0 && BaseIndex + NumBitsToRemove <= NumBits);
			if (BaseIndex < NumBits - NumBitsToRemove)
			{
				for (int32 Index = 0; Index < NumBitsToRemove; Index++)
				{
					(*this)[BaseIndex + Index] = (bool)(*this)[NumBits - NumBitsToRemove + Index];
				}
			}
			NumBits -= NumBitsToRemove;
		}

		uint32 GetAllocatedSize(void) const
		{
			return BitArrayHelper::CalculateNumWords(MaxBits) * sizeof(uint32);
		}

		int32 Find(bool bValue) const
		{
			const uint32 Test = bValue ? 0u : (uint32)-1;

			const uint32* RESTRICT DwordArray = GetData();
			const int32 LocalNumBits = NumBits;
			const int32 DwordCount = BitArrayHelper::CalculateNumWords(LocalNumBits);
			int32 DwordIndex = 0;
			while (DwordIndex < DwordCount && DwordArray[DwordIndex] == Test)
			{
				++DwordIndex;
			}

			if (DwordIndex < DwordCount)
			{
				const uint32 Bits = bValue ? (DwordArray[DwordIndex]) : ~(DwordArray[DwordIndex]);
				ASSUME(Bits != 0);
				const int32 LowestBitIndex = FMath::CountTrailingZeros(Bits) + (DwordIndex << NumBitsPerDWORDLogTwo);
				if (LowestBitIndex < LocalNumBits)
				{
					return LowestBitIndex;
				}
			}

			return INDEX_NONE;
		}

		int32 FindLast(bool bValue) const
		{
			const int32 LocalNumBits = NumBits;

			uint32 SlackIndex = ((LocalNumBits - 1) % NumBitsPerDWORD) + 1;
			uint32 Mask = ~0u >> (NumBitsPerDWORD - SlackIndex);

			uint32 DwordIndex = BitArrayHelper::CalculateNumWords(LocalNumBits);
			const uint32* RESTRICT DwordArray = GetData();
			const uint32 Test = bValue ? 0u : ~0u;
			for (;;)
			{
				if (DwordIndex == 0)
				{
					return INDEX_NONE;
				}
				--DwordIndex;
				if ((DwordArray[DwordIndex] & Mask) != (Test & Mask))
				{
					break;
				}
				Mask = ~0u;
			}

			const uint32 Bits = (bValue ? DwordArray[DwordIndex] : ~DwordArray[DwordIndex]) & Mask;
			ASSUME(Bits != 0);

			uint32 BitIndex = (NumBitsPerDWORD - 1) - FMath::CountLeadingZeros(Bits);

			int32 Result = BitIndex + (DwordIndex << NumBitsPerDWORDLogTwo);
			return Result;
		}

		FORCEINLINE bool Contains(bool bValue) const
		{
			return Find(bValue) != INDEX_NONE;
		}

		int32 FindAndSetFirstZeroBit(int32 ConservativeStartIndex = 0)
		{
			uint32* RESTRICT DwordArray = GetData();
			const int32 LocalNumBits = NumBits;
			const int32 DwordCount = BitArrayHelper::CalculateNumWords(LocalNumBits);
			int32 DwordIndex = FMath::DivideAndRoundDown(ConservativeStartIndex, NumBitsPerDWORD);
			while (DwordIndex < DwordCount && DwordArray[DwordIndex] == (uint32)-1)
			{
				++DwordIndex;
			}

			if (DwordIndex < DwordCount)
			{
				const uint32 Bits = ~(DwordArray[DwordIndex]);
				UE_ASSUME(Bits != 0);
				const uint32 LowestBit = (Bits) & (-(int32)Bits);
				const int32 LowestBitIndex = FMath::CountTrailingZeros(Bits) + (DwordIndex << NumBitsPerDWORDLogTwo);
				if (LowestBitIndex < LocalNumBits)
				{
					DwordArray[DwordIndex] |= LowestBit;
					return LowestBitIndex;
				}
			}

			return INDEX_NONE;
		}

		int32 FindAndSetLastZeroBit()
		{
			const int32 LocalNumBits = NumBits;

			// Get the correct mask for the last word
			uint32 SlackIndex = ((LocalNumBits - 1) % NumBitsPerDWORD) + 1;
			uint32 Mask = ~0u >> (NumBitsPerDWORD - SlackIndex);

			// Iterate over the array until we see a word with a zero bit.
			uint32 DwordIndex = BitArrayHelper::CalculateNumWords(LocalNumBits);
			uint32* RESTRICT DwordArray = GetData();
			for (;;)
			{
				if (DwordIndex == 0)
				{
					return INDEX_NONE;
				}
				--DwordIndex;
				if ((DwordArray[DwordIndex] & Mask) != Mask)
				{
					break;
				}
				Mask = ~0u;
			}

			// Flip the bits, then we only need to find the first one bit -- easy.
			const uint32 Bits = ~DwordArray[DwordIndex] & Mask;
			ASSUME(Bits != 0);

			uint32 BitIndex = (NumBitsPerDWORD - 1) - FMath::CountLeadingZeros(Bits);
			DwordArray[DwordIndex] |= 1u << BitIndex;

			int32 Result = BitIndex + (DwordIndex << NumBitsPerDWORDLogTwo);
			return Result;
		}

		FORCEINLINE bool IsValidIndex(int32 InIndex) const
		{
			return InIndex >= 0 && InIndex < NumBits;
		}

		FORCEINLINE int32 Num() const { return NumBits; }
		FORCEINLINE FBitReference operator[](int32 Index)
		{
			check(Index >= 0 && Index < NumBits);
			return FBitReference(
				GetData()[Index / NumBitsPerDWORD],
				1 << (Index & (NumBitsPerDWORD - 1))
			);
		}
		FORCEINLINE const FConstBitReference operator[](int32 Index) const
		{
			check(Index >= 0 && Index < NumBits);
			return FConstBitReference(
				GetData()[Index / NumBitsPerDWORD],
				1 << (Index & (NumBitsPerDWORD - 1))
			);
		}
		FORCEINLINE FBitReference AccessCorrespondingBit(const FRelativeBitReference& RelativeReference)
		{
			check(RelativeReference.Mask);
			check(RelativeReference.DWORDIndex >= 0);
			check(((uint32)RelativeReference.DWORDIndex + 1) * NumBitsPerDWORD - 1 - FMath::CountLeadingZeros(RelativeReference.Mask) < (uint32)NumBits);
			return FBitReference(
				GetData()[RelativeReference.DWORDIndex],
				RelativeReference.Mask
			);
		}
		FORCEINLINE const FConstBitReference AccessCorrespondingBit(const FRelativeBitReference& RelativeReference) const
		{
			check(RelativeReference.Mask);
			check(RelativeReference.DWORDIndex >= 0);
			check(((uint32)RelativeReference.DWORDIndex + 1) * NumBitsPerDWORD - 1 - FMath::CountLeadingZeros(RelativeReference.Mask) < (uint32)NumBits);
			return FConstBitReference(
				GetData()[RelativeReference.DWORDIndex],
				RelativeReference.Mask
			);
		}

		/** BitArray iterator. */
		class FIterator : public FRelativeBitReference
		{
		public:
			FORCEINLINE FIterator(TBitArray<Allocator>& InArray, int32 StartIndex = 0)
				: FRelativeBitReference(StartIndex)
				, Array(InArray)
				, Index(StartIndex)
			{
			}
			FORCEINLINE FIterator& operator++()
			{
				++Index;
				this->Mask <<= 1;
				if (!this->Mask)
				{
					// Advance to the next uint32.
					this->Mask = 1;
					++this->DWORDIndex;
				}
				return *this;
			}
			/** conversion to "bool" returning true if the iterator is valid. */
			FORCEINLINE explicit operator bool() const
			{
				return Index < Array.Num();
			}
			/** inverse of the "bool" operator */
			FORCEINLINE bool operator !() const
			{
				return !(bool)*this;
			}

			FORCEINLINE FBitReference GetValue() const { return FBitReference(Array.GetData()[this->DWORDIndex], this->Mask); }
			FORCEINLINE int32 GetIndex() const { return Index; }
		private:
			TBitArray<Allocator>& Array;
			int32 Index;
		};

		/** Const BitArray iterator. */
		class FConstIterator : public FRelativeBitReference
		{
		public:
			FORCEINLINE FConstIterator(const TBitArray<Allocator>& InArray, int32 StartIndex = 0)
				: FRelativeBitReference(StartIndex)
				, Array(InArray)
				, Index(StartIndex)
			{
			}
			FORCEINLINE FConstIterator& operator++()
			{
				++Index;
				this->Mask <<= 1;
				if (!this->Mask)
				{
					// Advance to the next uint32.
					this->Mask = 1;
					++this->DWORDIndex;
				}
				return *this;
			}

			/** conversion to "bool" returning true if the iterator is valid. */
			FORCEINLINE explicit operator bool() const
			{
				return Index < Array.Num();
			}
			/** inverse of the "bool" operator */
			FORCEINLINE bool operator !() const
			{
				return !(bool)*this;
			}

			FORCEINLINE FConstBitReference GetValue() const { return FConstBitReference(Array.GetData()[this->DWORDIndex], this->Mask); }
			FORCEINLINE int32 GetIndex() const { return Index; }
		private:
			const TBitArray<Allocator>& Array;
			int32 Index;
		};

		/** Const reverse iterator. */
		class FConstReverseIterator : public FRelativeBitReference
		{
		public:
			FORCEINLINE FConstReverseIterator(const TBitArray<Allocator>& InArray)
				: FRelativeBitReference(InArray.Num() - 1)
				, Array(InArray)
				, Index(InArray.Num() - 1)
			{
			}
			FORCEINLINE FConstReverseIterator& operator++()
			{
				--Index;
				this->Mask >>= 1;
				if (!this->Mask)
				{
					// Advance to the next uint32.
					this->Mask = (1 << (NumBitsPerDWORD - 1));
					--this->DWORDIndex;
				}
				return *this;
			}

			/** conversion to "bool" returning true if the iterator is valid. */
			FORCEINLINE explicit operator bool() const
			{
				return Index >= 0;
			}
			/** inverse of the "bool" operator */
			FORCEINLINE bool operator !() const
			{
				return !(bool)*this;
			}

			FORCEINLINE FConstBitReference GetValue() const { return FConstBitReference(Array.GetData()[this->DWORDIndex], this->Mask); }
			FORCEINLINE int32 GetIndex() const { return Index; }
		private:
			const TBitArray<Allocator>& Array;
			int32 Index;
		};

		FORCEINLINE const uint32* GetData() const
		{
			return (uint32*)AllocatorInstance.GetAllocation();
		}

		FORCEINLINE uint32* GetData()
		{
			return (uint32*)AllocatorInstance.GetAllocation();
		}

	private:
		AllocatorType AllocatorInstance;
		int32         NumBits;
		int32         MaxBits;

		FORCENOINLINE void Realloc(int32 PreviousNumBits)
		{
			const uint32 PreviousNumDWORDs = BitArrayHelper::CalculateNumWords(PreviousNumBits);
			const uint32 MaxDWORDs = BitArrayHelper::CalculateNumWords(MaxBits);

			AllocatorInstance.ResizeAllocation(PreviousNumDWORDs, MaxDWORDs, sizeof(uint32));

			if (MaxDWORDs)
			{
				Memzero((uint32*)AllocatorInstance.GetAllocation() + PreviousNumDWORDs, (MaxDWORDs - PreviousNumDWORDs) * sizeof(uint32));
			}
		}
	};

}

// SetBit Iterator，只访问被设为true的bit 
namespace Fuko
{
	template<typename Allocator>
	class TConstSetBitIterator : public FRelativeBitReference
	{
	public:

		/** Constructor. */
		TConstSetBitIterator(const TBitArray<Allocator>& InArray, int32 StartIndex = 0)
			: FRelativeBitReference(StartIndex)
			, Array(InArray)
			, UnvisitedBitMask((~0U) << (StartIndex & (NumBitsPerDWORD - 1)))
			, CurrentBitIndex(StartIndex)
			, BaseBitIndex(StartIndex & ~(NumBitsPerDWORD - 1))
		{
			check(StartIndex >= 0 && StartIndex <= Array.Num());
			if (StartIndex != Array.Num())
			{
				FindFirstSetBit();
			}
		}

		/** std::forwards iteration operator. */
		FORCEINLINE TConstSetBitIterator& operator++()
		{
			// Mark the current bit as visited.
			UnvisitedBitMask &= ~this->Mask;

			// Find the first set bit that hasn't been visited yet.
			FindFirstSetBit();

			return *this;
		}

		FORCEINLINE friend bool operator==(const TConstSetBitIterator& Lhs, const TConstSetBitIterator& Rhs)
		{
			// We only need to compare the bit index and the array... all the rest of the state is unobservable.
			return Lhs.CurrentBitIndex == Rhs.CurrentBitIndex && &Lhs.Array == &Rhs.Array;
		}

		FORCEINLINE friend bool operator!=(const TConstSetBitIterator& Lhs, const TConstSetBitIterator& Rhs)
		{
			return !(Lhs == Rhs);
		}

		/** conversion to "bool" returning true if the iterator is valid. */
		FORCEINLINE explicit operator bool() const
		{
			return CurrentBitIndex < Array.Num();
		}
		/** inverse of the "bool" operator */
		FORCEINLINE bool operator !() const
		{
			return !(bool)*this;
		}

		/** Index accessor. */
		FORCEINLINE int32 GetIndex() const
		{
			return CurrentBitIndex;
		}

	private:

		const TBitArray<Allocator>& Array;

		uint32 UnvisitedBitMask;
		int32 CurrentBitIndex;
		int32 BaseBitIndex;

		/** Find the first set bit starting with the current bit, inclusive. */
		void FindFirstSetBit()
		{
			const uint32* ArrayData = Array.GetData();
			const int32   ArrayNum = Array.Num();
			const int32   LastDWORDIndex = (ArrayNum - 1) / NumBitsPerDWORD;

			// Advance to the next non-zero uint32.
			uint32 RemainingBitMask = ArrayData[this->DWORDIndex] & UnvisitedBitMask;
			while (!RemainingBitMask)
			{
				++this->DWORDIndex;
				BaseBitIndex += NumBitsPerDWORD;
				if (this->DWORDIndex > LastDWORDIndex)
				{
					// We've advanced past the end of the array.
					CurrentBitIndex = ArrayNum;
					return;
				}

				RemainingBitMask = ArrayData[this->DWORDIndex];
				UnvisitedBitMask = ~0;
			}

			// This operation has the effect of unsetting the lowest set bit of BitMask
			const uint32 NewRemainingBitMask = RemainingBitMask & (RemainingBitMask - 1);

			// This operation XORs the above mask with the original mask, which has the effect
			// of returning only the bits which differ; specifically, the lowest bit
			this->Mask = NewRemainingBitMask ^ RemainingBitMask;

			// If the Nth bit was the lowest set bit of BitMask, then this gives us N
			CurrentBitIndex = BaseBitIndex + NumBitsPerDWORD - 1 - FMath::CountLeadingZeros(this->Mask);

			// If we've accidentally iterated off the end of an array but still within the same DWORD
			// then set the index to the last index of the array
			if (CurrentBitIndex > ArrayNum)
			{
				CurrentBitIndex = ArrayNum;
			}
		}
	};

	template<typename Allocator, typename OtherAllocator>
	class TConstDualSetBitIterator : public FRelativeBitReference
	{
	public:

		/** Constructor. */
		FORCEINLINE TConstDualSetBitIterator(
			const TBitArray<Allocator>& InArrayA,
			const TBitArray<OtherAllocator>& InArrayB,
			int32 StartIndex = 0
		)
			: FRelativeBitReference(StartIndex)
			, ArrayA(InArrayA)
			, ArrayB(InArrayB)
			, UnvisitedBitMask((~0U) << (StartIndex & (NumBitsPerDWORD - 1)))
			, CurrentBitIndex(StartIndex)
			, BaseBitIndex(StartIndex & ~(NumBitsPerDWORD - 1))
		{
			check(ArrayA.Num() == ArrayB.Num());

			FindFirstSetBit();
		}

		/** Advancement operator. */
		FORCEINLINE TConstDualSetBitIterator& operator++()
		{
			check(ArrayA.Num() == ArrayB.Num());

			// Mark the current bit as visited.
			UnvisitedBitMask &= ~this->Mask;

			// Find the first set bit that hasn't been visited yet.
			FindFirstSetBit();

			return *this;

		}

		/** conversion to "bool" returning true if the iterator is valid. */
		FORCEINLINE explicit operator bool() const
		{
			return CurrentBitIndex < ArrayA.Num();
		}
		/** inverse of the "bool" operator */
		FORCEINLINE bool operator !() const
		{
			return !(bool)*this;
		}

		/** Index accessor. */
		FORCEINLINE int32 GetIndex() const
		{
			return CurrentBitIndex;
		}

	private:

		const TBitArray<Allocator>& ArrayA;
		const TBitArray<OtherAllocator>& ArrayB;

		uint32 UnvisitedBitMask;
		int32 CurrentBitIndex;
		int32 BaseBitIndex;

		/** Find the first bit that is set in both arrays, starting with the current bit, inclusive. */
		void FindFirstSetBit()
		{
			static const uint32 EmptyArrayData = 0;
			const uint32* ArrayDataA = IfAThenAElseB(ArrayA.GetData(), &EmptyArrayData);
			const uint32* ArrayDataB = IfAThenAElseB(ArrayB.GetData(), &EmptyArrayData);

			// Advance to the next non-zero uint32.
			uint32 RemainingBitMask = ArrayDataA[this->DWORDIndex] & ArrayDataB[this->DWORDIndex] & UnvisitedBitMask;
			while (!RemainingBitMask)
			{
				this->DWORDIndex++;
				BaseBitIndex += NumBitsPerDWORD;
				const int32 LastDWORDIndex = (ArrayA.Num() - 1) / NumBitsPerDWORD;
				if (this->DWORDIndex <= LastDWORDIndex)
				{
					RemainingBitMask = ArrayDataA[this->DWORDIndex] & ArrayDataB[this->DWORDIndex];
					UnvisitedBitMask = ~0;
				}
				else
				{
					// We've advanced past the end of the array.
					CurrentBitIndex = ArrayA.Num();
					return;
				}
			};

			// We can assume that RemainingBitMask!=0 here.
			check(RemainingBitMask);

			// This operation has the effect of unsetting the lowest set bit of BitMask
			const uint32 NewRemainingBitMask = RemainingBitMask & (RemainingBitMask - 1);

			// This operation XORs the above mask with the original mask, which has the effect
			// of returning only the bits which differ; specifically, the lowest bit
			this->Mask = NewRemainingBitMask ^ RemainingBitMask;

			// If the Nth bit was the lowest set bit of BitMask, then this gives us N
			CurrentBitIndex = BaseBitIndex + NumBitsPerDWORD - 1 - FMath::CountLeadingZeros(this->Mask);
		}
	};
}
