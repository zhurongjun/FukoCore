#pragma once
#include "CoreConfig.h"
#include "Templates/TypeTraits.h"
#include "Math/MathUtility.h"
#include "Templates/UtilityTemp.h"
#include "Algo/Sort.h"
#include "CoreMinimal/Assert.h"
#include "Templates/Sorting.h"
#include "AllocatorsPmr.h"

// 检测是否在迭代期间修改数组 
#if FUKO_DEBUG
#define TARRAY_RANGED_FOR_CHECKS 1
#else
#define TARRAY_RANGED_FOR_CHECKS 0
#endif

// 迭代器
namespace Fuko
{
	// 通用迭代器
	template< typename ContainerType, typename ElementType, typename size_t>
	class TIndexedContainerIterator
	{
	public:
		TIndexedContainerIterator(ContainerType& InContainer, size_t StartIndex = 0)
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

		TIndexedContainerIterator& operator+=(size_t Offset)
		{
			Index += Offset;
			return *this;
		}

		TIndexedContainerIterator operator+(size_t Offset) const
		{
			TIndexedContainerIterator Tmp(*this);
			return Tmp += Offset;
		}

		TIndexedContainerIterator& operator-=(size_t Offset)
		{
			return *this += -Offset;
		}

		TIndexedContainerIterator operator-(size_t Offset) const
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

		size_t GetIndex() const
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
		size_t      Index;
	};
	// 重载+  
	template <typename ContainerType, typename ElementType, typename size_t>
	FORCEINLINE TIndexedContainerIterator<ContainerType, ElementType, size_t> operator+(size_t Offset, TIndexedContainerIterator<ContainerType, ElementType, size_t> RHS)
	{
		return RHS + Offset;
	}

	// 检测迭代期间是否发生改变
#if TARRAY_RANGED_FOR_CHECKS
	template <typename ElementType, typename size_t>
	struct TCheckedPointerIterator
	{
		explicit TCheckedPointerIterator(const size_t& InNum, ElementType* InPtr)
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
		const size_t& CurrentNum;
		size_t        InitialNum;

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
	template <typename FromArrayType, typename ToArrayType>
	struct TCanMoveTArrayPointersBetweenArrayTypes
	{
		typedef typename FromArrayType::Allocator   FromAllocatorType;
		typedef typename ToArrayType::Allocator   ToAllocatorType;
		typedef typename FromArrayType::ElementType FromElementType;
		typedef typename ToArrayType::ElementType ToElementType;

		enum
		{
			Value =
			std::is_same_v<FromAllocatorType, ToAllocatorType> || 
			TCanMoveBetweenAllocators<FromAllocatorType, ToAllocatorType>::Value &&
			(
				std::is_same_v       <ToElementType, FromElementType> ||
				TIsBitwiseConstructible<ToElementType, FromElementType>
			)
		};
	};
	
	template<typename T>
	class TArray
	{
	private:
		void FreeArray()
		{
			DestructItems(m_Elements, m_Num);
			if (m_Elements)
			{
				m_Allocator->Free(m_Elements);
				m_Elements = nullptr;
			}
			m_Num = 0;
			m_Max = 0;
		}

		template <typename OtherElementType>
		void CopyToEmpty(const OtherElementType* OtherData, size_t OtherNum, size_t ExtraSlack)
		{
			check(ExtraSlack >= 0);

			m_Num = OtherNum;

			if (OtherNum || ExtraSlack)
			{
				size_t NewMax = OtherNum + ExtraSlack;
				
				if (NewMax > m_Max) ResizeTo(NewMax);
				ConstructItems<ElementType>(GetData(), OtherData, OtherNum);
			}
		}

		FORCENOINLINE void ResizeGrow()
		{
			const size_t NewMax = m_Allocator->GetGrow(m_Num, m_Max, ElementSize, ElementAlign);
			ResizeTo(NewMax);
		}
		FORCENOINLINE void ResizeShrink()
		{
			const size_t NewMax = m_Allocator->GetShrink(m_Num, m_Max, ElementSize, ElementAlign);
			check(NewMax >= m_Num);
			ResizeTo(NewMax);
		}
		FORCENOINLINE void ResizeTo(size_t NewMax)
		{
			if (NewMax != m_Max)
			{
				m_Max = NewMax;
				if (m_Elements)
					m_Elements = (ElementType*)m_Allocator->Realloc(m_Elements, m_Max * ElementSize, ElementAlign);
				else
					m_Elements = (ElementType*)m_Allocator->Alloc(m_Max * ElementSize, ElementAlign);
			}
		}
	public:
		using ElementType = T;

		static constexpr size_t ElementAlign = alignof(ElementType);
		static constexpr size_t ElementSize = sizeof(ElementType);

	protected:
		IAllocator*		m_Allocator;
		ElementType*	m_Elements;
		size_t			m_Num;
		size_t			m_Max;

	public:
		// construct 
		FORCEINLINE TArray(IAllocator* Alloc = DefaultAllocator())
			: m_Elements(nullptr)
			, m_Num(0)
			, m_Max(0)
			, m_Allocator(Alloc)
		{
			check(m_Allocator != nullptr);
		}
		FORCEINLINE TArray(const ElementType* Ptr, size_t Count, IAllocator* Alloc = DefaultAllocator())
			: m_Elements(nullptr)
			, m_Num(0)
			, m_Max(0)
			, m_Allocator(Alloc)
		{
			check(Ptr != nullptr || Count == 0);
			check(m_Allocator != nullptr);

			CopyToEmpty(Ptr, Count, 0);
		}
		FORCEINLINE TArray(std::initializer_list<ElementType> InitList, IAllocator* Alloc = DefaultAllocator())
			: m_Elements(nullptr)
			, m_Num(0)
			, m_Max(0)
			, m_Allocator(Alloc)
		{
			check(m_Allocator != nullptr);

			CopyToEmpty(InitList.begin(), (size_t)InitList.size(), 0);
		}

		// copy construct 
		template <typename OtherElementType>
		FORCEINLINE explicit TArray(const TArray<OtherElementType>& Other, IAllocator* Alloc = DefaultAllocator())
			: m_Elements(nullptr)
			, m_Num(0)
			, m_Max(0)
			, m_Allocator(Alloc)
		{
			CopyToEmpty(Other.GetData(), Other.Num(), 0);
		}
		FORCEINLINE TArray(const TArray& Other, IAllocator* Alloc = DefaultAllocator())
			: m_Elements(nullptr)
			, m_Num(0)
			, m_Max(0)
			, m_Allocator(Alloc)
		{
			CopyToEmpty(Other.GetData(), Other.Num(), 0);
		}
		FORCEINLINE TArray(const TArray& Other, size_t ExtraSlack, IAllocator* Alloc = DefaultAllocator())
			: m_Elements(nullptr)
			, m_Num(0)
			, m_Max(0)
			, m_Allocator(Alloc)
		{
			CopyToEmpty(Other.GetData(), Other.Num(), ExtraSlack);
		}

		// assign operator 
		TArray& operator=(std::initializer_list<ElementType> InitList)
		{
			DestructItems(m_Elements, m_Num);
			CopyToEmpty(InitList.begin(), (size_t)InitList.size(), 0);
			return *this;
		}
		TArray& operator=(const TArray& Other)
		{
			if (this != &Other)
			{
				DestructItems(m_Elements, m_Num);
				CopyToEmpty(Other.GetData(), Other.Num(), m_Max, 0);
			}
			return *this;
		}
		
		// move construct 
		FORCEINLINE TArray(TArray&& Other)
			: m_Elements(Other.m_Elements)
			, m_Num(Other.m_Num)
			, m_Max(Other.m_Max)
			, m_Allocator(Other.m_Allocator)
		{
			Other.m_Elements = nullptr;
			Other.m_Num = 0;
			Other.m_Max = 0;
		}
	
		// move assign operator 
		TArray& operator=(TArray&& Other)
		{
			if (this != &Other)
			{
				DestructItems(m_Elements, m_Num);
				m_Elements = Other.m_Elements;
				m_Num = Other.m_Num;
				m_Max = Other.m_Max;
				m_Allocator = Other.m_Allocator;

				Other.m_Elements = nullptr;
				Other.m_Num = Other.m_Max = 0;
			}
			return *this;
		}
		
		// destruct 
		~TArray()
		{
			FreeArray();
		}
	public:
		// get infomation 
		FORCEINLINE ElementType* GetData() { return m_Elements; }
		FORCEINLINE const ElementType* GetData() const { return m_Elements; }
		FORCEINLINE uint32 GetTypeSize() const { return ElementSize; }
		FORCEINLINE size_t Num() const { return m_Num; }
		FORCEINLINE size_t Max() const { return m_Max; }
		FORCEINLINE size_t GetSlack() const { return m_Max - m_Num; }
		FORCEINLINE const IAllocator* GetAllocator() const { return m_Allocator; }
		FORCEINLINE IAllocator* GetAllocator() { return m_Allocator; }

		// compare
		FORCEINLINE bool operator==(const TArray& Rhs) const
		{
			return m_Num == Rhs.m_Num && CompareItems(m_Elements, Rhs.m_Elements, m_Num);
		}
		FORCEINLINE bool operator!=(const TArray& OtherArray) const
		{
			return !(*this == OtherArray);
		}

		// debug check
		FORCEINLINE void CheckInvariants() const
		{
			check((m_Num >= 0) & (m_Max >= m_Num));
		}
		FORCEINLINE void CheckAddress(const ElementType* Addr) const
		{
			checkf(Addr < m_Elements || Addr >= (m_Elements + m_Max), 
				TEXT("Attempting to use a container element (%p) which already comes from the container being modified (%p, m_Max: %d, m_Num: %d, SizeofElement: %d)!")
				, Addr, m_Elements, m_Max, m_Num, ElementSize);
		}
		FORCEINLINE void RangeCheck(size_t Index) const
		{
			CheckInvariants();
			checkf((Index >= 0) & (Index < m_Num), TEXT("Array index out of bounds: %i from an array of size %i"), Index, m_Num);
		}
		
		// runtime check 
		FORCEINLINE bool IsValidIndex(size_t Index) const { return Index >= 0 && Index < m_Num; }

		// operator []
		FORCEINLINE ElementType& operator[](size_t Index)
		{
			RangeCheck(Index);
			return GetData()[Index];
		}
		FORCEINLINE const ElementType& operator[](size_t Index) const
		{
			RangeCheck(Index);
			return GetData()[Index];
		}

		// get element 
		FORCEINLINE ElementType& Top() { return Last(); }
		FORCEINLINE const ElementType& Top() const { return Last(); }
		FORCEINLINE ElementType& Last(size_t IndexFromTheEnd = 0)
		{
			RangeCheck(m_Num - IndexFromTheEnd - 1);
			return GetData()[m_Num - IndexFromTheEnd - 1];
		}
		FORCEINLINE const ElementType& Last(size_t IndexFromTheEnd = 0) const
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
		FORCEINLINE void Push(ElementType&& Item) { Add(std::move(Item)); }
		FORCEINLINE void Push(const ElementType& Item) { Add(Item); }

		// shrink & reserve 
		FORCEINLINE void Shrink()
		{
			CheckInvariants();
			if (m_Max != m_Num)
			{
				ResizeTo(m_Num);
			}
		}
		FORCEINLINE void Reserve(size_t Number)
		{
			check(Number >= 0);
			if (Number > m_Max)
			{
				ResizeTo(Number);
			}
		}

		// set num & empty 
		void Reset(size_t NewSize = 0)
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
		void Empty(size_t Slack = 0)
		{
			check(Slack >= 0);

			DestructItems(GetData(), m_Num);
			m_Num = 0;
			if (m_Max != Slack) ResizeTo(Slack);
		}
		void SetNum(size_t NewNum, bool bAllowShrinking = true)
		{
			if (NewNum > Num())
			{
				const size_t Diff = NewNum - m_Num;
				const size_t Index = AddUninitialized(Diff);
				DefaultConstructItems<ElementType>(GetData() + Index, Diff);
			}
			else if (NewNum < Num())
			{
				RemoveAt(NewNum, Num() - NewNum, bAllowShrinking);
			}
		}
		void SetNumZeroed(size_t NewNum, bool bAllowShrinking = true)
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
		void SetNumUninitialized(size_t NewNum, bool bAllowShrinking = true)
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
		void SetNumUnsafe(size_t NewNum)
		{
			check(NewNum <= Num() && NewNum >= 0);
			m_Num = NewNum;
		}

		// find 
		template<typename KeyType>
		ElementType* Find(const KeyType& Item)
		{
			const ElementType* RESTRICT Ptr = m_Elements;
			const ElementType* RESTRICT End = m_Elements + m_Num;
			for (; Ptr != End; ++Ptr)
			{
				if (*Ptr == Item) return Ptr;
			}
			return nullptr;
		}
		template<typename KeyType>
		ElementType* FindLast(const KeyType& Item)
		{
			const ElementType* RESTRICT Ptr = m_Elements + m_Num;
			for (; Ptr != m_Elements; --Ptr)
			{
				if (*Ptr == Item) return Ptr;
			}
			return nullptr;
		}
		template<typename Predicate>
		ElementType* FindBy(Predicate&& Pred)
		{
			const ElementType* RESTRICT Ptr = m_Elements;
			const ElementType* RESTRICT End = m_Elements + m_Num;
			for (; Ptr != End; ++Ptr)
			{
				if (Pred(*Ptr)) return Ptr;
			}
			return nullptr;
		}
		template<typename Predicate>
		ElementType* FindLastBy(Predicate&& Pred)
		{
			const ElementType* RESTRICT Ptr = m_Elements + m_Num;
			for (; Ptr != m_Elements; --Ptr)
			{
				if (Pred(*Ptr)) return Ptr;
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
		FORCEINLINE size_t IndexOf(const KeyType& Item) const
		{
			const ElementType* FindPtr = this->Find(Item);
			return FindPtr == nullptr ? INDEX_NONE : FindPtr - m_Elements;
		}
		template<typename KeyType>
		FORCEINLINE size_t IndexOfLast(const KeyType& Item) const
		{
			const ElementType* FindPtr = this->FindLast(Item);
			return FindPtr == nullptr ? INDEX_NONE : FindPtr - m_Elements;
		}
		template<typename Predicate>
		FORCEINLINE size_t IndexOfBy(Predicate Pred) const
		{
			const ElementType* FindPtr = this->FindBy(Pred);
			return FindPtr == nullptr ? INDEX_NONE : FindPtr - m_Elements;
		}
		template<typename Predicate>
		FORCEINLINE size_t IndexOfByLastBy(Predicate Pred) const
		{
			const ElementType* FindPtr = this->FindLastBy(Pred);
			return FindPtr == nullptr ? INDEX_NONE : FindPtr - m_Elements;
		}
		
		// filter item 
		template<typename Predicate>
		void FilterBy(Predicate&& Pred, TArray<ElementType>& OutArray) const
		{
			const ElementType* RESTRICT Ptr = m_Elements;
			const ElementType* RESTRICT End = m_Elements + m_Num;
			for (; Ptr != End; ++Ptr)
			{
				if (Pred(*Ptr)) OutArray.Add(*Ptr);
			}
		}
		template<typename Predicate>
		TArray<ElementType> FilterBy(Predicate&& Pred) const
		{
			TArray<ElementType> OutArray;
			const ElementType* RESTRICT Ptr = m_Elements;
			const ElementType* RESTRICT End = m_Elements + m_Num;
			for (; Ptr != End; ++Ptr)
			{
				if (Pred(*Ptr)) OutArray.Add(*Ptr);
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
		void InsertUninitialized(size_t Index, size_t Count = 1)
		{
			CheckInvariants();
			check((Count >= 0) && (Index >= 0) && (Index <= m_Num) && (m_Num + Count > 0));

			const size_t OldNum = m_Num;
			m_Num += Count;
			if (m_Num > m_Max) ResizeGrow(OldNum);

			ElementType* Ptr = m_Elements + Index;
			RelocateConstructItems<ElementType>(Ptr + Count, Ptr, OldNum - Index);
		}
		FORCEINLINE void InsertZeroed(size_t Index, size_t Count = 1)
		{
			InsertUninitialized(Index, Count);
			Memzero(m_Elements + Index, Count * ElementSize);
		}
		FORCEINLINE ElementType& InsertZeroed_GetRef(size_t Index)
		{
			InsertUninitialized(Index, 1);
			ElementType* Ptr = m_Elements+ Index;
			Memzero(Ptr, ElementSize);
			return *Ptr;
		}
		FORCEINLINE void InsertDefaulted(size_t Index, size_t Count = 1)
		{
			InsertUninitialized(Index, Count);
			DefaultConstructItems<ElementType>(m_Elements + Index, Count);
		}
		FORCEINLINE ElementType& InsertDefaulted_GetRef(size_t Index)
		{
			InsertUninitialized(Index, 1);
			ElementType* Ptr = m_Elements + Index;
			DefaultConstructItems<ElementType>(Ptr, 1);
			return *Ptr;
		}

		// insert 
		FORCEINLINE void Insert(std::initializer_list<ElementType> InitList, const size_t InIndex)
		{
			InsertUninitialized(InIndex, InitList.size());
			ConstructItems<ElementType>(m_Elements + InIndex, InitList.begin(), InitList.size());
		}
		FORCEINLINE void Insert(const TArray& Items, const size_t InIndex)
		{
			check(this != &Items);
			InsertUninitialized(InIndex, Items.m_Num);
			ConstructItems<ElementType>(m_Elements + InIndex, Items.m_Elements, Items.m_Num);
		}
		FORCEINLINE void Insert(TArray&& Items, const size_t InIndex)
		{
			check(this != &Items);
			InsertUninitialized(InIndex, Items.Num());
			MoveConstructItems<ElementType>(m_Elements + InIndex, Items.m_Elements, Items.m_Num);
			Items.m_Num = 0;
		}
		FORCEINLINE void Insert(const ElementType* Ptr, size_t Count, size_t Index)
		{
			check(Ptr != nullptr);
			InsertUninitialized(Index, Count);
			ConstructItems<ElementType>(GetData() + Index, Ptr, Count);
		}
		FORCEINLINE void Insert(ElementType&& Item, size_t Index)
		{
			CheckAddress(&Item);
			InsertUninitialized(Index, 1);
			new(m_Elements + Index) ElementType(std::move(Item));
		}
		FORCEINLINE void Insert(const ElementType& Item, size_t Index)
		{
			CheckAddress(&Item);
			InsertUninitialized(Index, 1);
			new(m_Elements + Index) ElementType(Item);
		}
		FORCEINLINE ElementType& Insert_GetRef(ElementType&& Item, size_t Index)
		{
			CheckAddress(&Item);

			InsertUninitialized(Index, 1);
			ElementType* Ptr = GetData() + Index;
			new(Ptr) ElementType(std::move(Item));
			return *Ptr;
		}
		FORCEINLINE ElementType& Insert_GetRef(const ElementType& Item, size_t Index)
		{
			CheckAddress(&Item);

			InsertUninitialized(Index, 1);
			ElementType* Ptr = GetData() + Index;
			new(Ptr) ElementType(Item);
			return *Ptr;
		}

		// add
		FORCEINLINE size_t AddUninitialized(size_t Count = 1)
		{
			CheckInvariants();
			check(Count >= 0);

			const size_t OldNum = m_Num;
			if ((m_Num += Count) > m_Max)
			{
				ResizeGrow(OldNum);
			}
			return OldNum;
		}
		FORCEINLINE size_t Add(ElementType&& Item)
		{
			CheckAddress(&Item);
			return Emplace(std::move(Item));
		}
		FORCEINLINE size_t Add(const ElementType& Item)
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
		FORCEINLINE size_t Add(size_t Count = 1)
		{
			const size_t Index = AddUninitialized(Count);
			DefaultConstructItems<ElementType>(GetData() + Index, Count);
			return Index;
		}
		FORCEINLINE ElementType& Add_GetRef()
		{
			const size_t Index = AddUninitialized(1);
			ElementType* Ptr = GetData() + Index;
			DefaultConstructItems<ElementType>(Ptr, 1);
			return *Ptr;
		}
		FORCEINLINE size_t AddZeroed(size_t Count = 1)
		{
			const size_t Index = AddUninitialized(Count);
			Memzero(GetData() + Index, Count * ElementSize);
			return Index;
		}
		FORCEINLINE ElementType& AddZeroed_GetRef()
		{
			const size_t Index = AddUninitialized(1);
			ElementType* Ptr = GetData() + Index;
			Memzero(Ptr, ElementSize);
			return *Ptr;
		}

		// emplace 
		template <typename... ArgsType>
		FORCEINLINE size_t Emplace(ArgsType&&... Args)
		{
			const size_t Index = AddUninitialized();
			new(GetData() + Index) ElementType(std::forward<ArgsType>(Args)...);
			return Index;
		}
		template <typename... ArgsType>
		FORCEINLINE ElementType& Emplace_GetRef(ArgsType&&... Args)
		{
			const size_t Index = AddUninitialized();
			ElementType* Ptr = GetData() + Index;
			new(Ptr) ElementType(std::forward<ArgsType>(Args)...);
			return *Ptr;
		}
		template <typename... ArgsType>
		FORCEINLINE void EmplaceAt(size_t Index, ArgsType&&... Args)
		{
			InsertUninitialized(Index);
			new(GetData() + Index) ElementType(std::forward<ArgsType>(Args)...);
		}
		template <typename... ArgsType>
		FORCEINLINE ElementType& EmplaceAt_GetRef(size_t Index, ArgsType&&... Args)
		{
			InsertUninitialized(Index);
			ElementType* Ptr = GetData() + Index;
			new(Ptr) ElementType(std::forward<ArgsType>(Args)...);
			return *Ptr;
		}

		// add unique 
		FORCEINLINE size_t AddUnique(ElementType&& Item) 
		{
			size_t Index = IndexOf(Item);
			if (Index != INDEX_NONE) return Index;
			return Add(std::move(Item));
		}
		FORCEINLINE size_t AddUnique(const ElementType& Item) 
		{
			size_t Index = IndexOf(Item);
			if (Index != INDEX_NONE) return Index;
			return Add(Item);
		}

		// remove at
		void RemoveAt(size_t Index, size_t Count = 1, bool bAllowShrinking = true)
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
		void RemoveAtSwap(size_t Index, size_t Count = 1, bool bAllowShrinking = true)
		{
			if (Count)
			{
				CheckInvariants();
				check((Count >= 0) & (Index >= 0) & (Index + Count <= m_Num));

				// destruct 
				DestructItems(GetData() + Index, Count);

				// move memory 
				const size_t NumElementsInHole = Count;
				const size_t NumElementsAfterHole = m_Num - (Index + Count);
				const size_t NumElementsToMoveIntoHole = FMath::Min(NumElementsInHole, NumElementsAfterHole);
				if (NumElementsToMoveIntoHole)
				{
					Memcpy(
						m_Elements + Index,
						m_Elements + (m_Num - NumElementsToMoveIntoHole),
						NumElementsToMoveIntoHole * sizeof(ElementType)
					);
				}
				m_Num -= Count;

				if (bAllowShrinking) ResizeShrink();
			}
		}
		
		// remove
		size_t Remove(const ElementType& Item)
		{
			CheckAddress(&Item);
			return RemoveBy([&Item](ElementType& Element) { return Element == Item; });
		}
		size_t RemoveSwap(const ElementType& Item)
		{
			CheckAddress(&Item);

			const size_t OriginalNum = m_Num;
			for (size_t Index = 0; Index < m_Num; Index++)
			{
				if ((*this)[Index] == Item)
				{
					RemoveAtSwap(Index--);
				}
			}
			return OriginalNum - m_Num;
		}
		template <class Predicate>
		size_t RemoveBy(Predicate&& Pred)
		{
			const size_t OriginalNum = m_Num;
			if (!OriginalNum) return 0;

			size_t WriteIndex = 0;
			size_t ReadIndex = 0;
			bool NotMatch = !Pred(GetData()[ReadIndex]);
			do
			{
				size_t RunStartIndex = ReadIndex++;
				while (ReadIndex < OriginalNum && NotMatch == !Pred(GetData()[ReadIndex]))
				{
					ReadIndex++;
				}
				size_t RunLength = ReadIndex - RunStartIndex;
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
		void RemoveBySwap(const Predicate& Pred, bool bAllowShrinking = true)
		{
			for (size_t ItemIndex = 0; ItemIndex < Num();)
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
		}
		size_t RemoveSingle(const ElementType& Item)
		{
			size_t Index = Find(Item);
			if (Index == INDEX_NONE) return 0;

			auto* RemovePtr = GetData() + Index;

			DestructItems(RemovePtr, 1);
			const size_t NextIndex = Index + 1;
			RelocateConstructItems<ElementType>(RemovePtr, RemovePtr + 1, m_Num - (Index + 1));

			--m_Num;
			return 1;
		}
		size_t RemoveSingleSwap(const ElementType& Item, bool bAllowShrinking = true)
		{
			size_t Index = Find(Item);
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

			size_t Pos = AddUninitialized(Source.Num());
			ConstructItems<ElementType>(GetData() + Pos, Source.GetData(), Source.Num());
		}
		template <typename OtherElementType>
		FORCEINLINE void Append(TArray<OtherElementType>&& Source)
		{
			check((void*)this != (void*)&Source);

			if (!Source.Num()) return;

			size_t Pos = AddUninitialized(Source.Num());
			MoveAssignItems(GetData() + Pos, Source.GetData(), Source.Num());
		}
		FORCEINLINE void Append(const ElementType* Ptr, size_t Count)
		{
			check(Ptr != nullptr || Count == 0);

			size_t Pos = AddUninitialized(Count);
			ConstructItems<ElementType>(GetData() + Pos, Ptr, Count);
		}
		FORCEINLINE void Append(std::initializer_list<ElementType> InitList)
		{
			size_t Count = (size_t)InitList.size();

			size_t Pos = AddUninitialized(Count);
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
		void Init(const ElementType& Element, size_t Number)
		{
			Empty(Number);
			for (size_t Index = 0; Index < Number; ++Index)
			{
				new(*this) ElementType(Element);
			}
		}

		// swap 
		FORCEINLINE void Swap(size_t FirstIndexToSwap, size_t SecondIndexToSwap)
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
		typedef TIndexedContainerIterator<      TArray, ElementType, size_t> TIterator;
		typedef TIndexedContainerIterator<const TArray, const ElementType, size_t> TConstIterator;
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
		size_t HeapPush(ElementType&& InItem)
		{
			return HeapPush(std::move(InItem), TLess<ElementType>());
		}
		size_t HeapPush(const ElementType& InItem)
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
		void HeapRemoveAt(size_t Index, bool bAllowShrinking = true)
		{
			HeapRemoveAt(Index, TLess< ElementType >(), bAllowShrinking);
		}
		void HeapSort()
		{
			HeapSort(TLess<ElementType>());
		}
		template <class Predicate>
		FORCEINLINE void Heapify(const Predicate& Pred)
		{
			TDereferenceWrapper<ElementType, Predicate> PredicateWrapper(Pred);
			::Fuko::Algo::Heapify(*this, PredicateWrapper);
		}
		template <class Predicate>
		size_t HeapPush(ElementType&& InItem, const Predicate& Pred)
		{
			// 在尾部添加元素，然后向上检索堆
			Add(std::move(InItem));
			TDereferenceWrapper<ElementType, Predicate> PredicateWrapper(Pred);
			size_t Result = ::Fuko::Algo::Impl::HeapSiftUp(GetData(), 0, Num() - 1, FIdentityFunctor(), PredicateWrapper);

			return Result;
		}
		template <class Predicate>
		size_t HeapPush(const ElementType& InItem, const Predicate& Pred)
		{
			// 在尾部添加元素，然后向上检索堆
			Add(InItem);
			TDereferenceWrapper<ElementType, Predicate> PredicateWrapper(Pred);
			size_t Result = ::Fuko::Algo::Impl::HeapSiftUp(GetData(), 0, Num() - 1, FIdentityFunctor(), PredicateWrapper);

			return Result;
		}
		template <class Predicate>
		void HeapPop(ElementType& OutItem, const Predicate& Pred, bool bAllowShrinking = true)
		{
			OutItem = std::move((*this)[0]);
			RemoveAtSwap(0, 1, bAllowShrinking);

			TDereferenceWrapper< ElementType, Predicate> PredicateWrapper(Pred);
			::Fuko::Algo::Impl::HeapSiftDown(GetData(), 0, Num(), FIdentityFunctor(), PredicateWrapper);
		}
		template <class Predicate>
		void VerifyHeap(const Predicate& Pred)
		{
			check(Algo::IsHeap(*this, Pred));
		}
		template <class Predicate>
		void HeapPopDiscard(const Predicate& Pred, bool bAllowShrinking = true)
		{
			RemoveAtSwap(0, 1, bAllowShrinking);
			TDereferenceWrapper<ElementType, Predicate> PredicateWrapper(Pred);
			::Fuko::Algo::Impl::HeapSiftDown(GetData(), 0, Num(), FIdentityFunctor(), PredicateWrapper);
		}
		template <class Predicate>
		void HeapRemoveAt(size_t Index, const Predicate& Pred, bool bAllowShrinking = true)
		{
			RemoveAtSwap(Index, 1, bAllowShrinking);

			TDereferenceWrapper< ElementType, Predicate> PredicateWrapper(Pred);
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
		typedef TCheckedPointerIterator<      ElementType, size_t> RangedForIteratorType;
		typedef TCheckedPointerIterator<const ElementType, size_t> RangedForConstIteratorType;
#if TARRAY_RANGED_FOR_CHECKS
		FORCEINLINE RangedForIteratorType      begin() { return RangedForIteratorType(m_Num, GetData()); }
		FORCEINLINE RangedForConstIteratorType begin() const { return RangedForConstIteratorType(m_Num, GetData()); }
		FORCEINLINE RangedForIteratorType      end() { return RangedForIteratorType(m_Num, GetData() + Num()); }
		FORCEINLINE RangedForConstIteratorType end() const { return RangedForConstIteratorType(m_Num, GetData() + Num()); }
#else
		FORCEINLINE RangedForIteratorType      begin() { return GetData(); }
		FORCEINLINE RangedForConstIteratorType begin() const { return GetData(); }
		FORCEINLINE RangedForIteratorType      end() { return GetData() + Num(); }
		FORCEINLINE RangedForConstIteratorType end() const { return GetData() + Num(); }
#endif		
	};
}

// TypeTraits
namespace Fuko
{
	template <typename T>
	struct TIsContiguousContainer<TArray<T>>
	{
		enum { Value = true };
	};
}

// Operater new
template <typename T> void* operator new(size_t Size, Fuko::TArray<T>& Array)
{
	check(Size == sizeof(T));
	const auto Index = Array.AddUninitialized(1);
	return &Array[Index];
}
template <typename T> void* operator new(size_t Size, Fuko::TArray<T>& Array, typename Fuko::TArray<T>::size_t Index)
{
	check(Size == sizeof(T));
	Array.InsertUninitialized(Index);
	return &Array[Index];
}
