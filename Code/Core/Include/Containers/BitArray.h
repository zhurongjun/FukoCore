#pragma once
#include "CoreConfig.h"
#include "CoreType.h"
#include <Math/MathUtility.h>
#include "CoreMinimal/Assert.h"
#include <Memory/Allocators.h>
#include "Memory/Memory.h"
#include <Containers/Allocator.h>
#include <Algo/Container/BitArray.h>

// forward
namespace Fuko
{
	template<typename Alloc = TPmrAllocator<uint32>>
	class TBitArray;
}

// Bit reference
namespace Fuko
{
	class BitReference
	{
		uint32& Data;
		uint32	Mask;
	public:
		FORCEINLINE BitReference(uint32& InData, uint32 InMask) : Data(InData) , Mask(InMask) {}
		FORCEINLINE operator bool() const { return (Data & Mask) != 0; }
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
		FORCEINLINE BitReference& operator=(const BitReference& Copy)
		{
			*this = (bool)Copy;
			return *this;
		}
	};
	class ConstBitReference
	{
		const uint32&	Data;
		uint32			Mask;
	public:
		FORCEINLINE ConstBitReference(const uint32& InData, uint32 InMask) : Data(InData) , Mask(InMask) {}
		FORCEINLINE operator bool() const { return (Data & Mask) != 0; }
	};
}

// Bit iterator
namespace Fuko
{
	template<typename SizeType>
	class TBitIterator
	{
		uint32*		m_Data;
		SizeType	m_Num;
		uint32		m_Mask;
		SizeType	m_Index;
		SizeType	m_DWORDIndex;
	public:
		template<typename TAlloc>
		FORCEINLINE TBitIterator(TBitArray<TAlloc>& InArray, SizeType StartIndex = 0)
			: m_DWORDIndex(StartIndex >> NumBitsPerDWORDLogTwo)
			, m_Mask(1 << (StartIndex & (NumBitsPerDWORD - 1)))
			, m_Index(StartIndex)
			, m_Data(InArray.GetData())
			, m_Num(InArray.Num())
		{}
		FORCEINLINE TBitIterator(uint32* Data, SizeType Num, SizeType StartIndex = 0)
			: m_DWORDIndex(StartIndex >> NumBitsPerDWORDLogTwo)
			, m_Mask(1 << (StartIndex & (NumBitsPerDWORD - 1)))
			, m_Index(StartIndex)
			, m_Data(Data)
			, m_Num(Num)
		{}
		FORCEINLINE TBitIterator& operator++()
		{
			++m_Index;
			m_Mask <<= 1;

			// Advance to the next uint32.
			if (!m_Mask)
			{
				m_Mask = 1;
				++m_DWORDIndex;
			}
			return *this;
		}
		FORCEINLINE explicit operator bool() const { return m_Index < m_Num; }
		FORCEINLINE bool operator !() const { return !(bool)*this; }

		FORCEINLINE BitReference GetValue() const { return BitReference(m_Data[m_DWORDIndex], m_Mask); }
		FORCEINLINE SizeType GetIndex() const { return m_Index; }
	};
	template<typename SizeType>
	class TConstBitIterator
	{
		const uint32*	m_Data;
		SizeType		m_Num;
		SizeType		m_Index;
		SizeType		m_DWORDIndex;
		uint32			m_Mask;
	public:
		template<typename TAlloc>
		FORCEINLINE TConstBitIterator(const TBitArray<TAlloc>& InArray, SizeType StartIndex = 0)
			: m_DWORDIndex(StartIndex >> NumBitsPerDWORDLogTwo)
			, m_Mask(1 << (StartIndex & (NumBitsPerDWORD - 1)))
			, m_Data(InArray.GetData())
			, m_Num(InArray.GetNum())
			, m_Index(StartIndex)
		{}
		FORCEINLINE TConstBitIterator(const uint32* Data, SizeType Num, SizeType StartIndex = 0)
			: m_DWORDIndex(StartIndex >> NumBitsPerDWORDLogTwo)
			, m_Mask(1 << (StartIndex & (NumBitsPerDWORD - 1)))
			, m_Index(StartIndex)
			, m_Data(Data)
			, m_Num(Num)
		{}

		FORCEINLINE TConstBitIterator& operator++()
		{
			++m_Index;
			m_Mask <<= 1;
			// Advance to the next uint32.
			if (!m_Mask)
			{
				m_Mask = 1;
				++m_DWORDIndex;
			}
			return *this;
		}

		FORCEINLINE explicit operator bool() const { return m_Index < m_Num; }
		FORCEINLINE bool operator !() const { return !(bool)*this; }

		FORCEINLINE ConstBitReference GetValue() const { return ConstBitReference(m_Data[m_DWORDIndex], m_Mask); }
		FORCEINLINE SizeType GetIndex() const { return m_Index; }
	};
}

// Set Bit iterator
namespace Fuko
{
	template<typename SizeType>
	class TConstSetBitIterator
	{
		const uint32*	m_Data;
		SizeType		m_Num;
		SizeType		m_DWORDIndex;			// current bit index 
		uint32			m_Mask;				// current bit mask
		uint32			m_UnvisitedBitMask;	// mask to skip start bits 
		SizeType		m_CurrentBitIndex;	// current reach bit index

		void FindFirstSetBit()
		{
			const uint32*	ArrayData = m_Data;
			const SizeType	ArrayNum = m_Num;
			const SizeType	LastDWORDIndex = (ArrayNum - 1) / NumBitsPerDWORD;

			// skip zero int32 
			uint32 RemainingBitMask = ArrayData[m_DWORDIndex] & m_UnvisitedBitMask;
			while (!RemainingBitMask)
			{
				++m_DWORDIndex;

				// Out of bounds 
				if (m_DWORDIndex > LastDWORDIndex)
				{
					m_CurrentBitIndex = ArrayNum;
					return;
				}

				RemainingBitMask = ArrayData[m_DWORDIndex];
				m_UnvisitedBitMask = FullMask;
			}

			// get bit mask
			const uint32 NewRemainingBitMask = RemainingBitMask & (RemainingBitMask - 1);
			m_Mask = NewRemainingBitMask ^ RemainingBitMask;

			// If the Nth bit was the lowest set bit of BitMask, then this gives us N
			m_CurrentBitIndex = (m_DWORDIndex + 1) * NumBitsPerDWORD - FMath::CountLeadingZeros(m_Mask) - 1;

			// out of bounds
			if (m_CurrentBitIndex > ArrayNum) m_CurrentBitIndex = ArrayNum;
		}
	public:
		template<typename TAlloc>
		TConstSetBitIterator(const TBitArray<TAlloc>& InArray, SizeType StartIndex = 0)
			: m_DWORDIndex(StartIndex >> NumBitsPerDWORDLogTwo)
			, m_Mask(1 << (StartIndex & PerDWORDMask))
			, m_Data(InArray.GetData())
			, m_Num(InArray.Num())
			, m_UnvisitedBitMask(FullMask << (StartIndex & PerDWORDMask))
			, m_CurrentBitIndex(StartIndex)
		{
			check(StartIndex >= 0 && StartIndex <= m_Num);
			if (StartIndex != m_Num) FindFirstSetBit();
		}
		TConstSetBitIterator(const uint32* Data,SizeType Num, SizeType StartIndex = 0)
			: m_DWORDIndex(StartIndex >> NumBitsPerDWORDLogTwo)
			, m_Mask(1 << (StartIndex & PerDWORDMask))
			, m_Data(Data)
			, m_Num(Num)
			, m_UnvisitedBitMask(FullMask << (StartIndex & PerDWORDMask))
			, m_CurrentBitIndex(StartIndex)
		{
			check(StartIndex >= 0 && StartIndex <= m_Num);
			if (StartIndex != m_Num) FindFirstSetBit();
		}

		FORCEINLINE TConstSetBitIterator& operator++()
		{
			// skip current mask 
			m_UnvisitedBitMask &= ~m_Mask;

			// find next mask 
			FindFirstSetBit();
			return *this;
		}

		FORCEINLINE friend bool operator==(const TConstSetBitIterator& Lhs, const TConstSetBitIterator& Rhs) { return Lhs.m_CurrentBitIndex == Rhs.m_CurrentBitIndex && Lhs.m_Data == Rhs.m_Data; }
		FORCEINLINE friend bool operator!=(const TConstSetBitIterator& Lhs, const TConstSetBitIterator& Rhs) { return !(Lhs == Rhs); }
		FORCEINLINE explicit operator bool() const { return m_CurrentBitIndex < m_Num; }
		FORCEINLINE bool operator !() const { return !(bool)*this; }
		FORCEINLINE SizeType GetIndex() const { return m_CurrentBitIndex; }
	};
	template<typename SizeType>
	class TConstDualSetBitIterator
	{
		const uint32*	m_DataA;
		const uint32*	m_DataB;
		SizeType	m_NumA;
		SizeType	m_NumB;

		SizeType	m_DWORDIndex;			// current bit index 
		uint32		m_Mask;				// current bit mask
		uint32		m_UnvisitedBitMask;	// mask to skip start bits 
		SizeType	m_CurrentBitIndex;	// current reach bit index

		void FindFirstSetBit()
		{
			static const uint32 EmptyArrayData = 0;
			const uint32* ArrayDataA = m_DataA ? m_DataA : &EmptyArrayData;
			const uint32* ArrayDataB = m_DataB ? m_DataB : &EmptyArrayData;
			const SizeType ArrayNum = m_NumA;
			const SizeType LastDWORDIndex = (m_NumA - 1) / NumBitsPerDWORD;

			// skip zero int32 
			uint32 RemainingBitMask = ArrayDataA[m_DWORDIndex] & ArrayDataB[m_DWORDIndex] & m_UnvisitedBitMask;
			while (!RemainingBitMask)
			{
				m_DWORDIndex++;

				// out of bounds 
				if (m_DWORDIndex > LastDWORDIndex)
				{
					m_CurrentBitIndex = m_NumA;
					return;
				}
				RemainingBitMask = ArrayDataA[m_DWORDIndex] & ArrayDataB[m_DWORDIndex];
				m_UnvisitedBitMask = FullMask;
			};

			// get bit mask 
			const uint32 NewRemainingBitMask = RemainingBitMask & (RemainingBitMask - 1);
			m_Mask = NewRemainingBitMask ^ RemainingBitMask;

			// set current bit index 
			m_CurrentBitIndex = (m_DWORDIndex + 1) * NumBitsPerDWORD - FMath::CountLeadingZeros(m_Mask) - 1;
		}
	public:
		template<typename TAllocA, typename TAllocB>
		FORCEINLINE TConstDualSetBitIterator(
			const TBitArray<TAllocA>& InArrayA,
			const TBitArray<TAllocB>& InArrayB,
			int32 StartIndex = 0)
			: m_DWORDIndex(StartIndex >> NumBitsPerDWORDLogTwo)
			, m_Mask(1 << (StartIndex & PerDWORDMask))
			, m_DataA(InArrayA.GetData())
			, m_DataB(InArrayB.GetData())
			, m_NumA(InArrayA.Num())
			, m_NumB(InArrayB.Num())
			, m_UnvisitedBitMask(FullMask << (StartIndex & PerDWORDMask))
			, m_CurrentBitIndex(StartIndex)
		{
			check(InArrayA.Num() == InArrayB.Num());
			FindFirstSetBit();
		}
		FORCEINLINE TConstDualSetBitIterator(
			const uint32* DataA,
			SizeType	NumA,
			const uint32* DataB,
			SizeType	NumB,
			int32 StartIndex = 0)
			: m_DWORDIndex(StartIndex >> NumBitsPerDWORDLogTwo)
			, m_Mask(1 << (StartIndex & PerDWORDMask))
			, m_DataA(DataA)
			, m_DataB(DataB)
			, m_NumA(NumA)
			, m_NumB(NumB)
			, m_UnvisitedBitMask(FullMask << (StartIndex & PerDWORDMask))
			, m_CurrentBitIndex(StartIndex)
		{
			check(InArrayA.Num() == InArrayB.Num());
			FindFirstSetBit();
		}

		FORCEINLINE TConstDualSetBitIterator& operator++()
		{
			check(m_NumA == m_NumB);
			// skip current mask 
			m_UnvisitedBitMask &= ~m_Mask;

			// find next mask 
			FindFirstSetBit();
			return *this;
		}
		FORCEINLINE explicit operator bool() const { return m_CurrentBitIndex < m_NumA; }
		FORCEINLINE bool operator !() const { return !(bool)*this; }
		FORCEINLINE SizeType GetIndex() const { return m_CurrentBitIndex; }
	};
}

// TBitArray
namespace Fuko
{
	template<typename Alloc>
	class TBitArray final
	{
	public:
		using SizeType = typename Alloc::SizeType;
		using AllocType = Alloc;
	private:
		FORCEINLINE void ResizeGrow()
		{
			SizeType NewWord;
			if (m_NumBits > m_MaxBits)
			{
				NewWord = m_Allocator.GetGrow((m_NumBits >> NumBitsPerDWORDLogTwo) + 1, m_MaxBits >> NumBitsPerDWORDLogTwo);
				m_MaxBits = m_Allocator.Reserve(m_Data, NewWord) << NumBitsPerDWORDLogTwo;
			}
		}
		FORCEINLINE void ResizeTo(SizeType InNum)
		{
			const SizeType PrevNumDWORDs = Algo::CalculateNumWords(m_MaxBits);
			SizeType MaxDWORDs = Algo::CalculateNumWords(InNum);

			// realloc
			MaxDWORDs = m_Allocator.Reserve(m_Data, MaxDWORDs);

			// clean new memory 
			if (MaxDWORDs > PrevNumDWORDs) Memzero(GetData() + PrevNumDWORDs, (MaxDWORDs - PrevNumDWORDs) * sizeof(uint32));
			
			// change max 
			m_MaxBits = MaxDWORDs * NumBitsPerDWORD;
		}
	private:
		AllocType	m_Allocator;
		uint32*		m_Data;
		SizeType	m_NumBits;
		SizeType	m_MaxBits;
	public:
		friend class ConstSetBitIterator;
		friend class ConstDualSetBitIterator;

		// construct 
		TBitArray(AllocType&& InAlloc = AllocType())
			: m_Allocator(std::move(InAlloc))
			, m_Data(nullptr)
			, m_NumBits(0)
			, m_MaxBits(InAlloc.GetCount(0) * NumBitsPerDWORD)
		{}
		FORCEINLINE explicit TBitArray(bool bValue, int32 InNumBits, AllocType&& InAlloc = AllocType())
			: m_Allocator(std::move(InAlloc))
			, m_Data(nullptr)
			, m_NumBits(0)
			, m_MaxBits(InAlloc.GetCount(0) * NumBitsPerDWORD)
		{
			Init(bValue, InNumBits);
		}

		// copy construct 
		FORCEINLINE TBitArray(const TBitArray& Other, AllocType&& InAlloc = AllocType())
			: m_Allocator(std::move(InAlloc))
			, m_Data(nullptr)
			, m_NumBits(0)
			, m_MaxBits(InAlloc.GetCount(0) * NumBitsPerDWORD)
		{
			ResizeTo(Other.m_NumBits);
			m_NumBits = Other.m_NumBits;
			
			// copy data 
			if (Other.m_NumBits) Memcpy(GetData(), Other.GetData(), Algo::CalculateNumWords(m_NumBits) * sizeof(uint32));
		}

		// move construct 
		FORCEINLINE TBitArray(TBitArray&& Other)
			: m_Allocator(std::move(Other.m_Allocator))
			, m_Data(Other.m_Data)
			, m_NumBits(Other.m_NumBits)
			, m_MaxBits(Other.m_MaxBits)
		{
			Other.m_NumBits = Other.m_MaxBits = 0;
			Other.m_Data = nullptr;
		}

		// assign operator
		FORCEINLINE TBitArray& operator=(TBitArray&& Other)
		{
			if (this == &Other) return *this;

			// copy data 
			m_NumBits = Other.m_NumBits;
			m_MaxBits = Other.m_MaxBits;
			m_Data = Other.m_Data;
			m_Allocator = std::move(Other.m_Allocator);

			// invalidate other 
			Other.m_NumBits = 0;
			Other.m_MaxBits = 0;
			Other.m_Data = nullptr;
		}
		FORCEINLINE TBitArray& operator=(const TBitArray& Other)
		{
			if (this == &Other) return *this;

			ResizeTo(Other.Num());
			m_NumBits = Other.m_NumBits;
			if (m_NumBits) Memcpy(GetData(), Other.GetData(), Algo::CalculateNumWords(m_NumBits) * sizeof(uint32));
			return *this;
		}

		// destruct 
		FORCEINLINE ~TBitArray()
		{
			m_MaxBits = (m_Allocator.Free(m_Data)) << NumBitsPerDWORDLogTwo;
			m_NumBits = 0;
		}

		// compare operator 
		FORCEINLINE bool operator==(const TBitArray& Other) const
		{
			if (Num() != Other.Num()) return false;
			return Memcmp(GetData(), Other.GetData(), Algo::CalculateNumWords(m_NumBits) * sizeof(uint32)) == 0;
		}
		FORCEINLINE bool operator!=(const TBitArray& Other)
		{
			return !(*this == Other);
		}
		FORCEINLINE bool operator<(const TBitArray& Other) const
		{
			if (Num() != Other.Num()) return Num() < Other.Num();

			SizeType NumWords = Algo::CalculateNumWords(m_NumBits);
			const uint32* Data0 = GetData();
			const uint32* Data1 = Other.GetData();

			for (SizeType i = 0; i < NumWords; i++)
			{
				if (Data0[i] != Data1[i])
				{
					return Data0[i] < Data1[i];
				}
			}
			return false;
		}

		// get information
		FORCEINLINE uint32* GetData() { return m_Data; }
		FORCEINLINE const uint32* GetData() const { return m_Data; }
		FORCEINLINE SizeType Num() const { return m_NumBits; }
		FORCEINLINE SizeType Max() const { return m_MaxBits; }
		FORCENOINLINE AllocType& GetAllocator() { return m_Allocator; }
		FORCENOINLINE const AllocType& GetAllocator() const { return m_Allocator; }

		// set num & empty & reserve 
		void Empty(SizeType ExpectedNumBits = 0)
		{
			m_NumBits = 0;
			if (ExpectedNumBits != m_MaxBits) ResizeTo(ExpectedNumBits);
		}
		void Reserve(SizeType Number)
		{
			if (Number > m_MaxBits) ResizeTo(Number);
		}
		void Reset(SizeType ExpectedNumBits = 0)
		{
			Algo::SetWords(GetData(), Algo::CalculateNumWords(m_NumBits), false);
			if (ExpectedNumBits > m_MaxBits) ResizeTo(ExpectedNumBits);
			m_NumBits = 0;
		}
		void SetNumUninitialized(SizeType InNumBits)
		{
			m_NumBits = InNumBits;
			ResizeGrow();
		}

		// operator []
		FORCEINLINE BitReference operator[](SizeType Index)
		{
			check(Index >= 0 && Index < m_NumBits);
			return BitReference(
				GetData()[Index >> NumBitsPerDWORDLogTwo],
				1 << (Index & PerDWORDMask)
			);
		}
		FORCEINLINE const ConstBitReference operator[](SizeType Index) const
		{
			check(Index >= 0 && Index < m_NumBits);
			return ConstBitReference(
				GetData()[Index >> NumBitsPerDWORDLogTwo],
				1 << (Index & PerDWORDMask)
			);
		}

		// add 
		SizeType Add(const bool Value)
		{
			const SizeType Index = m_NumBits;
			
			++m_NumBits;
			ResizeGrow();

			(*this)[Index] = Value;
			return Index;
		}
		SizeType Add(const bool Value, SizeType NumToAdd)
		{
			const SizeType Index = m_NumBits;

			if (NumToAdd > 0)
			{
				m_NumBits += NumToAdd;
				ResizeGrow();

				for (SizeType It = Index, End = It + NumToAdd; It != End; ++It)
				{
					(*this)[It] = Value;
				}
			}

			return Index;
		}

		// Init 
		FORCEINLINE void Init(bool bValue, SizeType InNumBits)
		{
			m_NumBits = InNumBits;

			const SizeType NumWords = Algo::CalculateNumWords(m_NumBits);
			const SizeType MaxWords = Algo::CalculateNumWords(m_MaxBits);

			if (NumWords > MaxWords)
			{
				// Realloc 
				ResizeTo(NumWords * NumBitsPerDWORD);

				uint32* Words = GetData();
				Algo::SetWords(Words, NumWords, bValue);
				Words[NumWords - 1] &= Algo::GetLastWordMask(m_NumBits);
			}
			else if (bValue & (NumWords > 0))
			{
				// set to true
				uint32* Words = GetData();
				Algo::SetWords(Words, NumWords - 1, true);
				Words[NumWords - 1] = Algo::GetLastWordMask(m_NumBits);
				Algo::SetWords(Words + NumWords, MaxWords - NumWords, false);
			}
			else
			{
				// set to false 
				Algo::SetWords(GetData(), MaxWords, false);
			}
		}

		// validate 
		FORCEINLINE bool IsValidIndex(SizeType InIndex) const
		{
			return InIndex >= 0 && InIndex < m_NumBits;
		}

		// remove 
		void RemoveAt(SizeType BaseIndex, SizeType NumBitsToRemove = 1)
		{
			check(BaseIndex >= 0 && NumBitsToRemove >= 0 && BaseIndex + NumBitsToRemove <= m_NumBits);

			if (BaseIndex + NumBitsToRemove != m_NumBits)
			{
				// Until otherwise necessary, this is an obviously correct implementation rather than an efficient implementation.
				Iterator WriteIt(*this);
				for (ConstIterator ReadIt(*this); ReadIt; ++ReadIt)
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
		void RemoveAtSwap(SizeType BaseIndex, SizeType NumBitsToRemove = 1)
		{
			check(BaseIndex >= 0 && NumBitsToRemove >= 0 && BaseIndex + NumBitsToRemove <= m_NumBits);
			if (BaseIndex < m_NumBits - NumBitsToRemove)
			{
				for (SizeType Index = 0; Index < NumBitsToRemove; Index++)
				{
					(*this)[BaseIndex + Index] = (bool)(*this)[m_NumBits - NumBitsToRemove + Index];
				}
			}
			m_NumBits -= NumBitsToRemove;
		}

		// find 
		SizeType Find(bool bValue) const { return Algo::FindBit(GetData(), m_NumBits, bValue); }
		SizeType FindLast(bool bValue) const { return Algo::FindLastBit(GetData(), m_NumBits, bValue); }

		// find and set 
		SizeType FindAndSetFirstZeroBit(SizeType ConservativeStartIndex = 0) { return Algo::FindAndSetFirstZeroBit(GetData(), m_NumBits, ConservativeStartIndex); }
		SizeType FindAndSetLastZeroBit() { return Algo::FindAndSetLastZeroBit(GetData(), m_NumBits); }

		// contains 
		FORCEINLINE bool Contains(bool bValue) const { return Find(bValue) != INDEX_NONE; }

		// set range 
		FORCENOINLINE void SetRange(SizeType Index, SizeType Num, bool Value)
		{
			check(Index >= 0 && Num >= 0 && Index + Num <= m_NumBits);
			Algo::SetBitRange(GetData(), Index, Num, Value);
		}
	
		// iterator 
		using Iterator = TBitIterator<SizeType>;
		using ConstIterator = TBitIterator<SizeType>;
	
		// set iterator
		using ConstSetBitIterator = TConstSetBitIterator<SizeType>;
		using ConstDualSetBitIterator = TConstDualSetBitIterator<SizeType>;
	};
}