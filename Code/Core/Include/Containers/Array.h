#pragma once
#include "CoreConfig.h"
#include "Templates/TypeTraits.h"
#include "Math/MathUtility.h"
#include "Templates/UtilityTemp.h"
#include "Algo/Sort.h"
#include "CoreMinimal/Assert.h"
#include "Templates/Sorting.h"
#include "Allocator.h"
#include <Algo/IsHeap.h>
// forward
namespace Fuko
{
	template<typename T, template<typename> typename Alloc = TPmrAllocator>
	class TArray;
}

// 迭代器
namespace Fuko
{
	// 通用迭代器
	template< typename ContainerType, typename ElementType, typename SizeType>
	class TIndexedContainerIterator
	{
	public:
		TIndexedContainerIterator(ContainerType& InContainer, SizeType StartIndex = 0)
			: Container(InContainer)
			, Index(StartIndex)
		{
		}

		/** Advances iterator to the next element in the container. */
		TIndexedContainerIterator& operator++()
		{
			++Index;
			return *this;
		}
		TIndexedContainerIterator operator++(int)
		{
			TIndexedContainerIterator Tmp(*this);
			++Index;
			return Tmp;
		}

		TIndexedContainerIterator& operator--()
		{
			--Index;
			return *this;
		}
		TIndexedContainerIterator operator--(int)
		{
			TIndexedContainerIterator Tmp(*this);
			--Index;
			return Tmp;
		}

		TIndexedContainerIterator& operator+=(SizeType Offset)
		{
			Index += Offset;
			return *this;
		}

		TIndexedContainerIterator operator+(SizeType Offset) const
		{
			TIndexedContainerIterator Tmp(*this);
			return Tmp += Offset;
		}

		TIndexedContainerIterator& operator-=(SizeType Offset)
		{
			return *this += -Offset;
		}

		TIndexedContainerIterator operator-(SizeType Offset) const
		{
			TIndexedContainerIterator Tmp(*this);
			return Tmp -= Offset;
		}

		ElementType& operator* () const
		{
			return Container[Index];
		}

		ElementType* operator->() const
		{
			return &Container[Index];
		}

		FORCEINLINE explicit operator bool() const
		{
			return Container.IsValidIndex(Index);
		}

		SizeType GetIndex() const
		{
			return Index;
		}

		void Reset()
		{
			Index = 0;
		}

		void SetToEnd()
		{
			Index = Container.Num();
		}

		void RemoveCurrent()
		{
			Container.RemoveAt(Index);
			Index--;
		}

		FORCEINLINE friend bool operator==(const TIndexedContainerIterator& Lhs, const TIndexedContainerIterator& Rhs) { return &Lhs.Container == &Rhs.Container && Lhs.Index == Rhs.Index; }
		FORCEINLINE friend bool operator!=(const TIndexedContainerIterator& Lhs, const TIndexedContainerIterator& Rhs) { return &Lhs.Container != &Rhs.Container || Lhs.Index != Rhs.Index; }

	private:

		ContainerType& Container;
		SizeType      Index;
	};
	// 重载+  
	template <typename ContainerType, typename ElementType, typename SizeType>
	FORCEINLINE TIndexedContainerIterator<ContainerType, ElementType, SizeType> operator+(SizeType Offset, TIndexedContainerIterator<ContainerType, ElementType, SizeType> RHS)
	{
		return RHS + Offset;
	}

	// 检测迭代期间是否发生改变
#if FUKO_DEBUG
	template <typename ElementType, typename SizeType>
	struct TCheckedPointerIterator
	{
		explicit TCheckedPointerIterator(const SizeType& InNum, ElementType* InPtr)
			: Ptr(InPtr)
			, CurrentNum(InNum)
			, InitialNum(InNum)
		{
		}

		FORCEINLINE ElementType& operator*() const
		{
			return *Ptr;
		}

		FORCEINLINE TCheckedPointerIterator& operator++()
		{
			++Ptr;
			return *this;
		}

		FORCEINLINE TCheckedPointerIterator& operator--()
		{
			--Ptr;
			return *this;
		}

	private:
		ElementType*    Ptr;
		const SizeType& CurrentNum;
		SizeType        InitialNum;

		FORCEINLINE friend bool operator!=(const TCheckedPointerIterator& Lhs, const TCheckedPointerIterator& Rhs)
		{
			ensuref(Lhs.CurrentNum == Lhs.InitialNum, TEXT("Array has changed during ranged-for iteration!"));
			return Lhs.Ptr != Rhs.Ptr;
		}
	};
#endif
	// 自动解引用的迭代器
	template <typename ElementType, typename IteratorType>
	struct TDereferencingIterator
	{
		explicit TDereferencingIterator(IteratorType InIter)
			: Iter(InIter)
		{
		}

		FORCEINLINE ElementType& operator*() const
		{
			return *(ElementType*)*Iter;
		}

		FORCEINLINE TDereferencingIterator& operator++()
		{
			++Iter;
			return *this;
		}

	private:
		IteratorType Iter;

		FORCEINLINE friend bool operator!=(const TDereferencingIterator& Lhs, const TDereferencingIterator& Rhs)
		{
			return Lhs.Iter != Rhs.Iter;
		}
	};
}

// Array
namespace Fuko
{
	template<typename T, template<typename> typename Alloc>
	class TArray
	{
	public:
		using ElementType = T;
		using AllocType = Alloc<T>;
		using SizeType = typename AllocType::SizeType;
	protected:
		AllocType	m_Allocator;
		T*			m_Data;
		SizeType	m_Num;
		SizeType	m_Max;

		static constexpr SizeType ElementSize = (SizeType)sizeof(ElementType);
	private:
		FORCENOINLINE void FreeArray()
		{
			DestructItems(GetData(), m_Num);
			m_Num = 0;
			m_Max = m_Allocator.Free(m_Data);
		}
		FORCENOINLINE void ResizeGrow()
		{
			if (m_Num > m_Max)
			{
				SizeType NewMax = m_Allocator.GetGrow(m_Num, m_Max);
				m_Max = m_Allocator.Reserve(m_Data, NewMax);
			}
		}
		FORCENOINLINE void ResizeShrink()
		{
			auto NewMax = m_Allocator.GetShrink(m_Num, m_Max);
			if (NewMax < m_Max) m_Max = m_Allocator.Reserve(m_Data, NewMax);
		}
		FORCENOINLINE void ResizeTo(SizeType NewMax)
		{
			if (m_Max != NewMax) m_Max = m_Allocator.Reserve(m_Data, NewMax);
		}
		template <typename OtherElementType>
		void CopyToEmpty(const OtherElementType* OtherData, SizeType OtherNum, SizeType ExtraSlack)
		{
			check(ExtraSlack >= 0);

			m_Num = OtherNum;

			if (OtherNum || ExtraSlack)
			{
				SizeType NewMax = OtherNum + ExtraSlack;
				
				if (NewMax > m_Max) ResizeTo(NewMax);
				ConstructItems<ElementType>(GetData(), OtherData, OtherNum);
			}
		}
	public:
		// construct 
		FORCEINLINE TArray(AllocType&& InAlloc = AllocType())
			: m_Num(0)
			, m_Data(nullptr)
			, m_Max(0)
			, m_Allocator(std::move(InAlloc))
		{}
		FORCEINLINE TArray(const ElementType* Ptr, SizeType Count, AllocType&& InAlloc = AllocType())
			: m_Num(0)
			, m_Data(nullptr)
			, m_Max(0)
			, m_Allocator(std::move(InAlloc))
		{
			check(Ptr != nullptr || Count == 0);
			CopyToEmpty(Ptr, Count, 0);
		}
		FORCEINLINE TArray(std::initializer_list<ElementType> InitList, AllocType&& InAlloc = AllocType())
			: m_Num(0)
			, m_Data(nullptr)
			, m_Max(0)
			, m_Allocator(std::move(InAlloc))
		{
			CopyToEmpty(InitList.begin(), (SizeType)InitList.size(), 0);
		}

		// copy construct 
		template <typename OtherElementType>
		FORCEINLINE explicit TArray(const TArray<OtherElementType>& Other, AllocType&& InAlloc = AllocType())
			: m_Num(0)
			, m_Data(nullptr)
			, m_Max(0)
			, m_Allocator(std::move(InAlloc))
		{
			CopyToEmpty(Other.GetData(), Other.Num(), 0);
		}
		FORCEINLINE TArray(const TArray& Other, AllocType&& InAlloc = AllocType())
			: m_Num(0)
			, m_Data(nullptr)
			, m_Max(0)
			, m_Allocator(std::move(InAlloc))
		{
			CopyToEmpty(Other.GetData(), Other.Num(), 0);
		}
		FORCEINLINE TArray(const TArray& Other, SizeType ExtraSlack, AllocType&& InAlloc = AllocType())
			: m_Num(0)
			, m_Data(nullptr)
			, m_Max(0)
			, m_Allocator(std::move(InAlloc))
		{
			CopyToEmpty(Other.GetData(), Other.Num(), ExtraSlack);
		}

		// assign operator 
		TArray& operator=(std::initializer_list<ElementType> InitList)
		{
			DestructItems(GetData(), m_Num);
			CopyToEmpty(InitList.begin(), (SizeType)InitList.size(), 0);
			return *this;
		}
		TArray& operator=(const TArray& Other)
		{
			if (this != &Other)
			{
				DestructItems(GetData(), m_Num);
				CopyToEmpty(Other.GetData(), Other.Num(), 0);
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
		TArray& operator=(TArray&& Other)
		{
			if (this == &Other) return *this;
			
			FreeArray();

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
		~TArray()
		{
			FreeArray();
		}
		
	public:
		// get infomation 
		FORCEINLINE ElementType* GetData() { return m_Data; }
		FORCEINLINE const ElementType* GetData() const { return m_Data; }
		FORCEINLINE uint32 GetTypeSize() const { return ElementSize; }
		FORCEINLINE SizeType Num() const { return m_Num; }
		FORCEINLINE SizeType Max() const { return m_Max; }
		FORCEINLINE SizeType Slack() const { return m_Max - m_Num; }
		FORCEINLINE const AllocType& GetAllocator() const { return m_Allocator; }
		FORCEINLINE AllocType& GetAllocator() { return m_Allocator; }

		// compare
		FORCEINLINE bool operator==(const TArray& Rhs) const { return m_Num == Rhs.m_Num && CompareItems(GetData(), Rhs.GetData(), m_Num); }
		FORCEINLINE bool operator!=(const TArray& OtherArray) const { return !(*this == OtherArray); }

		// debug check
		FORCEINLINE void CheckInvariants() const { check((m_Num >= 0) & (m_Max >= m_Num)); }
		FORCEINLINE void CheckAddress(const ElementType* Addr) const
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
		FORCEINLINE ElementType& operator[](SizeType Index)
		{
			RangeCheck(Index);
			return GetData()[Index];
		}
		FORCEINLINE const ElementType& operator[](SizeType Index) const
		{
			RangeCheck(Index);
			return GetData()[Index];
		}

		// get element 
		FORCEINLINE ElementType& Top() { return Last(); }
		FORCEINLINE const ElementType& Top() const { return Last(); }
		FORCEINLINE ElementType& Last(SizeType IndexFromTheEnd = 0)
		{
			RangeCheck(m_Num - IndexFromTheEnd - 1);
			return GetData()[m_Num - IndexFromTheEnd - 1];
		}
		FORCEINLINE const ElementType& Last(SizeType IndexFromTheEnd = 0) const
		{
			RangeCheck(m_Num - IndexFromTheEnd - 1);
			return GetData()[m_Num - IndexFromTheEnd - 1];
		}

		// pop & push 
		FORCEINLINE ElementType Pop(bool bAllowShrinking = true)
		{
			RangeCheck(0);
			ElementType Result = std::move(GetData()[m_Num - 1]);
			RemoveAt(m_Num - 1, 1, bAllowShrinking);
			return Result;
		}
		FORCEINLINE void Push(ElementType&& Item) { Emplace(std::move(Item)); }
		FORCEINLINE void Push(const ElementType& Item) { Emplace(Item); }

		// shrink & reserve 
		FORCEINLINE void Shrink()
		{
			CheckInvariants();
			if (m_Max != m_Num)
			{
				ResizeTo(m_Num);
			}
		}
		FORCEINLINE void Reserve(SizeType Number)
		{
			check(Number >= 0);
			if (Number > m_Max)
			{
				ResizeTo(Number);
			}
		}

		// set num & empty 
		void Reset(SizeType NewSize = 0)
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
		void Empty(SizeType InSlack = 0)
		{
			check(InSlack >= 0);

			DestructItems(GetData(), m_Num);
			m_Num = 0;
			if (m_Max != InSlack) ResizeTo(InSlack);
		}
		void SetNum(SizeType NewNum, bool bAllowShrinking = true)
		{
			if (NewNum > Num())
			{
				const SizeType Diff = NewNum - m_Num;
				const SizeType Index = AddUninitialized(Diff);
				DefaultConstructItems<ElementType>(GetData() + Index, Diff);
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
		void SetNumUnsafe(SizeType NewNum)
		{
			check(NewNum <= Num() && NewNum >= 0);
			m_Num = NewNum;
		}

		// find 
		template<typename KeyType>
		ElementType* Find(const KeyType& Item)
		{
			const ElementType* RESTRICT Ptr = GetData();
			const ElementType* RESTRICT End = GetData() + m_Num;
			for (; Ptr != End; ++Ptr)
			{
				if (*Ptr == Item) return (ElementType*)Ptr;
			}
			return nullptr;
		}
		template<typename KeyType>
		ElementType* FindLast(const KeyType& Item)
		{
			const ElementType* RESTRICT Ptr = GetData() + m_Num - 1;
			for (; Ptr >= GetData(); --Ptr)
			{
				if (*Ptr == Item) return (ElementType*)Ptr;
			}
			return nullptr;
		}
		template<typename Predicate>
		ElementType* FindBy(Predicate&& Pred)
		{
			const ElementType* RESTRICT Ptr = GetData();
			const ElementType* RESTRICT End = GetData() + m_Num;
			for (; Ptr != End; ++Ptr)
			{
				if (Pred(*Ptr)) return (ElementType*)Ptr;
			}
			return nullptr;
		}
		template<typename Predicate>
		ElementType* FindLastBy(Predicate&& Pred)
		{
			const ElementType* RESTRICT Ptr = GetData() + m_Num - 1;
			for (; Ptr >= GetData(); --Ptr)
			{
				if (Pred(*Ptr)) return (ElementType*)Ptr;
			}
			return nullptr;
		}
		template<typename KeyType>
		FORCEINLINE const ElementType* Find(const KeyType& Item) const
		{
			return const_cast<TArray*>(this)->Find(Item);
		}
		template<typename KeyType>
		FORCEINLINE const ElementType* FindLast(const KeyType& Item)const
		{
			return const_cast<TArray*>(this)->FindLast(Item);
		}
		template<typename Predicate>
		FORCEINLINE const ElementType* FindBy(Predicate&& Pred) const
		{
			return const_cast<TArray*>(this)->FindBy(std::forward<Predicate>(Pred));
		}
		template<typename Predicate>
		FORCEINLINE  ElementType* FindLastBy(Predicate&& Pred) const
		{
			return const_cast<TArray*>(this)->FindLastBy(std::forward<Predicate>(Pred));
		}

		// indexof 
		template<typename KeyType>
		FORCEINLINE SizeType IndexOf(const KeyType& Item) const
		{
			const ElementType* FindPtr = this->Find(Item);
			return FindPtr == nullptr ? INDEX_NONE : FindPtr - GetData();
		}
		template<typename KeyType>
		FORCEINLINE SizeType IndexOfLast(const KeyType& Item) const
		{
			const ElementType* FindPtr = this->FindLast(Item);
			return FindPtr == nullptr ? INDEX_NONE : FindPtr - GetData();
		}
		template<typename Predicate>
		FORCEINLINE SizeType IndexOfBy(Predicate Pred) const
		{
			const ElementType* FindPtr = this->FindBy(Pred);
			return FindPtr == nullptr ? INDEX_NONE : FindPtr - GetData();
		}
		template<typename Predicate>
		FORCEINLINE SizeType IndexOfLastBy(Predicate Pred) const
		{
			const ElementType* FindPtr = this->FindLastBy(Pred);
			return FindPtr == nullptr ? INDEX_NONE : FindPtr - GetData();
		}
		
		// filter item 
		template<typename Predicate>
		void FilterBy(Predicate&& Pred, TArray<ElementType>& OutArray) const
		{
			const ElementType* RESTRICT Ptr = GetData();
			const ElementType* RESTRICT End = GetData() + m_Num;
			for (; Ptr != End; ++Ptr)
			{
				if (Pred(*Ptr)) OutArray.Emplace(*Ptr);
			}
		}
		template<typename Predicate>
		TArray<ElementType> FilterBy(Predicate&& Pred) const
		{
			TArray<ElementType> OutArray;
			const ElementType* RESTRICT Ptr = GetData();
			const ElementType* RESTRICT End = GetData() + m_Num;
			for (; Ptr != End; ++Ptr)
			{
				if (Pred(*Ptr)) OutArray.Emplace(*Ptr);
			}
			return OutArray;
		}

		// contain
		template <typename KeyType>
		FORCEINLINE bool Contains(const KeyType& Item) const
		{
			return Find(Item) != nullptr;
		}
		template <typename Predicate>
		FORCEINLINE bool ContainsBy(Predicate Pred) const
		{
			return FindBy(Pred) != nullptr;
		}

		// special insert 
		void InsertUninitialized(SizeType Index, SizeType Count = 1)
		{
			CheckInvariants();
			check((Count >= 0) && (Index >= 0) && (Index <= m_Num) && (m_Num + Count > 0));

			const SizeType OldNum = m_Num;
			m_Num += Count;
			ResizeGrow();

			ElementType* Ptr = GetData() + Index;
			RelocateConstructItems<ElementType>(Ptr + Count, Ptr, OldNum - Index);
		}
		FORCEINLINE void InsertZeroed(SizeType Index, SizeType Count = 1)
		{
			InsertUninitialized(Index, Count);
			Memzero(GetData() + Index, Count * ElementSize);
		}
		FORCEINLINE ElementType& InsertZeroed_GetRef(SizeType Index)
		{
			InsertUninitialized(Index, 1);
			ElementType* Ptr = GetData()+ Index;
			Memzero(Ptr, ElementSize);
			return *Ptr;
		}
		FORCEINLINE void InsertDefaulted(SizeType Index, SizeType Count = 1)
		{
			InsertUninitialized(Index, Count);
			DefaultConstructItems<ElementType>(GetData() + Index, Count);
		}
		FORCEINLINE ElementType& InsertDefaulted_GetRef(SizeType Index)
		{
			InsertUninitialized(Index, 1);
			ElementType* Ptr = GetData() + Index;
			DefaultConstructItems<ElementType>(Ptr, 1);
			return *Ptr;
		}

		// insert 
		FORCEINLINE void Insert(std::initializer_list<ElementType> InitList, const SizeType InIndex)
		{
			InsertUninitialized(InIndex, InitList.size());
			ConstructItems<ElementType>(GetData() + InIndex, InitList.begin(), InitList.size());
		}
		FORCEINLINE void Insert(const TArray& Items, const SizeType InIndex)
		{
			check(this != &Items);
			InsertUninitialized(InIndex, Items.m_Num);
			ConstructItems<ElementType>(GetData() + InIndex, Items.GetData(), Items.m_Num);
		}
		FORCEINLINE void Insert(TArray&& Items, const SizeType InIndex)
		{
			check(this != &Items);
			InsertUninitialized(InIndex, Items.Num());
			MoveConstructItems<ElementType>(GetData() + InIndex, Items.GetData(), Items.m_Num);
			Items.m_Num = 0;
		}
		FORCEINLINE void Insert(const ElementType* Ptr, SizeType Count, SizeType Index)
		{
			check(Ptr != nullptr);
			InsertUninitialized(Index, Count);
			ConstructItems<ElementType>(GetData() + Index, Ptr, Count);
		}
		FORCEINLINE void Insert(ElementType&& Item, SizeType Index)
		{
			CheckAddress(&Item);
			InsertUninitialized(Index, 1);
			new(GetData() + Index) ElementType(std::move(Item));
		}
		FORCEINLINE void Insert(const ElementType& Item, SizeType Index)
		{
			CheckAddress(&Item);
			InsertUninitialized(Index, 1);
			new(GetData() + Index) ElementType(Item);
		}
		FORCEINLINE ElementType& Insert_GetRef(ElementType&& Item, SizeType Index)
		{
			CheckAddress(&Item);

			InsertUninitialized(Index, 1);
			ElementType* Ptr = GetData() + Index;
			new(Ptr) ElementType(std::move(Item));
			return *Ptr;
		}
		FORCEINLINE ElementType& Insert_GetRef(const ElementType& Item, SizeType Index)
		{
			CheckAddress(&Item);

			InsertUninitialized(Index, 1);
			ElementType* Ptr = GetData() + Index;
			new(Ptr) ElementType(Item);
			return *Ptr;
		}

		// add
		FORCEINLINE SizeType AddUninitialized(SizeType Count = 1)
		{
			CheckInvariants();
			check(Count >= 0);

			const SizeType OldNum = m_Num;
			m_Num += Count;

			ResizeGrow();
			return OldNum;
		}
		FORCEINLINE SizeType Add(ElementType&& Item)
		{
			CheckAddress(&Item);
			return Emplace(std::move(Item));
		}
		FORCEINLINE SizeType Add(const ElementType& Item)
		{
			CheckAddress(&Item);
			return Emplace(Item);
		}
		FORCEINLINE ElementType& Add_GetRef(ElementType&& Item)
		{
			CheckAddress(&Item);
			return Emplace_GetRef(std::move(Item));
		}
		FORCEINLINE ElementType& Add_GetRef(const ElementType& Item)
		{
			CheckAddress(&Item);
			return Emplace_GetRef(Item);
		}
		FORCEINLINE SizeType AddN(SizeType Count = 1)
		{
			const SizeType Index = AddUninitialized(Count);
			DefaultConstructItems<ElementType>(GetData() + Index, Count);
			return Index;
		}
		FORCEINLINE ElementType& Add_GetRef()
		{
			const SizeType Index = AddUninitialized(1);
			ElementType* Ptr = GetData() + Index;
			DefaultConstructItems<ElementType>(Ptr, 1);
			return *Ptr;
		}
		FORCEINLINE SizeType AddZeroed(SizeType Count = 1)
		{
			const SizeType Index = AddUninitialized(Count);
			Memzero(GetData() + Index, Count * ElementSize);
			return Index;
		}
		FORCEINLINE ElementType& AddZeroed_GetRef()
		{
			const SizeType Index = AddUninitialized(1);
			ElementType* Ptr = GetData() + Index;
			Memzero(Ptr, ElementSize);
			return *Ptr;
		}

		// emplace 
		template <typename... ArgsType>
		FORCEINLINE SizeType Emplace(ArgsType&&... Args)
		{
			const SizeType Index = AddUninitialized();
			new(GetData() + Index) ElementType(std::forward<ArgsType>(Args)...);
			return Index;
		}
		template <typename... ArgsType>
		FORCEINLINE ElementType& Emplace_GetRef(ArgsType&&... Args)
		{
			const SizeType Index = AddUninitialized();
			ElementType* Ptr = GetData() + Index;
			new(Ptr) ElementType(std::forward<ArgsType>(Args)...);
			return *Ptr;
		}
		template <typename... ArgsType>
		FORCEINLINE void EmplaceAt(SizeType Index, ArgsType&&... Args)
		{
			InsertUninitialized(Index);
			new(GetData() + Index) ElementType(std::forward<ArgsType>(Args)...);
		}
		template <typename... ArgsType>
		FORCEINLINE ElementType& EmplaceAt_GetRef(SizeType Index, ArgsType&&... Args)
		{
			InsertUninitialized(Index);
			ElementType* Ptr = GetData() + Index;
			new(Ptr) ElementType(std::forward<ArgsType>(Args)...);
			return *Ptr;
		}

		// add unique 
		FORCEINLINE SizeType AddUnique(ElementType&& Item) 
		{
			SizeType Index = IndexOf(Item);
			if (Index != INDEX_NONE) return Index;
			return Add(std::move(Item));
		}
		FORCEINLINE SizeType AddUnique(const ElementType& Item) 
		{
			SizeType Index = IndexOf(Item);
			if (Index != INDEX_NONE) return Index;
			return Add(Item);
		}

		// remove at
		void RemoveAt(SizeType Index, SizeType Count = 1, bool bAllowShrinking = true)
		{
			if (Count)
			{
				check((Count >= 0) & (Index >= 0) & (Index + Count <= m_Num));
				CheckInvariants();

				ElementType* HoleBegin = GetData() + Index;
				ElementType* HoleEnd = HoleBegin + Count;

				// destruct 
				DestructItems(HoleBegin, Count);
				
				// move memory 
				RelocateConstructItems(HoleBegin, HoleEnd, Count);
				m_Num -= Count;

				if (bAllowShrinking) ResizeShrink();
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
						NumElementsToMoveIntoHole * sizeof(ElementType)
					);
				}
				m_Num -= Count;

				if (bAllowShrinking) ResizeShrink();
			}
		}
		
		// remove
		SizeType Remove(const ElementType& Item)
		{
			CheckAddress(&Item);
			return RemoveBy([&Item](ElementType& Element) { return Element == Item; });
		}
		SizeType RemoveSwap(const ElementType& Item)
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
						Memmove(&GetData()[WriteIndex], &GetData()[RunStartIndex], sizeof(ElementType)* RunLength);
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
		SizeType RemoveSingle(const ElementType& Item)
		{
			SizeType Index = IndexOf(Item);
			if (Index == INDEX_NONE) return 0;

			auto* RemovePtr = GetData() + Index;

			DestructItems(RemovePtr, 1);
			const SizeType NextIndex = Index + 1;
			RelocateConstructItems<ElementType>(RemovePtr, RemovePtr + 1, m_Num - (Index + 1));

			--m_Num;
			return 1;
		}
		SizeType RemoveSingleSwap(const ElementType& Item, bool bAllowShrinking = true)
		{
			SizeType Index = IndexOf(Item);
			if (Index == INDEX_NONE) return 0;

			RemoveAtSwap(Index, 1, bAllowShrinking);

			return 1;
		}

		// append 
		template <typename OtherElementType>
		FORCEINLINE void Append(const TArray<OtherElementType>& Source)
		{
			check((void*)this != (void*)&Source);

			if (!Source.Num()) return;

			SizeType Pos = AddUninitialized(Source.Num());
			ConstructItems<ElementType>(GetData() + Pos, Source.GetData(), Source.Num());
		}
		template <typename OtherElementType>
		FORCEINLINE void Append(TArray<OtherElementType>&& Source)
		{
			check((void*)this != (void*)&Source);

			if (!Source.Num()) return;

			SizeType Pos = AddUninitialized(Source.Num());
			RelocateConstructItems(GetData() + Pos, Source.GetData(), Source.Num());
			Source.Empty();
		}
		FORCEINLINE void Append(const ElementType* Ptr, SizeType Count)
		{
			check(Ptr != nullptr || Count == 0);

			SizeType Pos = AddUninitialized(Count);
			ConstructItems<ElementType>(GetData() + Pos, Ptr, Count);
		}
		FORCEINLINE void Append(std::initializer_list<ElementType> InitList)
		{
			SizeType Count = (SizeType)InitList.size();

			SizeType Pos = AddUninitialized(Count);
			ConstructItems<ElementType>(GetData() + Pos, InitList.begin(), Count);
		}
		FORCEINLINE TArray& operator+=(TArray&& Other)
		{
			Append(std::move(Other));
			return *this;
		}
		FORCEINLINE TArray& operator+=(const TArray& Other)
		{
			Append(Other);
			return *this;
		}
		FORCEINLINE TArray& operator+=(std::initializer_list<ElementType> InitList)
		{
			Append(InitList);
			return *this;
		}

		// Init 
		void Init(const ElementType& Element, SizeType Number)
		{
			Empty(Number);
			for (SizeType Index = 0; Index < Number; ++Index)
			{
				new(*this) ElementType(Element);
			}
		}

		// swap 
		FORCEINLINE void Swap(SizeType FirstIndexToSwap, SizeType SecondIndexToSwap)
		{
			check((FirstIndexToSwap >= 0) && (SecondIndexToSwap >= 0));
			check((m_Num > FirstIndexToSwap) && (m_Num > SecondIndexToSwap));
			if (FirstIndexToSwap != SecondIndexToSwap)
			{
				Memswap(GetData() + FirstIndexToSwap, GetData() + SecondIndexToSwap, sizeof(ElementType));
			}
		}

		// sort 
		void Sort()
		{
			::Sort(GetData(), Num());
		}
		template <class Predicate>
		void Sort(const Predicate& Pred)
		{
			::Sort(GetData(), Num(), Pred);
		}
		void StableSort()
		{
			::StableSort(GetData(), Num());
		}
		template <class Predicate>
		void StableSort(const Predicate& Pred)
		{
			::StableSort(GetData(), Num(), Pred);
		}

		// Iterators
		typedef TIndexedContainerIterator<      TArray, ElementType, SizeType> TIterator;
		typedef TIndexedContainerIterator<const TArray, const ElementType, SizeType> TConstIterator;
		TIterator CreateIterator()
		{
			return TIterator(*this);
		}
		TConstIterator CreateConstIterator() const
		{
			return TConstIterator(*this);
		}

		// support heap 
		void Heapify()
		{
			Heapify(TLess<ElementType>());
		}
		SizeType HeapPush(ElementType&& InItem)
		{
			return HeapPush(std::move(InItem), TLess<ElementType>());
		}
		SizeType HeapPush(const ElementType& InItem)
		{
			return HeapPush(InItem, TLess<ElementType>());
		}
		void HeapPop(ElementType& OutItem, bool bAllowShrinking = true)
		{
			HeapPop(OutItem, TLess<ElementType>(), bAllowShrinking);
		}
		void HeapPopDiscard(bool bAllowShrinking = true)
		{
			HeapPopDiscard(TLess<ElementType>(), bAllowShrinking);
		}
		const ElementType& HeapTop() const
		{
			return (*this)[0];
		}
		ElementType& HeapTop()
		{
			return (*this)[0];
		}
		void HeapRemoveAt(SizeType Index, bool bAllowShrinking = true)
		{
			HeapRemoveAt(Index, TLess< ElementType >(), bAllowShrinking);
		}
		void HeapSort()
		{
			HeapSort(TLess<ElementType>());
		}
		template <class Predicate>
		FORCEINLINE void Heapify(Predicate&& Pred)
		{
			TDereferenceWrapper<ElementType, Predicate> PredicateWrapper(std::move(Pred));
			::Fuko::Algo::Heapify(*this, PredicateWrapper);
		}
		template <class Predicate>
		SizeType HeapPush(ElementType&& InItem, Predicate&& Pred)
		{
			// 在尾部添加元素，然后向上检索堆
			Add(std::move(InItem));
			TDereferenceWrapper<ElementType, Predicate> PredicateWrapper(std::move(Pred));
			SizeType Result = ::Fuko::Algo::Impl::HeapSiftUp(GetData(), 0, Num() - 1, FIdentityFunctor(), PredicateWrapper);

			return Result;
		}
		template <class Predicate>
		SizeType HeapPush(const ElementType& InItem, Predicate&& Pred)
		{
			// 在尾部添加元素，然后向上检索堆
			Add(InItem);
			TDereferenceWrapper<ElementType, Predicate> PredicateWrapper(std::move(Pred));
			SizeType Result = ::Fuko::Algo::Impl::HeapSiftUp(GetData(), 0, Num() - 1, FIdentityFunctor(), PredicateWrapper);

			return Result;
		}
		template <class Predicate>
		void HeapPop(ElementType& OutItem, Predicate&& Pred, bool bAllowShrinking = true)
		{
			OutItem = std::move((*this)[0]);
			RemoveAtSwap(0, 1, bAllowShrinking);

			TDereferenceWrapper< ElementType, Predicate> PredicateWrapper(std::move(Pred));
			::Fuko::Algo::Impl::HeapSiftDown(GetData(), 0, Num(), FIdentityFunctor(), PredicateWrapper);
		}
		template <class Predicate>
		void VerifyHeap(Predicate&& Pred)
		{
			check(::Fuko::Algo::IsHeap(*this, std::move(Pred)));
		}
		template <class Predicate>
		void HeapPopDiscard(Predicate&& Pred, bool bAllowShrinking = true)
		{
			RemoveAtSwap(0, 1, bAllowShrinking);
			TDereferenceWrapper<ElementType, Predicate> PredicateWrapper(std::move(Pred));
			::Fuko::Algo::Impl::HeapSiftDown(GetData(), 0, Num(), FIdentityFunctor(), PredicateWrapper);
		}
		template <class Predicate>
		void HeapRemoveAt(SizeType Index, Predicate& Pred, bool bAllowShrinking = true)
		{
			RemoveAtSwap(Index, 1, bAllowShrinking);

			TDereferenceWrapper< ElementType, Predicate> PredicateWrapper(std::move(Pred));
			::Fuko::Algo::Impl::HeapSiftDown(GetData(), Index, Num(), FIdentityFunctor(), PredicateWrapper);
			::Fuko::Algo::Impl::HeapSiftUp(GetData(), 0, FMath::Min(Index, Num() - 1), FIdentityFunctor(), PredicateWrapper);
		}
		template <class Predicate>
		void HeapSort(const Predicate& Pred)
		{
			TDereferenceWrapper<ElementType, Predicate> PredicateWrapper(Pred);
			::Fuko::Algo::HeapSort(*this, PredicateWrapper);
		}

		// support foreach 
#if FUKO_DEBUG
		typedef TCheckedPointerIterator<      ElementType, SizeType> RangedForIteratorType;
		typedef TCheckedPointerIterator<const ElementType, SizeType> RangedForConstIteratorType;
		FORCEINLINE RangedForIteratorType      begin() { return RangedForIteratorType(m_Num, GetData()); }
		FORCEINLINE RangedForConstIteratorType begin() const { return RangedForConstIteratorType(m_Num, GetData()); }
		FORCEINLINE RangedForIteratorType      end() { return RangedForIteratorType(m_Num, GetData() + Num()); }
		FORCEINLINE RangedForConstIteratorType end() const { return RangedForConstIteratorType(m_Num, GetData() + Num()); }
#else
		FORCEINLINE ElementType*      	begin() { return GetData(); }
		FORCEINLINE const ElementType* 	begin() const { return GetData(); }
		FORCEINLINE ElementType*      	end() { return GetData() + Num(); }
		FORCEINLINE const ElementType* 	end() const { return GetData() + Num(); }
#endif		
	};
}

// TypeTraits
template <typename T>
struct TIsContiguousContainer<Fuko::TArray<T>>
{
	enum { value = true };
};

// Operater new
template <typename T> void* operator new(size_t Size, Fuko::TArray<T>& Array)
{
	check(Size == sizeof(T));
	const auto Index = Array.AddUninitialized(1);
	return &Array[Index];
}
template <typename T> void* operator new(size_t Size, Fuko::TArray<T>& Array, typename Fuko::TArray<T>::SizeType Index)
{
	check(Size == sizeof(T));
	Array.InsertUninitialized(Index);
	return &Array[Index];
}
