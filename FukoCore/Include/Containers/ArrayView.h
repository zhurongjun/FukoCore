#pragma once
#include "CoreConfig.h"
#include "CoreType.h"
#include "Templates/Sorting.h"
#include "Containers/Array.h"

// ArrayView
namespace Fuko
{
	template<typename InElementType>
	class TArrayView
	{
	public:
		using ElementType = InElementType;

	public:
		TArrayView() = default;
		template <typename OtherRangeType>
		FORCEINLINE TArrayView(OtherRangeType&& Other)
			: DataPtr(::GetData(std::forward<OtherRangeType>(Other)))
			, ArrayNum(::GetNum(std::forward<OtherRangeType>(Other)))
		{
			using OtherElementType = decltype(::GetData(std::forward<OtherRangeType>(Other)));
			using OtherCountType = decltype(GetNum(std::forward<OtherRangeType>(Other)));

			static_assert(std::is_convertible_v<OtherElementType, ElementType >> , "OtherElementType must convertible to OwnElementType");
			static_assert(TIsContiguousContainer_v<OtherRangeType>, "OtherRangeType must a ContiguousContainer");
			check((ArrayNum >= 0) && ((sizeof(OtherCountType) < sizeof(int32))
				|| (InCount <= static_cast<decltype(InCount)>(std::numeric_limits<int32>::max()))));
		}

		template <typename OtherElementType>
		FORCEINLINE TArrayView(OtherElementType* InData, int32 InCount)
			: DataPtr(InData)
			, ArrayNum(InCount)
		{
			static_assert(std::is_convertible_v<OtherElementType, ElementType >>, "OtherElementType must convertible to OwnElementType");
			check(ArrayNum >= 0);
		}

		FORCEINLINE TArrayView(std::initializer_list<ElementType> List)
			: DataPtr(::GetData(List))
			, ArrayNum(::GetNum(List))
		{ }

	public:

		FORCEINLINE ElementType* GetData() const
		{
			return DataPtr;
		}

		FORCEINLINE size_t GetTypeSize() const
		{
			return sizeof(ElementType);
		}

		FORCEINLINE void CheckInvariants() const
		{
			check(ArrayNum >= 0);
		}

		FORCEINLINE void RangeCheck(int32 Index) const
		{
			CheckInvariants();

			checkf((Index >= 0) & (Index < ArrayNum), TEXT("Array index out of bounds: %i from an array of size %i"), Index, ArrayNum); // & for one branch
		}

		FORCEINLINE bool IsValidIndex(int32 Index) const
		{
			return (Index >= 0) && (Index < ArrayNum);
		}

		FORCEINLINE int32 Num() const
		{
			return ArrayNum;
		}

		FORCEINLINE ElementType& operator[](int32 Index) const
		{
			RangeCheck(Index);
			return GetData()[Index];
		}

		FORCEINLINE ElementType& Last(int32 IndexFromTheEnd = 0) const
		{
			RangeCheck(ArrayNum - IndexFromTheEnd - 1);
			return GetData()[ArrayNum - IndexFromTheEnd - 1];
		}

		FORCEINLINE TArrayView Slice(int32 Index, int32 InNum) const
		{
			check(InNum > 0);
			check(IsValidIndex(Index));
			check(IsValidIndex(Index + InNum - 1));
			return TArrayView(DataPtr + Index, InNum);
		}

		FORCEINLINE bool Find(const ElementType& Item, int32& Index) const
		{
			Index = this->Find(Item);
			return Index != INDEX_NONE;
		}

		int32 Find(const ElementType& Item) const
		{
			const ElementType* RESTRICT Start = GetData();
			for (const ElementType* RESTRICT Data = Start, *RESTRICT DataEnd = Data + ArrayNum; Data != DataEnd; ++Data)
			{
				if (*Data == Item)
				{
					return static_cast<int32>(Data - Start);
				}
			}
			return INDEX_NONE;
		}

		FORCEINLINE bool FindLast(const ElementType& Item, int32& Index) const
		{
			Index = this->FindLast(Item);
			return Index != INDEX_NONE;
		}

		template <typename Predicate>
		int32 FindLastByPredicate(Predicate Pred, int32 StartIndex) const
		{
			check(StartIndex >= 0 && StartIndex <= this->Num());
			for (const ElementType* RESTRICT Start = GetData(), *RESTRICT Data = Start + StartIndex; Data != Start; )
			{
				--Data;
				if (Pred(*Data))
				{
					return static_cast<int32>(Data - Start);
				}
			}
			return INDEX_NONE;
		}

		template <typename Predicate>
		FORCEINLINE int32 FindLastByPredicate(Predicate Pred) const
		{
			return FindLastByPredicate(Pred, ArrayNum);
		}

		template <typename KeyType>
		int32 IndexOfByKey(const KeyType& Key) const
		{
			const ElementType* RESTRICT Start = GetData();
			for (const ElementType* RESTRICT Data = Start, *RESTRICT DataEnd = Start + ArrayNum; Data != DataEnd; ++Data)
			{
				if (*Data == Key)
				{
					return static_cast<int32>(Data - Start);
				}
			}
			return INDEX_NONE;
		}

		template <typename Predicate>
		int32 IndexOfByPredicate(Predicate Pred) const
		{
			const ElementType* RESTRICT Start = GetData();
			for (const ElementType* RESTRICT Data = Start, *RESTRICT DataEnd = Start + ArrayNum; Data != DataEnd; ++Data)
			{
				if (Pred(*Data))
				{
					return static_cast<int32>(Data - Start);
				}
			}
			return INDEX_NONE;
		}

		template <typename KeyType>
		ElementType* FindByKey(const KeyType& Key) const
		{
			for (ElementType* RESTRICT Data = GetData(), *RESTRICT DataEnd = Data + ArrayNum; Data != DataEnd; ++Data)
			{
				if (*Data == Key)
				{
					return Data;
				}
			}

			return nullptr;
		}

		template <typename Predicate>
		ElementType* FindByPredicate(Predicate Pred) const
		{
			for (ElementType* RESTRICT Data = GetData(), *RESTRICT DataEnd = Data + ArrayNum; Data != DataEnd; ++Data)
			{
				if (Pred(*Data))
				{
					return Data;
				}
			}

			return nullptr;
		}

		template <typename Predicate>
		TArray<std::remove_const_t<ElementType>> FilterByPredicate(Predicate Pred) const
		{
			TArray<std::remove_const_t<ElementType>> FilterResults;
			for (const ElementType* RESTRICT Data = GetData(), *RESTRICT DataEnd = Data + ArrayNum; Data != DataEnd; ++Data)
			{
				if (Pred(*Data))
				{
					FilterResults.Add(*Data);
				}
			}
			return FilterResults;
		}

		template <typename ComparisonType>
		bool Contains(const ComparisonType& Item) const
		{
			for (const ElementType* RESTRICT Data = GetData(), *RESTRICT DataEnd = Data + ArrayNum; Data != DataEnd; ++Data)
			{
				if (*Data == Item)
				{
					return true;
				}
			}
			return false;
		}

		template <typename Predicate>
		FORCEINLINE bool ContainsByPredicate(Predicate Pred) const
		{
			return FindByPredicate(Pred) != nullptr;
		}

	public:
		FORCEINLINE ElementType* begin() const { return GetData(); }
		FORCEINLINE ElementType* end() const { return GetData() + Num(); }

	public:
		void Sort()
		{
			::Sort(GetData(), Num());
		}

		template <class PREDICATE_CLASS>
		void Sort(const PREDICATE_CLASS& Predicate)
		{
			::Sort(GetData(), Num(), Predicate);
		}

		void StableSort()
		{
			::StableSort(GetData(), Num());
		}

		template <class PREDICATE_CLASS>
		void StableSort(const PREDICATE_CLASS& Predicate)
		{
			::StableSort(GetData(), Num(), Predicate);
		}

	private:
		ElementType* DataPtr;
		int32 ArrayNum;
	};
}

// TypeTraits
namespace Fuko
{
	template <typename InElementType>
	struct TIsZeroConstructType<TArrayView<InElementType>>
	{
		enum { Value = true };
	};

	template <typename T>
	struct TIsContiguousContainer<TArrayView<T>>
	{
		enum { Value = true };
	};
}

// HelpFunctions
namespace Fuko
{
	template <typename OtherRangeType>
	auto MakeArrayView(OtherRangeType&& Other)
	{
		return TArrayView<std::remove_pointer_t<decltype(GetData(std::declval<OtherRangeType&>()))>>(std::forward<OtherRangeType>(Other));
	}

	template<typename ElementType>
	auto MakeArrayView(ElementType* Pointer, int32 Size)
	{
		return TArrayView<ElementType>(Pointer, Size);
	}

	template <typename T>
	TArrayView<const T> MakeArrayView(std::initializer_list<T> List)
	{
		return TArrayView<const T>(List);
	}
}