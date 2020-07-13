#pragma once
#include <CoreConfig.h>
#include "Templates/TypeTraits.h"
#include "Math/MathUtility.h"
#include "Templates/UtilityTemp.h"
#include "Algo/Sort.h"
#include "CoreMinimal/Assert.h"
#include "Allocator.h"
#include <Algo/Find.h>
#include <Algo/Sort.h>
#include <Algo/StableSort.h>

// Array
namespace Fuko
{
	template<typename T, typename Alloc = PmrAllocator>
	class TArray
	{
		using SizeType = typename Alloc::USizeType;
	protected:
		Alloc		m_Allocator;
		T*			m_Data;
		SizeType	m_Num;
		SizeType	m_Max;

		static constexpr SizeType ElementSize = (SizeType)sizeof(T);
	private:
		FORCENOINLINE void _FreeArray()
		{
			DestructItems(GetData(), m_Num);
			m_Num = 0;
			m_Max = m_Allocator.Free(m_Data);
		}
		FORCENOINLINE void _ResizeGrow()
		{
			if (m_Num > m_Max)
			{
				SizeType NewMax = m_Allocator.GetGrow(m_Num, m_Max);
				m_Max = m_Allocator.Reserve(m_Data, NewMax);
			}
		}
		FORCENOINLINE void _ResizeShrink()
		{
			auto NewMax = m_Allocator.GetShrink(m_Num, m_Max);
			if (NewMax < m_Max) m_Max = m_Allocator.Reserve(m_Data, NewMax);
		}
		FORCENOINLINE void _ResizeTo(SizeType NewMax)
		{
			if (m_Max != NewMax) m_Max = m_Allocator.Reserve(m_Data, NewMax);
		}
		template <typename T2>
		void _CopyToEmpty(const T2* OtherData, SizeType OtherNum, SizeType ExtraSlack)
		{
			check(ExtraSlack >= 0);

			m_Num = OtherNum;

			if (OtherNum || ExtraSlack)
			{
				SizeType NewMax = OtherNum + ExtraSlack;
				
				if (NewMax > m_Max) _ResizeTo(NewMax);
				ConstructItems<T>(GetData(), OtherData, OtherNum);
			}
		}
	public:
		// construct 
		FORCEINLINE TArray(const Alloc& InAlloc = Alloc())
			: m_Num(0)
			, m_Data(nullptr)
			, m_Max(0)
			, m_Allocator(InAlloc)
		{}
		FORCEINLINE TArray(const T* Ptr, SizeType Count, const Alloc& InAlloc = Alloc())
			: m_Num(0)
			, m_Data(nullptr)
			, m_Max(0)
			, m_Allocator(InAlloc)
		{
			check(Ptr != nullptr || Count == 0);
			_CopyToEmpty(Ptr, Count, 0);
		}
		FORCEINLINE TArray(std::initializer_list<T> InitList, const Alloc& InAlloc = Alloc())
			: m_Num(0)
			, m_Data(nullptr)
			, m_Max(0)
			, m_Allocator(InAlloc)
		{
			_CopyToEmpty(InitList.begin(), (SizeType)InitList.size(), 0);
		}

		// copy construct 
		template <typename TOther>
		FORCEINLINE TArray(const TArray<TOther>& Other, const Alloc& InAlloc = Alloc())
			: m_Num(0)
			, m_Data(nullptr)
			, m_Max(0)
			, m_Allocator(std::move(InAlloc))
		{
			_CopyToEmpty(Other.GetData(), Other.Num(), 0);
		}
		FORCEINLINE TArray(const TArray& Other, const Alloc& InAlloc = Alloc())
			: m_Num(0)
			, m_Data(nullptr)
			, m_Max(0)
			, m_Allocator(std::move(InAlloc))
		{
			_CopyToEmpty(Other.GetData(), Other.Num(), 0);
		}
		FORCEINLINE TArray(const TArray& Other, SizeType ExtraSlack, Alloc&& InAlloc = Alloc())
			: m_Num(0)
			, m_Data(nullptr)
			, m_Max(0)
			, m_Allocator(std::move(InAlloc))
		{
			_CopyToEmpty(Other.GetData(), Other.Num(), ExtraSlack);
		}

		// assign operator 
		FORCEINLINE TArray& operator=(std::initializer_list<T> InitList)
		{
			DestructItems(GetData(), m_Num);
			_CopyToEmpty(InitList.begin(), (SizeType)InitList.size(), 0);
			return *this;
		}
		FORCEINLINE TArray& operator=(const TArray& Other)
		{
			if (this != &Other)
			{
				DestructItems(GetData(), m_Num);
				_CopyToEmpty(Other.GetData(), Other.Num(), 0);
			}
			return *this;
		}
		
		// move construct 
		FORCEINLINE TArray(TArray&& Other)
			: m_Num(Other.m_Num)
			, m_Data(Other.m_Data)
			, m_Max(Other.m_Max)
			, m_Allocator(std::move(Other.m_Allocator))
		{
			Other.m_Num = 0;
			Other.m_Max = 0;
			Other.m_Data = nullptr;
		}
	
		// move assign operator 
		FORCEINLINE TArray& operator=(TArray&& Other)
		{
			if (this == &Other) return *this;
			
			_FreeArray();

			// copy info 
			m_Allocator = std::move(Other.m_Allocator);
			m_Num = Other.m_Num;
			m_Max = Other.m_Max;
			m_Data = Other.m_Data;

			// invalidate other 
			Other.m_Num = Other.m_Max = 0;
			Other.m_Data = nullptr;
			return *this;
		}
		
		// destruct 
		FORCEINLINE ~TArray() { _FreeArray(); }
		
		// get infomation 
		FORCEINLINE T* GetData() { return m_Data; }
		FORCEINLINE const T* GetData() const { return m_Data; }
		FORCEINLINE SizeType Num() const { return m_Num; }
		FORCEINLINE SizeType Max() const { return m_Max; }
		FORCEINLINE SizeType Slack() const { return m_Max - m_Num; }
		FORCEINLINE const Alloc& GetAllocator() const { return m_Allocator; }
		FORCEINLINE Alloc& GetAllocator() { return m_Allocator; }

		// compare
		FORCEINLINE bool operator==(const TArray& Rhs) const { return m_Num == Rhs.m_Num && CompareItems(GetData(), Rhs.GetData(), m_Num); }
		FORCEINLINE bool operator!=(const TArray& OtherArray) const { return !(*this == OtherArray); }

		// debug check
		FORCEINLINE void CheckInvariants() const { check((m_Num >= 0) & (m_Max >= m_Num)); }
		FORCEINLINE void CheckAddress(const T* Addr) const
		{
			checkf(Addr < GetData() || Addr >= (GetData() + m_Max),
				TEXT("Attempting to use a container element (%p) which already comes from the container being modified (%p, m_Max: %d, m_Num: %d, SizeofElement: %d)!")
				, Addr, GetData(), m_Max, m_Num, ElementSize);
		}
		FORCEINLINE void RangeCheck(SizeType Index) const
		{
			CheckInvariants();
			checkf((Index >= 0) & (Index < m_Num), TEXT("Array index out of bounds: %i from an array of size %i"), Index, m_Num);
		}
		
		// runtime check 
		FORCEINLINE bool IsValidIndex(SizeType Index) const { return Index >= 0 && Index < m_Num; }

		// operator []
		FORCEINLINE T& operator[](SizeType Index)
		{
			RangeCheck(Index);
			return GetData()[Index];
		}
		FORCEINLINE const T& operator[](SizeType Index) const
		{
			RangeCheck(Index);
			return GetData()[Index];
		}

		// get element 
		FORCEINLINE T& Last(SizeType IndexFromTheEnd = 0)
		{
			RangeCheck(m_Num - IndexFromTheEnd - 1);
			return GetData()[m_Num - IndexFromTheEnd - 1];
		}
		FORCEINLINE const T& Last(SizeType IndexFromTheEnd = 0) const
		{
			RangeCheck(m_Num - IndexFromTheEnd - 1);
			return GetData()[m_Num - IndexFromTheEnd - 1];
		}

		// pop & push 
		FORCEINLINE T Pop(bool bAllowShrinking = true)
		{
			RangeCheck(0);
			T Result = std::move(GetData()[m_Num - 1]);
			RemoveAt(m_Num - 1, 1, bAllowShrinking);
			return Result;
		}
		FORCEINLINE void Push(T&& Item) { Emplace(std::move(Item)); }
		FORCEINLINE void Push(const T& Item) { Emplace(Item); }

		// shrink & reserve 
		FORCEINLINE void Shrink()
		{
			CheckInvariants();
			if (m_Max != m_Num)
			{
				_ResizeTo(m_Num);
			}
		}
		FORCEINLINE void Reserve(SizeType Number)
		{
			check(Number >= 0);
			if (Number > m_Max)
			{
				_ResizeTo(Number);
			}
		}
		FORCEINLINE void Reset(SizeType NewSize = 0)
		{
			if (NewSize <= m_Max)
			{
				DestructItems(GetData(), m_Num);
				m_Num = 0;
			}
			else
			{
				Empty(NewSize);
			}
		}
		FORCEINLINE void Empty(SizeType InSlack = 0)
		{
			check(InSlack >= 0);

			DestructItems(GetData(), m_Num);
			m_Num = 0;
			if (m_Max != InSlack) _ResizeTo(InSlack);
		}
		FORCEINLINE void SetNum(SizeType NewNum, bool bAllowShrinking = true)
		{
			if (NewNum > Num())
			{
				const SizeType Diff = NewNum - m_Num;
				const SizeType Index = AddUninitialized(Diff);
				DefaultConstructItems<T>(GetData() + Index, Diff);
			}
			else if (NewNum < Num())
			{
				RemoveAt(NewNum, Num() - NewNum, bAllowShrinking);
			}
		}
		void SetNumZeroed(SizeType NewNum, bool bAllowShrinking = true)
		{
			if (NewNum > Num())
			{
				AddZeroed(NewNum - Num());
			}
			else if (NewNum < Num())
			{
				RemoveAt(NewNum, Num() - NewNum, bAllowShrinking);
			}
		}
		void SetNumUninitialized(SizeType NewNum, bool bAllowShrinking = true)
		{
			if (NewNum > Num())
			{
				AddUninitialized(NewNum - Num());
			}
			else if (NewNum < Num())
			{
				RemoveAt(NewNum, Num() - NewNum, bAllowShrinking);
			}
		}

		// find 
		template<typename KeyType>
		T* Find(const KeyType& Item) { return Algo::Find(m_Data, m_Num, Item); }
		template<typename KeyType>
		T* FindLast(const KeyType& Item) { return Algo::FindLast(m_Data, m_Num, Item); }
		template<typename TPred>
		T* FindBy(TPred&& Pred) { return Algo::FindBy(m_Data, m_Num, std::forward<TPred>(Pred)); }
		template<typename TPred>
		T* FindLastBy(TPred&& Pred) { return Algo::FindLastBy(m_Data, m_Num, std::forward<TPred>(Pred)); }
		template<typename KeyType>
		FORCEINLINE const T* Find(const KeyType& Item) const { return const_cast<TArray*>(this)->Find(Item); }
		template<typename KeyType>
		FORCEINLINE const T* FindLast(const KeyType& Item)const { return const_cast<TArray*>(this)->FindLast(Item); }
		template<typename Predicate>
		FORCEINLINE const T* FindBy(Predicate&& Pred) const { return const_cast<TArray*>(this)->FindBy(std::forward<Predicate>(Pred)); }
		template<typename Predicate>
		FORCEINLINE const T* FindLastBy(Predicate&& Pred) const { return const_cast<TArray*>(this)->FindLastBy(std::forward<Predicate>(Pred)); }

		// filter item 
		template<typename Predicate>
		void FilterBy(Predicate&& Pred, TArray<T>& OutArray) const
		{
			const T* RESTRICT Ptr = GetData();
			const T* RESTRICT End = GetData() + m_Num;
			for (; Ptr != End; ++Ptr)
			{
				if (Pred(*Ptr)) OutArray.Emplace(*Ptr);
			}
		}
		template<typename Predicate>
		TArray<T> FilterBy(Predicate&& Pred) const
		{
			TArray<T> OutArray;
			const T* RESTRICT Ptr = GetData();
			const T* RESTRICT End = GetData() + m_Num;
			for (; Ptr != End; ++Ptr)
			{
				if (Pred(*Ptr)) OutArray.Emplace(*Ptr);
			}
			return OutArray;
		}

		// contain
		template <typename KeyType>
		FORCEINLINE bool Contains(const KeyType& Item) const { return Find(Item) != nullptr; }
		template <typename Predicate>
		FORCEINLINE bool ContainsBy(Predicate Pred) const { return FindBy(Pred) != nullptr; }

		// special insert 
		void InsertUninitialized(SizeType Index, SizeType Count = 1)
		{
			CheckInvariants();
			check((Count >= 0) && (Index >= 0) && (Index <= m_Num) && (m_Num + Count > 0));

			const SizeType OldNum = m_Num;
			m_Num += Count;
			_ResizeGrow();

			T* Ptr = GetData() + Index;
			RelocateConstructItems<T>(Ptr + Count, Ptr, OldNum - Index);
		}
		FORCEINLINE void InsertZeroed(SizeType Index, SizeType Count = 1)
		{
			InsertUninitialized(Index, Count);
			Memzero(GetData() + Index, Count * ElementSize);
		}
		FORCEINLINE T& InsertZeroed_GetRef(SizeType Index)
		{
			InsertUninitialized(Index, 1);
			T* Ptr = GetData()+ Index;
			Memzero(Ptr, ElementSize);
			return *Ptr;
		}
		FORCEINLINE void InsertDefaulted(SizeType Index, SizeType Count = 1)
		{
			InsertUninitialized(Index, Count);
			DefaultConstructItems<T>(GetData() + Index, Count);
		}
		FORCEINLINE T& InsertDefaulted_GetRef(SizeType Index)
		{
			InsertUninitialized(Index, 1);
			T* Ptr = GetData() + Index;
			DefaultConstructItems<T>(Ptr, 1);
			return *Ptr;
		}

		// insert 
		FORCEINLINE void Insert(std::initializer_list<T> InitList, const SizeType InIndex)
		{
			InsertUninitialized(InIndex, InitList.size());
			ConstructItems<T>(GetData() + InIndex, InitList.begin(), InitList.size());
		}
		FORCEINLINE void Insert(const TArray& Items, const SizeType InIndex)
		{
			check(this != &Items);
			InsertUninitialized(InIndex, Items.m_Num);
			ConstructItems<T>(GetData() + InIndex, Items.GetData(), Items.m_Num);
		}
		FORCEINLINE void Insert(TArray&& Items, const SizeType InIndex)
		{
			check(this != &Items);
			InsertUninitialized(InIndex, Items.Num());
			MoveConstructItems<T>(GetData() + InIndex, Items.GetData(), Items.m_Num);
			Items.m_Num = 0;
		}
		FORCEINLINE void Insert(const T* Ptr, SizeType Count, SizeType Index)
		{
			check(Ptr != nullptr);
			InsertUninitialized(Index, Count);
			ConstructItems<T>(GetData() + Index, Ptr, Count);
		}
		FORCEINLINE void Insert(T&& Item, SizeType Index)
		{
			CheckAddress(&Item);
			InsertUninitialized(Index, 1);
			new(GetData() + Index) T(std::move(Item));
		}
		FORCEINLINE void Insert(const T& Item, SizeType Index)
		{
			CheckAddress(&Item);
			InsertUninitialized(Index, 1);
			new(GetData() + Index) T(Item);
		}
		FORCEINLINE T& Insert_GetRef(T&& Item, SizeType Index)
		{
			CheckAddress(&Item);

			InsertUninitialized(Index, 1);
			T* Ptr = GetData() + Index;
			new(Ptr) T(std::move(Item));
			return *Ptr;
		}
		FORCEINLINE T& Insert_GetRef(const T& Item, SizeType Index)
		{
			CheckAddress(&Item);

			InsertUninitialized(Index, 1);
			T* Ptr = GetData() + Index;
			new(Ptr) T(Item);
			return *Ptr;
		}

		// add
		FORCEINLINE SizeType AddUninitialized(SizeType Count = 1)
		{
			CheckInvariants();
			check(Count >= 0);

			const SizeType OldNum = m_Num;
			m_Num += Count;

			_ResizeGrow();
			return OldNum;
		}
		FORCEINLINE SizeType Add(T&& Item)
		{
			CheckAddress(&Item);
			return Emplace(std::move(Item));
		}
		FORCEINLINE SizeType Add(const T& Item)
		{
			CheckAddress(&Item);
			return Emplace(Item);
		}
		FORCEINLINE T& Add_GetRef(T&& Item)
		{
			CheckAddress(&Item);
			return Emplace_GetRef(std::move(Item));
		}
		FORCEINLINE T& Add_GetRef(const T& Item)
		{
			CheckAddress(&Item);
			return Emplace_GetRef(Item);
		}
		FORCEINLINE SizeType AddN(SizeType Count = 1)
		{
			const SizeType Index = AddUninitialized(Count);
			DefaultConstructItems<T>(GetData() + Index, Count);
			return Index;
		}
		FORCEINLINE T& Add_GetRef()
		{
			const SizeType Index = AddUninitialized(1);
			T* Ptr = GetData() + Index;
			DefaultConstructItems<T>(Ptr, 1);
			return *Ptr;
		}
		FORCEINLINE SizeType AddZeroed(SizeType Count = 1)
		{
			const SizeType Index = AddUninitialized(Count);
			Memzero(GetData() + Index, Count * ElementSize);
			return Index;
		}
		FORCEINLINE T& AddZeroed_GetRef()
		{
			const SizeType Index = AddUninitialized(1);
			T* Ptr = GetData() + Index;
			Memzero(Ptr, ElementSize);
			return *Ptr;
		}

		// emplace 
		template <typename... ArgsType>
		FORCEINLINE SizeType Emplace(ArgsType&&... Args)
		{
			const SizeType Index = AddUninitialized();
			new(GetData() + Index) T(std::forward<ArgsType>(Args)...);
			return Index;
		}
		template <typename... ArgsType>
		FORCEINLINE T& Emplace_GetRef(ArgsType&&... Args)
		{
			const SizeType Index = AddUninitialized();
			T* Ptr = GetData() + Index;
			new(Ptr) T(std::forward<ArgsType>(Args)...);
			return *Ptr;
		}
		template <typename... ArgsType>
		FORCEINLINE void EmplaceAt(SizeType Index, ArgsType&&... Args)
		{
			InsertUninitialized(Index);
			new(GetData() + Index) T(std::forward<ArgsType>(Args)...);
		}
		template <typename... ArgsType>
		FORCEINLINE T& EmplaceAt_GetRef(SizeType Index, ArgsType&&... Args)
		{
			InsertUninitialized(Index);
			T* Ptr = GetData() + Index;
			new(Ptr) T(std::forward<ArgsType>(Args)...);
			return *Ptr;
		}

		// add unique 
		FORCEINLINE SizeType AddUnique(T&& Item) 
		{
			T* Element = Find(Item);
			if (Element != nullptr) return Element - GetData();
			return Add(std::move(Item));
		}
		FORCEINLINE SizeType AddUnique(const T& Item) 
		{
			T* Element = Find(Item);
			if (Element != nullptr) return Element - GetData();
			return Add(Item);
		}

		// remove at
		void RemoveAt(SizeType Index, SizeType Count = 1, bool bAllowShrinking = true)
		{
			if (Count)
			{
				check((Count >= 0) & (Index >= 0) & (Index + Count <= m_Num));
				CheckInvariants();

				T* HoleBegin = GetData() + Index;
				T* HoleEnd = HoleBegin + Count;

				// destruct 
				DestructItems(HoleBegin, Count);
				
				// move memory 
				RelocateConstructItems(HoleBegin, HoleEnd, Count);
				m_Num -= Count;

				if (bAllowShrinking) _ResizeShrink();
			}
		}
		void RemoveAtSwap(SizeType Index, SizeType Count = 1, bool bAllowShrinking = true)
		{
			if (Count)
			{
				CheckInvariants();
				check((Count >= 0) & (Index >= 0) & (Index + Count <= m_Num));

				// destruct 
				DestructItems(GetData() + Index, Count);

				// move memory 
				const SizeType NumElementsInHole = Count;
				const SizeType NumElementsAfterHole = m_Num - (Index + Count);
				const SizeType NumElementsToMoveIntoHole = FMath::Min(NumElementsInHole, NumElementsAfterHole);
				if (NumElementsToMoveIntoHole)
				{
					Memcpy(
						GetData() + Index,
						GetData() + (m_Num - NumElementsToMoveIntoHole),
						NumElementsToMoveIntoHole * sizeof(T)
					);
				}
				m_Num -= Count;

				if (bAllowShrinking) _ResizeShrink();
			}
		}
		
		// remove
		SizeType Remove(const T& Item)
		{
			CheckAddress(&Item);
			return RemoveBy([&Item](T& Element) { return Element == Item; });
		}
		SizeType RemoveSwap(const T& Item)
		{
			CheckAddress(&Item);

			const SizeType OriginalNum = m_Num;
			for (SizeType Index = 0; Index < m_Num; Index++)
			{
				if ((*this)[Index] == Item)
				{
					RemoveAtSwap(Index--);
				}
			}
			return OriginalNum - m_Num;
		}
		template <class Predicate>
		SizeType RemoveBy(Predicate&& Pred)
		{
			const SizeType OriginalNum = m_Num;
			if (!OriginalNum) return 0;

			SizeType WriteIndex = 0;
			SizeType ReadIndex = 0;
			bool NotMatch = !Pred(GetData()[ReadIndex]);
			do
			{
				SizeType RunStartIndex = ReadIndex++;
				while (ReadIndex < OriginalNum && NotMatch == !Pred(GetData()[ReadIndex]))
				{
					ReadIndex++;
				}
				SizeType RunLength = ReadIndex - RunStartIndex;
				check(RunLength > 0);
				if (NotMatch)
				{
					// this was a non-matching run, we need to move it
					if (WriteIndex != RunStartIndex)
					{
						Memmove(&GetData()[WriteIndex], &GetData()[RunStartIndex], sizeof(T)* RunLength);
					}
					WriteIndex += RunLength;
				}
				else
				{
					// this was a matching run, delete it
					DestructItems(GetData() + RunStartIndex, RunLength);
				}
				NotMatch = !NotMatch;
			} while (ReadIndex < OriginalNum);

			m_Num = WriteIndex;
			return OriginalNum - m_Num;
		}
		template <class Predicate>
		SizeType RemoveBySwap(const Predicate& Pred, bool bAllowShrinking = true)
		{
			const SizeType OriginalNum = m_Num;
			for (SizeType ItemIndex = 0; ItemIndex < Num();)
			{
				if (Pred((*this)[ItemIndex]))
				{
					RemoveAtSwap(ItemIndex, 1, bAllowShrinking);
				}
				else
				{
					++ItemIndex;
				}
			}
			return OriginalNum - m_Num;
		}
		SizeType RemoveSingle(const T& Item)
		{
			T* Element = Find(Item);
			if (Element == nullptr) return 0;

			auto* RemovePtr = GetData() + Index;

			DestructItems(RemovePtr, 1);
			SizeType Index = Element - GetData();
			const SizeType NextIndex = Index + 1;
			RelocateConstructItems<T>(RemovePtr, RemovePtr + 1, m_Num - (Index + 1));

			--m_Num;
			return 1;
		}
		SizeType RemoveSingleSwap(const T& Item, bool bAllowShrinking = true)
		{
			T* Element = Find(Item);
			if (Element == nullptr) return 0;

			RemoveAtSwap(Element - GetData(), 1, bAllowShrinking);

			return 1;
		}

		// append 
		template <typename TOther>
		FORCEINLINE TArray& operator+=(const TArray<TOther>& Source)
		{
			check((void*)this != (void*)&Source);

			if (!Source.Num()) return this;

			SizeType Pos = AddUninitialized(Source.Num());
			ConstructItems<T>(GetData() + Pos, Source.GetData(), Source.Num());
			return *this;
		}
		template <typename TOther>
		FORCEINLINE TArray& operator+=(TArray<TOther>&& Source)
		{
			check((void*)this != (void*)&Source);

			if (!Source.Num()) return *this;

			SizeType Pos = AddUninitialized(Source.Num());
			RelocateConstructItems(GetData() + Pos, Source.GetData(), Source.Num());
			Source.Empty();
			return *this;
		}
		FORCEINLINE TArray& operator+=(std::initializer_list<T> InitList)
		{
			SizeType Count = (SizeType)InitList.size();

			SizeType Pos = AddUninitialized(Count);
			ConstructItems<T>(GetData() + Pos, InitList.begin(), Count);
			return *this;
		}

		// Init 
		void Init(const T& Element, SizeType Number)
		{
			Empty(Number);
			for (SizeType Index = 0; Index < Number; ++Index) Emplace(Element);
		}

		// swap 
		FORCEINLINE void Swap(SizeType FirstIndexToSwap, SizeType SecondIndexToSwap)
		{
			check((FirstIndexToSwap >= 0) && (SecondIndexToSwap >= 0));
			check((m_Num > FirstIndexToSwap) && (m_Num > SecondIndexToSwap));
			if (FirstIndexToSwap != SecondIndexToSwap)
			{
				Memswap(GetData() + FirstIndexToSwap, GetData() + SecondIndexToSwap, sizeof(T));
			}
		}

		// sort 
		template<class TPred = TLess<T>>
		void Sort(TPred&& Pred = TPred()) { Algo::IntroSort(GetData(), Num(), std::forward<TPred>(Pred)); }
		template<class TPred = TLess<T>>
		void StableSort(TPred&& Pred = TPred()) { Algo::StableSort(GetData(), Num(), std::forward<TPred>(Pred)); }

		// support heap 
		T& HeapTop() { return *m_Data; }
		template<class TPred = TLess<T>>
		FORCEINLINE void Heapify(TPred&& Pred = TPred())
		{
			Algo::Heapify(GetData(), Num(), std::forward<TPred>(Pred));
		}
		template<class TPred = TLess<T>>
		SizeType HeapPush(T&& InItem, TPred&& Pred = TPred())
		{
			Add(std::move(InItem));
			return Algo::HeapSiftUp(GetData(), (SizeType)0, Num() - 1, std::forward<TPred>(Pred));
		}
		template<class TPred = TLess<T>>
		SizeType HeapPush(const T& InItem, TPred&& Pred = TPred())
		{
			Add(InItem);
			return Algo::HeapSiftUp(GetData(), 0, Num() - 1, std::forward<TPred>(Pred));
		}
		template<class TPred = TLess<T>>
		void HeapPop(T& OutItem, TPred&& Pred = TPred(), bool bAllowShrinking = true)
		{
			OutItem = std::move((*this)[0]);
			RemoveAtSwap(0, 1, bAllowShrinking);

			Algo::HeapSiftDown(GetData(), (SizeType)0, Num(), std::forward<TPred>(Pred));
		}
		template<class TPred = TLess<T>>
		bool IsHeap(TPred&& Pred = TPred())
		{
			return Algo::IsHeap(GetData(), Num(), std::forward<TPred>(Pred));
		}
		template<class TPred = TLess<T>>
		void HeapPopDiscard(TPred&& Pred = TPred(), bool bAllowShrinking = true)
		{
			RemoveAtSwap(0, 1, bAllowShrinking);
			Algo::HeapSiftDown(GetData(), 0, Num(), std::forward<TPred>(Pred));
		}
		template<class TPred = TLess<T>>
		void HeapRemoveAt(SizeType Index, TPred&& Pred = TPred(), bool bAllowShrinking = true)
		{
			RemoveAtSwap(Index, 1, bAllowShrinking);

			Algo::HeapSiftDown(GetData(), Index, Num(), std::forward<TPred>(Pred));
			Algo::HeapSiftUp(GetData(), 0, FMath::Min(Index, Num() - 1), std::forward<TPred>(Pred));
		}
		template<class TPred = TLess<T>>
		void HeapSort(TPred&& Pred = TPred()) { Algo::HeapSort(GetData(), Num(), std::forward<TPred>(Pred)); }

		FORCEINLINE T*      	begin()			{ return GetData(); }
		FORCEINLINE const T* 	begin() const	{ return GetData(); }
		FORCEINLINE T*      	end()			{ return GetData() + Num(); }
		FORCEINLINE const T* 	end() const		{ return GetData() + Num(); }
	};
}

// TypeTraits
template <typename T>
struct TIsContiguousContainer<Fuko::TArray<T>>
{
	enum { value = true };
};
