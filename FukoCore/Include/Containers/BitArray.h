#pragma once
#include "CoreConfig.h"
#include "CoreType.h"
#include <Math/MathUtility.h>
#include "CoreMinimal/Assert.h"
#include <Containers/Allocators.h>
#include "AllocatorsPmr.h"

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
		static constexpr size_t NumBitsPerDWORD = 32;
		static constexpr size_t NumBitsPerDWORDLogTwo = 5;
		FORCEINLINE explicit FRelativeBitReference(size_t BitIndex)
			: DWORDIndex(BitIndex >> NumBitsPerDWORDLogTwo)
			, Mask(1 << (BitIndex & (NumBitsPerDWORD - 1)))
		{
		}

		size_t  DWORDIndex;
		uint32	Mask;
	};
}

// BitArray
namespace Fuko
{
	class BitArray final
	{
		static constexpr size_t NumBitsPerDWORD = 32;
		static constexpr size_t NumBitsPerDWORDLogTwo = 5;
		static constexpr uint32 EmptyMask = 0u;
		static constexpr uint32 FullMask = ~EmptyMask;

		FORCEINLINE static size_t CalculateNumWords(size_t NumBits)
		{
			check(NumBits >= 0);
			return FMath::DivideAndRoundUp(NumBits, NumBitsPerDWORD);
		}
		FORCEINLINE uint32 GetLastWordMask() const
		{
			const uint32 UnusedBits = (NumBitsPerDWORD - m_NumBits % NumBitsPerDWORD) % NumBitsPerDWORD;
			return ~0u >> UnusedBits;
		}
		FORCEINLINE static void SetWords(uint32* Words, size_t NumWords, bool bValue)
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

		FORCEINLINE void ResizeTo(size_t InNum)
		{
			const size_t PrevNumDWORDs = CalculateNumWords(m_MaxBits);
			const size_t MaxDWORDs = CalculateNumWords(InNum);

			if (m_Data) m_Data = (uint32*)m_Allocator->Realloc(m_Data, MaxDWORDs * sizeof(uint32));
			else m_Data = (uint32*)m_Allocator->Alloc(MaxDWORDs * sizeof(uint32));

			if (MaxDWORDs > PrevNumDWORDs)
				Memzero(m_Data + PrevNumDWORDs, (MaxDWORDs - PrevNumDWORDs) * sizeof(uint32));
			m_MaxBits = MaxDWORDs * NumBitsPerDWORD;
		}
	private:
		IAllocator* m_Allocator;
		uint32*		m_Data;
		size_t		m_NumBits;
		size_t		m_MaxBits;
	public:
		friend class TConstSetBitIterator;
		friend class TConstDualSetBitIterator;

		// construct 
		BitArray(IAllocator* Alloc = DefaultAllocator())
			: m_Allocator(Alloc)
			, m_Data(nullptr)
			, m_NumBits(0)
			, m_MaxBits(0)
		{
			check(m_Allocator != nullptr);
		}
		FORCEINLINE explicit BitArray(bool bValue, int32 InNumBits, IAllocator* Alloc = DefaultAllocator())
			: m_Allocator(Alloc)
			, m_Data(nullptr)
			, m_NumBits(0)
			, m_MaxBits(0)
		{
			check(m_Allocator != nullptr);
			Init(bValue, InNumBits);
		}

		// copy construct 
		FORCEINLINE BitArray(const BitArray& Other, IAllocator* Alloc = DefaultAllocator())
			: m_Allocator(Alloc)
			, m_Data(nullptr)
			, m_NumBits(0)
			, m_MaxBits(0)
		{
			check(m_Allocator != nullptr);
			Empty(Other.m_NumBits);
			m_NumBits = Other.m_NumBits;
			if (Other.m_NumBits)
			{
				Memcpy(GetData(), Other.GetData(), CalculateNumWords(m_NumBits) * sizeof(uint32));
			}
		}

		// move construct 
		FORCEINLINE BitArray(BitArray&& Other)
			: m_Allocator(Other.m_Allocator)
			, m_Data(Other.m_Data)
			, m_NumBits(Other.m_NumBits)
			, m_MaxBits(Other.m_MaxBits)
		{
			check(m_Allocator != nullptr);
		}

		// assign operator
		FORCEINLINE BitArray& operator=(BitArray&& Other)
		{
			if (this == &Other) return *this;

			// free memory 
			if (m_Data) m_Allocator->Free(m_Data);

			// copy data 
			m_Data = Other.m_Data;
			m_NumBits = Other.m_NumBits;
			m_MaxBits = Other.m_MaxBits;
			m_Allocator = Other.m_Allocator;

			// invalidate other 
			Other.m_NumBits = 0;
			Other.m_MaxBits = 0;
		}
		FORCEINLINE BitArray& operator=(const BitArray& Other)
		{
			if (this == &Other) return *this;

			Empty(Other.Num());
			m_NumBits = Other.m_NumBits;
			if (m_NumBits)
			{
				Memcpy(GetData(), Other.GetData(), CalculateNumWords(m_NumBits) * sizeof(uint32));
			}
			return *this;
		}

		// destruct 
		FORCEINLINE ~BitArray()
		{
			if (m_Data) m_Allocator->Free(m_Data);
			m_NumBits = 0;
			m_MaxBits = 0;
		}

		// compare operator 
		FORCEINLINE bool operator==(const BitArray& Other) const
		{
			if (Num() != Other.Num()) return false;
			return Memcmp(GetData(), Other.GetData(), CalculateNumWords(m_NumBits) * sizeof(uint32)) == 0;
		}
		FORCEINLINE bool operator!=(const BitArray& Other)
		{
			return !(*this == Other);
		}
		FORCEINLINE bool operator<(const BitArray& Other) const
		{
			if (Num() != Other.Num()) return Num() < Other.Num();

			size_t NumWords = CalculateNumWords(m_NumBits);
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

		// get information
		FORCEINLINE const uint32* GetData() const { return m_Data; }
		FORCEINLINE uint32* GetData() { return m_Data; }
		FORCEINLINE size_t Num() const { return m_NumBits; }
		FORCEINLINE size_t Max() const { return m_MaxBits; }
		FORCENOINLINE IAllocator* GetAllocator() { return m_Allocator; }
		FORCENOINLINE const IAllocator* GetAllocator() const { return m_Allocator; }

		// set num & empty & reserve 
		void Empty(size_t ExpectedNumBits = 0)
		{
			ExpectedNumBits = CalculateNumWords(ExpectedNumBits) * NumBitsPerDWORD;
			const size_t InitialMaxBits = 0;

			m_NumBits = 0;

			// resize 
			if (ExpectedNumBits > m_MaxBits || m_MaxBits > InitialMaxBits)
			{
				ResizeTo(FMath::Max(ExpectedNumBits, InitialMaxBits));
			}
		}
		void Reserve(size_t Number)
		{
			if (Number > m_MaxBits)
			{
				const size_t MaxDWORDs = m_Allocator->GetGrow(
					CalculateNumWords(Number),
					CalculateNumWords(m_MaxBits),
					sizeof(uint32)
				);
				ResizeTo(MaxDWORDs * NumBitsPerDWORD);
			}
		}
		void Reset()
		{
			SetWords(GetData(), CalculateNumWords(m_NumBits), false);
			m_NumBits = 0;
		}
		void SetNumUninitialized(size_t InNumBits)
		{
			m_NumBits = InNumBits;

			if (InNumBits > m_MaxBits)
			{
				ResizeTo(InNumBits);
			}
		}

		// operator []
		FORCEINLINE FBitReference operator[](size_t Index)
		{
			check(Index >= 0 && Index < m_NumBits);
			return FBitReference(
				GetData()[Index / NumBitsPerDWORD],
				1 << (Index & (NumBitsPerDWORD - 1))
			);
		}
		FORCEINLINE const FConstBitReference operator[](size_t Index) const
		{
			check(Index >= 0 && Index < m_NumBits);
			return FConstBitReference(
				GetData()[Index / NumBitsPerDWORD],
				1 << (Index & (NumBitsPerDWORD - 1))
			);
		}

		// add 
		size_t Add(const bool Value)
		{
			const size_t Index = m_NumBits;

			Reserve(Index + 1);
			++m_NumBits;
			(*this)[Index] = Value;

			return Index;
		}
		size_t Add(const bool Value, size_t NumToAdd)
		{
			const size_t Index = m_NumBits;

			if (NumToAdd > 0)
			{
				Reserve(Index + NumToAdd);
				m_NumBits += NumToAdd;
				for (size_t It = Index, End = It + NumToAdd; It != End; ++It)
				{
					(*this)[It] = Value;
				}
			}

			return Index;
		}

		// Init 
		FORCEINLINE void Init(bool bValue, size_t InNumBits)
		{
			m_NumBits = InNumBits;

			const size_t NumWords = CalculateNumWords(m_NumBits);
			const size_t MaxWords = CalculateNumWords(m_MaxBits);

			if (NumWords > MaxWords)
			{
				// Realloc 
				ResizeTo(NumWords * NumBitsPerDWORD);

				uint32* Words = GetData();
				SetWords(Words, NumWords, bValue);
				Words[NumWords - 1] &= GetLastWordMask();
			}
			else if (bValue & (NumWords > 0))
			{
				// set to true
				uint32* Words = GetData();
				SetWords(Words, NumWords - 1, true);
				Words[NumWords - 1] = GetLastWordMask();
				SetWords(Words + NumWords, MaxWords - NumWords, false);
			}
			else
			{
				// set to false 
				SetWords(GetData(), MaxWords, false);
			}
		}

		// validate 
		FORCEINLINE bool IsValidIndex(size_t InIndex) const
		{
			return InIndex >= 0 && InIndex < m_NumBits;
		}

		// remove 
		void RemoveAt(size_t BaseIndex, size_t NumBitsToRemove = 1)
		{
			check(BaseIndex >= 0 && NumBitsToRemove >= 0 && BaseIndex + NumBitsToRemove <= m_NumBits);

			if (BaseIndex + NumBitsToRemove != m_NumBits)
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
			m_NumBits -= NumBitsToRemove;
		}
		void RemoveAtSwap(size_t BaseIndex, size_t NumBitsToRemove = 1)
		{
			check(BaseIndex >= 0 && NumBitsToRemove >= 0 && BaseIndex + NumBitsToRemove <= m_NumBits);
			if (BaseIndex < m_NumBits - NumBitsToRemove)
			{
				for (size_t Index = 0; Index < NumBitsToRemove; Index++)
				{
					(*this)[BaseIndex + Index] = (bool)(*this)[m_NumBits - NumBitsToRemove + Index];
				}
			}
			m_NumBits -= NumBitsToRemove;
		}

		// find 
		size_t Find(bool bValue) const
		{
			// test to skip head 
			const uint32 Test = bValue ? EmptyMask : FullMask;

			const uint32* RESTRICT DwordArray = GetData();
			const size_t DwordCount = CalculateNumWords(m_NumBits);
			size_t DwordIndex = 0;

			// skip head 
			while (DwordIndex < DwordCount && DwordArray[DwordIndex] == Test) ++DwordIndex;

			// now find bit 
			if (DwordIndex < DwordCount)
			{
				// reserve for CountTrailingZeros
				const uint32 Bits = bValue ? (DwordArray[DwordIndex]) : ~(DwordArray[DwordIndex]);
				check(Bits != 0);
				const size_t LowestBitIndex = FMath::CountTrailingZeros(Bits) + (DwordIndex << NumBitsPerDWORDLogTwo);
				
				if (LowestBitIndex < m_NumBits) return LowestBitIndex;
			}
			return INDEX_NONE;
		}
		size_t FindLast(bool bValue) const
		{
			size_t DwordIndex = CalculateNumWords(m_NumBits) - 1;
			const uint32* RESTRICT DwordArray = GetData();
			const uint32 Test = bValue ? EmptyMask : FullMask;
			uint32 Mask = FullMask >> GetLastWordMask();

			// skip tail 
			if ((DwordArray[DwordIndex] & Mask) == (Test & Mask))
			{
				--DwordIndex;
				while (DwordIndex >= 0 && DwordArray[DwordIndex] == Test) --DwordIndex;
			}

			// now find bit index 
			if (DwordIndex >= 0)
			{
				const uint32 Bits = (bValue ? DwordArray[DwordIndex] : ~DwordArray[DwordIndex]) & Mask;
				check(Bits != 0);
				uint32 BitIndex = (NumBitsPerDWORD - 1) - FMath::CountLeadingZeros(Bits);
				size_t Result = BitIndex + (DwordIndex << NumBitsPerDWORDLogTwo);
				return Result;
			}
			return INDEX_NONE;
		}

		// find and set 
		size_t FindAndSetFirstZeroBit(size_t ConservativeStartIndex = 0)
		{
			uint32* RESTRICT DwordArray = GetData();
			const size_t LocalNumBits = m_NumBits;
			const size_t DwordCount = CalculateNumWords(LocalNumBits);
			size_t DwordIndex = FMath::DivideAndRoundDown(ConservativeStartIndex, NumBitsPerDWORD);
			while (DwordIndex < DwordCount && DwordArray[DwordIndex] == (uint32)-1)
			{
				++DwordIndex;
			}

			if (DwordIndex < DwordCount)
			{
				const uint32 Bits = ~(DwordArray[DwordIndex]);
				check(Bits != 0);
				const uint32 LowestBit = (Bits) & (-(int32)Bits);
				const size_t LowestBitIndex = FMath::CountTrailingZeros(Bits) + (DwordIndex << NumBitsPerDWORDLogTwo);
				if (LowestBitIndex < LocalNumBits)
				{
					DwordArray[DwordIndex] |= LowestBit;
					return LowestBitIndex;
				}
			}

			return INDEX_NONE;
		}
		size_t FindAndSetLastZeroBit()
		{
			const size_t LocalNumBits = m_NumBits;

			// Get the correct mask for the last word
			uint32 SlackIndex = ((LocalNumBits - 1) % NumBitsPerDWORD) + 1;
			uint32 Mask = FullMask >> (NumBitsPerDWORD - SlackIndex);

			// Iterate over the array until we see a word with a zero bit.
			size_t DwordIndex = CalculateNumWords(LocalNumBits);
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
				Mask = FullMask;
			}

			// Flip the bits, then we only need to find the first one bit -- easy.
			const uint32 Bits = ~DwordArray[DwordIndex] & Mask;
			check(Bits != 0);

			uint32 BitIndex = (NumBitsPerDWORD - 1) - FMath::CountLeadingZeros(Bits);
			DwordArray[DwordIndex] |= 1u << BitIndex;

			size_t Result = BitIndex + (DwordIndex << NumBitsPerDWORDLogTwo);
			return Result;
		}

		// contains 
		FORCEINLINE bool Contains(bool bValue) const
		{
			return Find(bValue) != INDEX_NONE;
		}

		// set range 
		FORCENOINLINE void SetRange(size_t Index, size_t Num, bool Value)
		{
			check(Index >= 0 && Num >= 0 && Index + Num <= m_NumBits);

			if (Num == 0) return;

			// calculate dword index and count  
			size_t StartIndex = Index / NumBitsPerDWORD;
			size_t Count = (Index + Num + (NumBitsPerDWORD - 1)) / NumBitsPerDWORD - StartIndex;

			// calculate mask 
			uint32 StartMask = FullMask << (Index % NumBitsPerDWORD);
			uint32 EndMask = FullMask >> (NumBitsPerDWORD - (Index + Num) % NumBitsPerDWORD) % NumBitsPerDWORD;

			uint32* Data = GetData() + StartIndex;
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
		
		// access 
		FORCEINLINE FBitReference AccessCorrespondingBit(const FRelativeBitReference& RelativeReference)
		{
			check(RelativeReference.Mask);
			check(RelativeReference.DWORDIndex >= 0);
			check(((uint32)RelativeReference.DWORDIndex + 1) * NumBitsPerDWORD - 1 - FMath::CountLeadingZeros(RelativeReference.Mask) < (uint32)m_NumBits);
			return FBitReference(
				GetData()[RelativeReference.DWORDIndex],
				RelativeReference.Mask
			);
		}
		FORCEINLINE const FConstBitReference AccessCorrespondingBit(const FRelativeBitReference& RelativeReference) const
		{
			check(RelativeReference.Mask);
			check(RelativeReference.DWORDIndex >= 0);
			check(((uint32)RelativeReference.DWORDIndex + 1) * NumBitsPerDWORD - 1 - FMath::CountLeadingZeros(RelativeReference.Mask) < (uint32)m_NumBits);
			return FConstBitReference(
				GetData()[RelativeReference.DWORDIndex],
				RelativeReference.Mask
			);
		}

		// iterator 
		class FIterator : public FRelativeBitReference
		{
		public:
			FORCEINLINE FIterator(BitArray& InArray, size_t StartIndex = 0)
				: FRelativeBitReference(StartIndex)
				, Array(InArray)
				, Index(StartIndex)
			{
			}

			FORCEINLINE FIterator& operator++()
			{
				++Index;
				this->Mask <<= 1;
				
				// Advance to the next uint32.
				if (!this->Mask)
				{
					this->Mask = 1;
					++this->DWORDIndex;
				}
				return *this;
			}
			
			FORCEINLINE explicit operator bool() const
			{
				return Index < Array.Num();
			}

			FORCEINLINE bool operator !() const
			{
				return !(bool)*this;
			}

			FORCEINLINE FBitReference GetValue() const { return FBitReference(Array.GetData()[this->DWORDIndex], this->Mask); }
			FORCEINLINE size_t GetIndex() const { return Index; }
		private:
			BitArray&	Array;
			size_t		Index;
		};

		// const iterator 
		class FConstIterator : public FRelativeBitReference
		{
		public:
			FORCEINLINE FConstIterator(const BitArray& InArray, size_t StartIndex = 0)
				: FRelativeBitReference(StartIndex)
				, Array(InArray)
				, Index(StartIndex)
			{
			}

			FORCEINLINE FConstIterator& operator++()
			{
				++Index;
				this->Mask <<= 1;
				// Advance to the next uint32.
				if (!this->Mask)
				{
					this->Mask = 1;
					++this->DWORDIndex;
				}
				return *this;
			}

			FORCEINLINE explicit operator bool() const
			{
				return Index < Array.Num();
			}

			FORCEINLINE bool operator !() const
			{
				return !(bool)*this;
			}

			FORCEINLINE FConstBitReference GetValue() const { return FConstBitReference(Array.GetData()[this->DWORDIndex], this->Mask); }
			FORCEINLINE size_t GetIndex() const { return Index; }
		private:
			const BitArray& Array;
			size_t			Index;
		};

		// const reverse iterator 
		class FConstReverseIterator : public FRelativeBitReference
		{
		public:
			FORCEINLINE FConstReverseIterator(const BitArray& InArray)
				: FRelativeBitReference(InArray.Num() - 1)
				, Array(InArray)
				, Index(InArray.Num() - 1)
			{
			}

			FORCEINLINE FConstReverseIterator& operator++()
			{
				--Index;
				this->Mask >>= 1;
				
				// Advance to the next uint32.
				if (!this->Mask)
				{
					this->Mask = (1 << (NumBitsPerDWORD - 1));
					--this->DWORDIndex;
				}
				return *this;
			}

			FORCEINLINE explicit operator bool() const
			{
				return Index >= 0;
			}

			FORCEINLINE bool operator !() const
			{
				return !(bool)*this;
			}

			FORCEINLINE FConstBitReference GetValue() const { return FConstBitReference(Array.GetData()[this->DWORDIndex], this->Mask); }
			FORCEINLINE size_t GetIndex() const { return Index; }
		private:
			const BitArray&	Array;
			size_t			Index;
		};
	};
}

// SetBit Iterator，只访问被设为true的bit 
namespace Fuko
{
	class TConstSetBitIterator : public FRelativeBitReference
	{
	public:
		TConstSetBitIterator(const BitArray& InArray, size_t StartIndex = 0)
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

		FORCEINLINE TConstSetBitIterator& operator++()
		{
			UnvisitedBitMask &= ~this->Mask;

			FindFirstSetBit();

			return *this;
		}

		FORCEINLINE friend bool operator==(const TConstSetBitIterator& Lhs, const TConstSetBitIterator& Rhs)
		{
			return Lhs.CurrentBitIndex == Rhs.CurrentBitIndex && &Lhs.Array == &Rhs.Array;
		}

		FORCEINLINE friend bool operator!=(const TConstSetBitIterator& Lhs, const TConstSetBitIterator& Rhs)
		{
			return !(Lhs == Rhs);
		}

		FORCEINLINE explicit operator bool() const
		{
			return CurrentBitIndex < Array.Num();
		}

		FORCEINLINE bool operator !() const
		{
			return !(bool)*this;
		}

		FORCEINLINE size_t GetIndex() const
		{
			return CurrentBitIndex;
		}

	private:
		const BitArray& Array;

		uint32		UnvisitedBitMask;	// mask to skip start bits 
		size_t		CurrentBitIndex;	// current reach bit index
		size_t		BaseBitIndex;		// bit index that we begin to find 

		void FindFirstSetBit()
		{
			const uint32*	ArrayData = Array.GetData();
			const size_t	ArrayNum = Array.Num();
			const size_t	LastDWORDIndex = (ArrayNum - 1) / NumBitsPerDWORD;
		
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

	class TConstDualSetBitIterator : public FRelativeBitReference
	{
	public:

		/** Constructor. */
		FORCEINLINE TConstDualSetBitIterator(
			const BitArray& InArrayA,
			const BitArray& InArrayB,
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
		FORCEINLINE size_t GetIndex() const
		{
			return CurrentBitIndex;
		}

	private:

		const BitArray& ArrayA;
		const BitArray& ArrayB;

		uint32 UnvisitedBitMask;	// mask to skip start bits 
		size_t CurrentBitIndex;		// current reach bit index
		size_t BaseBitIndex;		// bit index that we begin to find 

		/** Find the first bit that is set in both arrays, starting with the current bit, inclusive. */
		void FindFirstSetBit()
		{
			static const uint32 EmptyArrayData = 0;
			const uint32* ArrayDataA = ArrayA.GetData() ? ArrayA.GetData() : &EmptyArrayData;
			const uint32* ArrayDataB = ArrayB.GetData() ? ArrayB.GetData() : &EmptyArrayData;

			// Advance to the next non-zero uint32.
			uint32 RemainingBitMask = ArrayDataA[this->DWORDIndex] & ArrayDataB[this->DWORDIndex] & UnvisitedBitMask;
			while (!RemainingBitMask)
			{
				this->DWORDIndex++;
				BaseBitIndex += NumBitsPerDWORD;
				const size_t LastDWORDIndex = (ArrayA.Num() - 1) / NumBitsPerDWORD;
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
