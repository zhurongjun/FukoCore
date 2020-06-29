#pragma once
#include "CoreConfig.h"
#include "Templates/TypeTraits.h"
#include "Allocators.h"
#include "Math/MathUtility.h"
#include "Templates/UtilityTemp.h"
#include "Algo/Sort.h"
#include "CoreMinimal/Assert.h"
#include "Templates/Sorting.h"

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
#if TARRAY_RANGED_FOR_CHECKS
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
			TAreTypesEqual_v<FromAllocatorType, ToAllocatorType> || 
			TCanMoveBetweenAllocators<FromAllocatorType, ToAllocatorType>::Value &&
			TContainerTraits<FromArrayType>::MoveWillEmptyContainer &&
			(
				TAreTypesEqual_v       <ToElementType, FromElementType> ||
				TIsBitwiseConstructible<ToElementType, FromElementType>
			)
		};
	};
	
	template<typename InElementType, typename InAllocator>
	class TArray
	{
		template <typename OtherInElementType, typename OtherAllocator>
		friend class TArray;
	public:
		typedef typename InAllocator::SizeType SizeType;
		typedef InElementType	ElementType;
		typedef InAllocator		Allocator;

		// 优先使用无类型的Allocator 
		typedef TChooseClass_t<
			Allocator::NeedsElementType,
			typename Allocator::template ForElementType<ElementType>,
			typename Allocator::ForAnyElementType
		> ElementAllocatorType;

		static_assert(TIsSigned_v<SizeType>, "TArray only supports signed index types");
	
		//-------------------------------------------构造，拷贝，赋值-------------------------------------------
	public:
		/**
		 * @fn FORCEINLINE TArray::TArray() : ArrayNum(0) , ArrayMax(AllocatorInstance.GetInitialCapacity())
		 *
		 * @brief 默认构造
		 *
		 */
		FORCEINLINE TArray()
			: ArrayNum(0)
			, ArrayMax(AllocatorInstance.GetInitialCapacity())
		{}

		/**
		 * @fn FORCEINLINE TArray::TArray(const ElementType* Ptr, SizeType Count)
		 *
		 * @brief 从一个原始数组构造
		 *
		 * @param  Ptr   数组首地址
		 * @param  Count 数组长度
		 *
		 */
		FORCEINLINE TArray(const ElementType* Ptr, SizeType Count)
		{
			check(Ptr != nullptr || Count == 0);

			CopyToEmpty(Ptr, Count, 0, 0);
		}

		/**
		 * @fn TArray::TArray(std::initializer_list<InElementType> InitList)
		 *
		 * @brief 从initializer_list构造 
		 *
		 * @param  initializer_list
		 */
		TArray(std::initializer_list<InElementType> InitList)
		{
			CopyToEmpty(InitList.begin(), (SizeType)InitList.size(), 0, 0);
		}

		/**
		 * @fn template <typename OtherElementType, typename OtherAllocator> FORCEINLINE explicit TArray::TArray(const TArray<OtherElementType, OtherAllocator>& Other)
		 *
		 * @brief 拷贝构造
		 *
		 * @tparam OtherElementType 拷贝源的元素类型
		 * @tparam OtherAllocator   拷贝源的内存分配器
		 * @param  Other 拷贝源
		 */
		template <typename OtherElementType, typename OtherAllocator>
		FORCEINLINE explicit TArray(const TArray<OtherElementType, OtherAllocator>& Other)
		{
			CopyToEmpty(Other.GetData(), Other.Num(), 0, 0);
		}

		/**
		 * @fn FORCEINLINE TArray::TArray(const TArray& Other)
		 *
		 * @brief 拷贝构造
		 *
		 * @param  拷贝源
		 */
		FORCEINLINE TArray(const TArray& Other)
		{
			CopyToEmpty(Other.GetData(), Other.Num(), 0, 0);
		}

		/**
		 * @fn FORCEINLINE TArray::TArray(const TArray& Other, SizeType ExtraSlack)
		 *
		 * @brief 拷贝构造
		 *
		 * @param  Other	  拷贝源
		 * @param  ExtraSlack 预留的额外空间
		 */
		FORCEINLINE TArray(const TArray& Other, SizeType ExtraSlack)
		{
			CopyToEmpty(Other.GetData(), Other.Num(), 0, ExtraSlack);
		}

		/**
		 * @fn TArray& TArray::operator=(std::initializer_list<InElementType> InitList)
		 *
		 * @brief 赋值运算符重载，从initializer_list拷贝
		 *
		 * @param  InitList The initializer_list to copy from.
		 *
		 * @returns 自己的引用
		 */
		TArray& operator=(std::initializer_list<InElementType> InitList)
		{
			DestructItems(GetData(), ArrayNum);
			CopyToEmpty(InitList.begin(), (SizeType)InitList.size(), ArrayMax, 0);
			return *this;
		}

		/**
		 * @fn template<typename OtherAllocator> TArray& TArray::operator=(const TArray<ElementType, OtherAllocator>& Other)
		 *
		 * @brief 使用其它类型的Array赋值
		 *
		 * @param  源数组
		 *
		 * @returns 自己的引用
		 */
		template<typename OtherAllocator>
		TArray& operator=(const TArray<ElementType, OtherAllocator>& Other)
		{
			DestructItems(GetData(), ArrayNum);
			CopyToEmpty(Other.GetData(), Other.Num(), ArrayMax, 0);
			return *this;
		}

		/**
		 * @fn TArray& TArray::operator=(const TArray& Other)
		 *
		 * @brief 赋值运算
		 *
		 * @param  源数组
		 *
		 * @returns 自己的引用
		 */
		TArray& operator=(const TArray& Other)
		{
			if (this != &Other)
			{
				DestructItems(GetData(), ArrayNum);
				CopyToEmpty(Other.GetData(), Other.Num(), ArrayMax, 0);
			}
			return *this;
		}

		//-------------------------------------------移动构造辅助函数-------------------------------------------
	private:

		/**
		 * @fn template <typename FromArrayType, typename ToArrayType> static FORCEINLINE void TArray::MoveOrCopy(ToArrayType& ToArray, FromArrayType& FromArray, SizeType PrevMax)
		 *
		 * @brief 根据情况判断调用移动还是拷贝
		 *
		 * @param [in,out] ToArray   目标数组
		 * @param [in,out] FromArray 源数组
		 * @param 		   PrevMax   目标数组当前分配的大小
		 */
		template <typename FromArrayType, typename ToArrayType>
		static FORCEINLINE void MoveOrCopy(ToArrayType& ToArray, FromArrayType& FromArray, SizeType PrevMax)
		{
			using FromAllocatorType = typename FromArrayType::Allocator;
			using ToAllocatorType = typename ToArrayType::Allocator;

			if constexpr (TCanMoveTArrayPointersBetweenArrayTypes<FromArrayType, ToArrayType>::Value)	// 能否调用移动函数
			{
				if constexpr (TCanMoveBetweenAllocators<FromAllocatorType, ToAllocatorType>::Value)	// Allocator之间是否提供了Move函数
				{
					ToArray.AllocatorInstance.template MoveToEmptyFromOtherAllocator<FromAllocatorType>(FromArray.AllocatorInstance);
				}
				else
				{
					ToArray.AllocatorInstance.MoveToEmpty(FromArray.AllocatorInstance);
				}

				ToArray.ArrayNum = FromArray.ArrayNum;
				ToArray.ArrayMax = FromArray.ArrayMax;

				// 确保数据不会丢失
				checkf(ToArray.ArrayNum == FromArray.ArrayNum && ToArray.ArrayMax == FromArray.ArrayMax, TEXT("Data lost when moving to a container with a more constrained size type"));

				FromArray.ArrayNum = 0;
				FromArray.ArrayMax = FromArray.AllocatorInstance.GetInitialCapacity();
			}
			else
			{
				ToArray.CopyToEmpty(FromArray.GetData(), FromArray.Num(), PrevMax, 0);
			}
		}

		/**
		 * @fn template <typename FromArrayType, typename ToArrayType> static FORCEINLINE void TArray::MoveOrCopyWithSlack(ToArrayType& ToArray, FromArrayType& FromArray, SizeType PrevMax, SizeType ExtraSlack)
		 *
		 * @brief 根据情况判断调用移动还是拷贝
		 * 
		 * @param [in,out] ToArray    目标数组
		 * @param [in,out] FromArray  源数组
		 * @param 		   PrevMax    目标数组当前分配的大小
		 * @param 		   ExtraSlack 预留的闲余空间
		 */
		template <typename FromArrayType, typename ToArrayType>
		static FORCEINLINE void MoveOrCopyWithSlack(ToArrayType& ToArray, FromArrayType& FromArray, SizeType PrevMax, SizeType ExtraSlack)
		{
			if constexpr (TCanMoveTArrayPointersBetweenArrayTypes<FromArrayType, ToArrayType>::Value)
			{
				MoveOrCopy(ToArray, FromArray, PrevMax);
				ToArray.Reserve(ToArray.ArrayNum + ExtraSlack);
			}
			else
			{
				ToArray.CopyToEmpty(FromArray.GetData(), FromArray.Num(), PrevMax, ExtraSlack);
			}
		}

		//-------------------------------------------移动构造-------------------------------------------
	public:
		/**
		 * @fn FORCEINLINE TArray::TArray(TArray&& Other)
		 *
		 * @brief 移动构造
		 *
		 * @param [in] 源数组
		 */
		FORCEINLINE TArray(TArray&& Other)
		{
			MoveOrCopy(*this, Other, 0);
		}

		/**
		 * @fn template <typename OtherElementType, typename OtherAllocator> FORCEINLINE explicit TArray::TArray(TArray<OtherElementType, OtherAllocator>&& Other)
		 *
		 * @brief 移动构造
		 *
		 * @param [in] (不同类型的)源数组
		 */
		template <typename OtherElementType, typename OtherAllocator>
		FORCEINLINE explicit TArray(TArray<OtherElementType, OtherAllocator>&& Other)
		{
			MoveOrCopy(*this, Other, 0);
		}

		/**
		 * @fn template <typename OtherElementType> TArray::TArray(TArray<OtherElementType, Allocator>&& Other, SizeType ExtraSlack)
		 *
		 * @brief 移动构造
		 *
		 * @param [in]	Other		源数组
		 * @param 		ExtraSlack	预留的空间
		 */
		template <typename OtherElementType>
		TArray(TArray<OtherElementType, Allocator>&& Other, SizeType ExtraSlack)
		{
			MoveOrCopyWithSlack(*this, Other, 0, ExtraSlack);
		}

		/**
		 * @fn TArray& TArray::operator=(TArray&& Other)
		 *
		 * @brief 移动赋值
		 *
		 * @param [in] 源数组
		 *
		 * @returns 自己的引用
		 */
		TArray& operator=(TArray&& Other)
		{
			if (this != &Other)
			{
				DestructItems(GetData(), ArrayNum);
				MoveOrCopy(*this, Other, ArrayMax);
			}
			return *this;
		}

		/** 析构函数 */
		~TArray()
		{
			DestructItems(GetData(), ArrayNum);

			// 保证DebugGet实例化
#if defined(_MSC_VER) && !defined(__clang__)
			volatile const ElementType* Dummy = &DebugGet(0);
#endif
		}

		/**
		 * @fn FORCEINLINE ElementType* TArray::GetData()
		 *
		 * @brief 得到真实的数据指针
		 *
		 * @returns 指向数组的首地址，如果ArrayMax为0则为nullptr
		 */
		FORCEINLINE ElementType* GetData()
		{
			return (ElementType*)AllocatorInstance.GetAllocation();
		}

		/**
		 * @fn FORCEINLINE const ElementType* TArray::GetData() const
		 *
		 * @brief 得到真实的数据指针
		 *
		 * @returns 指向数组的首地址，如果ArrayMax为0则为nullptr
		 */
		FORCEINLINE const ElementType* GetData() const
		{
			return (const ElementType*)AllocatorInstance.GetAllocation();
		}

		/**
		 * @fn FORCEINLINE uint32 TArray::GetTypeSize() const
		 *
		 * @brief 得到类型的字节数大小
		 *
		 * @returns 类型的字节数
		 */
		FORCEINLINE uint32 GetTypeSize() const
		{
			return sizeof(ElementType);
		}

		/**
		 * @fn FORCEINLINE SIZE_T TArray::GetAllocatedSize(void) const
		 *
		 * @brief 得到分配器分配的实体堆内存大小
		 *
		 * @returns 分配器分配的实体堆内存大小
		 */
		FORCEINLINE SIZE_T GetAllocatedSize(void) const
		{
			return AllocatorInstance.GetAllocatedSize(ArrayMax, sizeof(ElementType));
		}

		/**
		 * @fn FORCEINLINE SizeType TArray::GetSlack() const
		 *
		 * @brief 得到剩余能够容纳的元素数量
		 *
		 * @returns 剩余能够容纳的元素数量
		 */
		FORCEINLINE SizeType GetSlack() const
		{
			return ArrayMax - ArrayNum;
		}

		/**
		 * @fn FORCEINLINE void TArray::CheckInvariants() const
		 *
		 * @brief 检查数组的有效性
		 */
		FORCEINLINE void CheckInvariants() const
		{
			check((ArrayNum >= 0) & (ArrayMax >= ArrayNum));
		}

		/**
		 * @fn FORCEINLINE void TArray::CheckAddress(const ElementType* Addr) const
		 *
		 * @brief 检查某个地址是否在范围内
		 *
		 * @param  地址
		 */
		FORCEINLINE void CheckAddress(const ElementType* Addr) const
		{
			checkf(Addr < GetData() || Addr >= (GetData() + ArrayMax), TEXT("Attempting to use a container element (%p) which already comes from the container being modified (%p, ArrayMax: %d, ArrayNum: %d, SizeofElement: %d)!"), Addr, GetData(), ArrayMax, ArrayNum, sizeof(ElementType));
		}
		
		/**
		 * @fn FORCEINLINE void TArray::RangeCheck(SizeType Index) const
		 *
		 * @brief 检查数组索引的有效性
		 *
		 * @param  数组索引
		 */
		FORCEINLINE void RangeCheck(SizeType Index) const
		{
			CheckInvariants();

			if constexpr (Allocator::RequireRangeCheck)
			{
				checkf((Index >= 0) & (Index < ArrayNum), TEXT("Array index out of bounds: %i from an array of size %i"), Index, ArrayNum); // & for one branch
			}
		}

		/**
		 * @fn FORCEINLINE bool TArray::IsValidIndex(SizeType Index) const
		 *
		 * @brief 查询Index是否是一个有效的索引
		 *
		 * @param  索引
		 *
		 * @returns 是否是一个有效的索引
		 */
		FORCEINLINE bool IsValidIndex(SizeType Index) const
		{
			return Index >= 0 && Index < ArrayNum;
		}

		/**
		 * @fn FORCEINLINE SizeType TArray::Num() const
		 *
		 * @brief 得到数组中元素的个数
		 *
		 * @returns 数组元素的个数
		 */
		FORCEINLINE SizeType Num() const
		{
			return ArrayNum;
		}

		/**
		 * @fn FORCEINLINE SizeType TArray::Max() const
		 *
		 * @brief 得到数组容量的上限
		 *
		 * @returns 数组容量上限
		 */
		FORCEINLINE SizeType Max() const
		{
			return ArrayMax;
		}

		/**
		 * @fn FORCEINLINE ElementType& TArray::operator[](SizeType Index)
		 *
		 * @brief Array indexer operator
		 *
		 * @param  索引
		 *
		 * @returns 对应Index的数据
		 */
		FORCEINLINE ElementType& operator[](SizeType Index)
		{
			RangeCheck(Index);
			return GetData()[Index];
		}

		/**
		 * @fn FORCEINLINE const ElementType& TArray::operator[](SizeType Index) const
		 *
		 * @brief Array indexer operator
		 *
		 * @param  索引
		 *
		 * @returns 对应Index的数据
		 */
		FORCEINLINE const ElementType& operator[](SizeType Index) const
		{
			RangeCheck(Index);
			return GetData()[Index];
		}

		/**
		 * @fn FORCEINLINE ElementType TArray::Pop(bool bAllowShrinking = true)
		 *
		 * @brief 从数组尾部弹出一个元素
		 *
		 * @param  是否允许收缩数组占用的空间
		 *
		 * @returns 弹出的元素
		 */
		FORCEINLINE ElementType Pop(bool bAllowShrinking = true)
		{
			RangeCheck(0);
			ElementType Result = MoveTempIfPossible(GetData()[ArrayNum - 1]);
			RemoveAt(ArrayNum - 1, 1, bAllowShrinking);
			return Result;
		}

		/**
		 * @fn FORCEINLINE void TArray::Push(ElementType&& Item)
		 *
		 * @brief 向数组末尾入栈
		 *
		 * @param [in] Item 入栈的元素
		 */
		FORCEINLINE void Push(ElementType&& Item)
		{
			Add(MoveTempIfPossible(Item));
		}

		/**
		 * @fn FORCEINLINE void TArray::Push(const ElementType& Item)
		 *
		 * @brief 向数组末尾入栈
		 *
		 * @param  Item 入栈的元素
		 */
		FORCEINLINE void Push(const ElementType& Item)
		{
			Add(Item);
		}

		/**
		 * @fn FORCEINLINE ElementType& TArray::Top()
		 *
		 * @brief 得到数组顶部元素
		 *
		 * @returns 数组顶部的元素
		 */
		FORCEINLINE ElementType& Top()
		{
			return Last();
		}

		/**
		 * @fn FORCEINLINE const ElementType& TArray::Top() const
		 *
		 * @brief 得到数组顶部元素
		 *
		 * @returns 数组顶部的元素
		 */
		FORCEINLINE const ElementType& Top() const
		{
			return Last();
		}

		/**
		 * @fn FORCEINLINE ElementType& TArray::Last(SizeType IndexFromTheEnd = 0)
		 *
		 * @brief 得到数组底部第N个元素
		 *
		 * @param  N
		 *
		 * @returns 距离数组底部N的元素
		 */
		FORCEINLINE ElementType& Last(SizeType IndexFromTheEnd = 0)
		{
			RangeCheck(ArrayNum - IndexFromTheEnd - 1);
			return GetData()[ArrayNum - IndexFromTheEnd - 1];
		}

		/**
		 * @fn FORCEINLINE const ElementType& TArray::Last(SizeType IndexFromTheEnd = 0) const
		 *
		 * @brief 得到距离数组底部第N个元素
		 *
		 * @param  IndexFromTheEnd N
		 *
		 * @returns 距离数组底部N的元素
		 */
		FORCEINLINE const ElementType& Last(SizeType IndexFromTheEnd = 0) const
		{
			RangeCheck(ArrayNum - IndexFromTheEnd - 1);
			return GetData()[ArrayNum - IndexFromTheEnd - 1];
		}

		/**
		 * @fn FORCEINLINE void TArray::Shrink()
		 *
		 * @brief 将数组收缩到当前的元素量
		 */
		FORCEINLINE void Shrink()
		{
			CheckInvariants();
			if (ArrayMax != ArrayNum)
			{
				ResizeTo(ArrayNum);
			}
		}

		/**
		 * @fn FORCEINLINE bool TArray::Find(const ElementType& Item, SizeType& Index) const
		 *
		 * @brief 查找指定元素
		 *
		 * @param 		   Item  指定的元素
		 * @param [in]	   Index 元素的索引，如果没找到为INDEX_NONE
		 *
		 * @returns 是否找到指定元素
		 */
		FORCEINLINE bool Find(const ElementType& Item, SizeType& Index) const
		{
			Index = this->Find(Item);
			return Index != INDEX_NONE;
		}

		/**
		 * @fn SizeType TArray::Find(const ElementType& Item) const
		 *
		 * @brief 查找指定元素
		 *
		 * @param  Item 指定的元素
		 *
		 * @returns 元素的索引，如果没找到为INDEX_NONE
		 */
		SizeType Find(const ElementType& Item) const
		{
			const ElementType* RESTRICT Start = GetData();
			for (const ElementType* RESTRICT Data = Start, *RESTRICT DataEnd = Data + ArrayNum; Data != DataEnd; ++Data)
			{
				if (*Data == Item)
				{
					return static_cast<SizeType>(Data - Start);
				}
			}
			return INDEX_NONE;
		}

		/**
		 * @fn FORCEINLINE bool TArray::FindLast(const ElementType& Item, SizeType& Index) const
		 *
		 * @brief 从尾部开始查找指定元素
		 *
		 * @param 		   Item  指定的元素
		 * @param [in,out] Index 元素的索引，如果没找到为INDEX_NONE
		 *
		 * @returns 是否找到指定元素
		 */
		FORCEINLINE bool FindLast(const ElementType& Item, SizeType& Index) const
		{
			Index = this->FindLast(Item);
			return Index != INDEX_NONE;
		}

		/**
		 * @fn SizeType TArray::FindLast(const ElementType& Item) const
		 *
		 * @brief 从尾部开始查找指定元素
		 *
		 * @param  Item 指定的元素
		 *
		 * @returns 元素的索引，如果没找到为INDEX_NONE
		 */
		SizeType FindLast(const ElementType& Item) const
		{
			for (const ElementType* RESTRICT Start = GetData(), *RESTRICT Data = Start + ArrayNum; Data != Start; )
			{
				--Data;
				if (*Data == Item)
				{
					return static_cast<SizeType>(Data - Start);
				}
			}
			return INDEX_NONE;
		}

		template <typename Predicate>
		SizeType FindLastByPredicate(Predicate Pred, SizeType Count) const
		{
			check(Count >= 0 && Count <= this->Num());
			for (const ElementType* RESTRICT Start = GetData(), *RESTRICT Data = Start + Count; Data != Start; )
			{
				--Data;
				if (Pred(*Data))
				{
					return static_cast<SizeType>(Data - Start);
				}
			}
			return INDEX_NONE;
		}
		
		template <typename Predicate>
		FORCEINLINE SizeType FindLastByPredicate(Predicate Pred) const
		{
			return FindLastByPredicate(Pred, ArrayNum);
		}
		
		template <typename KeyType>
		SizeType IndexOfByKey(const KeyType& Key) const
		{
			const ElementType* RESTRICT Start = GetData();
			for (const ElementType* RESTRICT Data = Start, *RESTRICT DataEnd = Start + ArrayNum; Data != DataEnd; ++Data)
			{
				if (*Data == Key)
				{
					return static_cast<SizeType>(Data - Start);
				}
			}
			return INDEX_NONE;
		}
		
		template <typename Predicate>
		SizeType IndexOfByPredicate(Predicate Pred) const
		{
			const ElementType* RESTRICT Start = GetData();
			for (const ElementType* RESTRICT Data = Start, *RESTRICT DataEnd = Start + ArrayNum; Data != DataEnd; ++Data)
			{
				if (Pred(*Data))
				{
					return static_cast<SizeType>(Data - Start);
				}
			}
			return INDEX_NONE;
		}

		template <typename KeyType>
		FORCEINLINE const ElementType* FindByKey(const KeyType& Key) const
		{
			return const_cast<TArray*>(this)->FindByKey(Key);
		}

		template <typename KeyType>
		ElementType* FindByKey(const KeyType& Key)
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
		FORCEINLINE const ElementType* FindByPredicate(Predicate Pred) const
		{
			return const_cast<TArray*>(this)->FindByPredicate(Pred);
		}

		template <typename Predicate>
		ElementType* FindByPredicate(Predicate Pred)
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
		TArray<ElementType> FilterByPredicate(Predicate Pred) const
		{
			TArray<ElementType> FilterResults;
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

		bool operator==(const TArray& OtherArray) const
		{
			SizeType Count = Num();

			return Count == OtherArray.Num() && CompareItems(GetData(), OtherArray.GetData(), Count);
		}

		FORCEINLINE bool operator!=(const TArray& OtherArray) const
		{
			return !(*this == OtherArray);
		}

		FORCEINLINE SizeType AddUninitialized(SizeType Count = 1)
		{
			CheckInvariants();
			check(Count >= 0);

			const SizeType OldNum = ArrayNum;
			if ((ArrayNum += Count) > ArrayMax)
			{
				ResizeGrow(OldNum);
			}
			return OldNum;
		}

	private:
		template <typename OtherSizeType>
		void InsertUninitializedImpl(SizeType Index, OtherSizeType Count)
		{
			CheckInvariants();
			check((Count >= 0) & (Index >= 0) & (Index <= ArrayNum));

			SizeType NewNum = Count;
			checkf((OtherSizeType)NewNum == Count, TEXT("Invalid number of elements to add to this array type: %llu"), (unsigned long long)NewNum);

			const SizeType OldNum = ArrayNum;
			if ((ArrayNum += Count) > ArrayMax)
			{
				ResizeGrow(OldNum);
			}
			ElementType* Data = GetData() + Index;
			RelocateConstructItems<ElementType>(Data + Count, Data, OldNum - Index);
		}

	public:
		FORCEINLINE void InsertUninitialized(SizeType Index, SizeType Count = 1)
		{
			InsertUninitializedImpl(Index, Count);
		}

		void InsertZeroed(SizeType Index, SizeType Count = 1)
		{
			InsertUninitializedImpl(Index, Count);
			Memzero(GetData() + Index, Count * sizeof(ElementType));
		}
		ElementType& InsertZeroed_GetRef(SizeType Index)
		{
			InsertUninitializedImpl(Index, 1);
			ElementType* Ptr = GetData() + Index;
			Memzero(Ptr, sizeof(ElementType));
			return *Ptr;
		}

		void InsertDefaulted(SizeType Index, SizeType Count = 1)
		{
			InsertUninitializedImpl(Index, Count);
			DefaultConstructItems<ElementType>(GetData() + Index, Count);
		}
		ElementType& InsertDefaulted_GetRef(SizeType Index)
		{
			InsertUninitializedImpl(Index, 1);
			ElementType* Ptr = GetData() + Index;
			DefaultConstructItems<ElementType>(Ptr, 1);
			return *Ptr;
		}

		SizeType Insert(std::initializer_list<ElementType> InitList, const SizeType InIndex)
		{
			SizeType NumNewElements = (SizeType)InitList.size();

			InsertUninitializedImpl(InIndex, NumNewElements);
			ConstructItems<ElementType>(GetData() + InIndex, InitList.begin(), NumNewElements);

			return InIndex;
		}
		template <typename OtherAllocator>
		SizeType Insert(const TArray<ElementType, OtherAllocator>& Items, const SizeType InIndex)
		{
			check((const void*)this != (const void*)&Items);

			auto NumNewElements = Items.Num();

			InsertUninitializedImpl(InIndex, NumNewElements);
			ConstructItems<ElementType>(GetData() + InIndex, Items.GetData(), NumNewElements);

			return InIndex;
		}
		template <typename OtherAllocator>
		SizeType Insert(TArray<ElementType, OtherAllocator>&& Items, const SizeType InIndex)
		{
			check((const void*)this != (const void*)&Items);

			auto NumNewElements = Items.Num();

			InsertUninitializedImpl(InIndex, NumNewElements);
			RelocateConstructItems<ElementType>(GetData() + InIndex, Items.GetData(), NumNewElements);
			Items.ArrayNum = 0;

			return InIndex;
		}
		SizeType Insert(const ElementType* Ptr, SizeType Count, SizeType Index)
		{
			check(Ptr != nullptr);

			InsertUninitializedImpl(Index, Count);
			ConstructItems<ElementType>(GetData() + Index, Ptr, Count);

			return Index;
		}
		SizeType Insert(ElementType&& Item, SizeType Index)
		{
			CheckAddress(&Item);
			InsertUninitializedImpl(Index, 1);
			new(GetData() + Index) ElementType(MoveTempIfPossible(Item));
			return Index;
		}
		SizeType Insert(const ElementType& Item, SizeType Index)
		{
			CheckAddress(&Item);
			InsertUninitializedImpl(Index, 1);
			new(GetData() + Index) ElementType(Item);
			return Index;
		}

		ElementType& Insert_GetRef(ElementType&& Item, SizeType Index)
		{
			CheckAddress(&Item);

			// construct a copy in place at Index (this new operator will insert at 
			// Index, then construct that memory with Item)
			InsertUninitializedImpl(Index, 1);
			ElementType* Ptr = GetData() + Index;
			new(Ptr) ElementType(MoveTempIfPossible(Item));
			return *Ptr;
		}
		ElementType& Insert_GetRef(const ElementType& Item, SizeType Index)
		{
			CheckAddress(&Item);

			// construct a copy in place at Index (this new operator will insert at 
			// Index, then construct that memory with Item)
			InsertUninitializedImpl(Index, 1);
			ElementType* Ptr = GetData() + Index;
			new(Ptr) ElementType(Item);
			return *Ptr;
		}
	private:
		void RemoveAtImpl(SizeType Index, SizeType Count, bool bAllowShrinking)
		{
			if (Count)
			{
				CheckInvariants();
				check((Count >= 0) & (Index >= 0) & (Index + Count <= ArrayNum));

				DestructItems(GetData() + Index, Count);

				SizeType NumToMove = ArrayNum - Index - Count;
				if (NumToMove)
				{
					Memmove
					(
						(uint8*)AllocatorInstance.GetAllocation() + (Index) * sizeof(ElementType),
						(uint8*)AllocatorInstance.GetAllocation() + (Index + Count) * sizeof(ElementType),
						NumToMove * sizeof(ElementType)
					);
				}
				ArrayNum -= Count;

				if (bAllowShrinking)
				{
					ResizeShrink();
				}
			}
		}

	public:
		FORCEINLINE void RemoveAt(SizeType Index)
		{
			RemoveAtImpl(Index, 1, true);
		}

		template <typename CountType>
		FORCEINLINE void RemoveAt(SizeType Index, CountType Count, bool bAllowShrinking = true)
		{
			static_assert(!TAreTypesEqual_v<CountType, bool>, "TArray::RemoveAt: unexpected bool passed as the Count argument");
			RemoveAtImpl(Index, (SizeType)Count, bAllowShrinking);
		}

	private:
		void RemoveAtSwapImpl(SizeType Index, SizeType Count = 1, bool bAllowShrinking = true)
		{
			if (Count)
			{
				CheckInvariants();
				check((Count >= 0) & (Index >= 0) & (Index + Count <= ArrayNum));

				DestructItems(GetData() + Index, Count);

				const SizeType NumElementsInHole = Count;
				const SizeType NumElementsAfterHole = ArrayNum - (Index + Count);
				const SizeType NumElementsToMoveIntoHole = FMath::Min(NumElementsInHole, NumElementsAfterHole);
				if (NumElementsToMoveIntoHole)
				{
					Memcpy(
						(uint8*)AllocatorInstance.GetAllocation() + (Index) * sizeof(ElementType),
						(uint8*)AllocatorInstance.GetAllocation() + (ArrayNum - NumElementsToMoveIntoHole) * sizeof(ElementType),
						NumElementsToMoveIntoHole * sizeof(ElementType)
					);
				}
				ArrayNum -= Count;

				if (bAllowShrinking)
				{
					ResizeShrink();
				}
			}
		}

	public:
		
		FORCEINLINE void RemoveAtSwap(SizeType Index)
		{
			RemoveAtSwapImpl(Index, 1, true);
		}

		template <typename CountType>
		FORCEINLINE void RemoveAtSwap(SizeType Index, CountType Count, bool bAllowShrinking = true)
		{
			static_assert(!TAreTypesEqual<CountType, bool>::Value, "TArray::RemoveAtSwap: unexpected bool passed as the Count argument");
			RemoveAtSwapImpl(Index, Count, bAllowShrinking);
		}

		void Reset(SizeType NewSize = 0)
		{
			if (NewSize <= ArrayMax)
			{
				DestructItems(GetData(), ArrayNum);
				ArrayNum = 0;
			}
			else
			{
				Empty(NewSize);
			}
		}

		void Empty(SizeType Slack = 0)
		{
			DestructItems(GetData(), ArrayNum);

			check(Slack >= 0);
			ArrayNum = 0;

			if (ArrayMax != Slack)
			{
				ResizeTo(Slack);
			}
		}

		void SetNum(SizeType NewNum, bool bAllowShrinking = true)
		{
			if (NewNum > Num())
			{
				const SizeType Diff = NewNum - ArrayNum;
				const SizeType Index = AddUninitialized(Diff);
				DefaultConstructItems<ElementType>((uint8*)AllocatorInstance.GetAllocation() + Index * sizeof(ElementType), Diff);
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

		void SetNumUnsafeInternal(SizeType NewNum)
		{
			check(NewNum <= Num() && NewNum >= 0);
			ArrayNum = NewNum;
		}

		template <typename OtherElementType, typename OtherAllocator>
		void Append(const TArray<OtherElementType, OtherAllocator>& Source)
		{
			check((void*)this != (void*)&Source);

			SizeType SourceCount = Source.Num();

			// Do nothing if the source is empty.
			if (!SourceCount)
			{
				return;
			}

			// Allocate memory for the new elements.
			Reserve(ArrayNum + SourceCount);
			ConstructItems<ElementType>(GetData() + ArrayNum, Source.GetData(), SourceCount);

			ArrayNum += SourceCount;
		}

		template <typename OtherElementType, typename OtherAllocator>
		void Append(TArray<OtherElementType, OtherAllocator>&& Source)
		{
			check((void*)this != (void*)&Source);

			SizeType SourceCount = Source.Num();

			// Do nothing if the source is empty.
			if (!SourceCount)
			{
				return;
			}

			// Allocate memory for the new elements.
			Reserve(ArrayNum + SourceCount);
			RelocateConstructItems<ElementType>(GetData() + ArrayNum, Source.GetData(), SourceCount);
			Source.ArrayNum = 0;

			ArrayNum += SourceCount;
		}

		void Append(const ElementType* Ptr, SizeType Count)
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

		TArray& operator+=(TArray&& Other)
		{
			Append(MoveTemp(Other));
			return *this;
		}

		TArray& operator+=(const TArray& Other)
		{
			Append(Other);
			return *this;
		}

		TArray& operator+=(std::initializer_list<ElementType> InitList)
		{
			Append(InitList);
			return *this;
		}

		template <typename... ArgsType>
		FORCEINLINE SizeType Emplace(ArgsType&&... Args)
		{
			const SizeType Index = AddUninitialized(1);
			new(GetData() + Index) ElementType(Forward<ArgsType>(Args)...);
			return Index;
		}

		template <typename... ArgsType>
		FORCEINLINE ElementType& Emplace_GetRef(ArgsType&&... Args)
		{
			const SizeType Index = AddUninitialized(1);
			ElementType* Ptr = GetData() + Index;
			new(Ptr) ElementType(Forward<ArgsType>(Args)...);
			return *Ptr;
		}

		template <typename... ArgsType>
		FORCEINLINE void EmplaceAt(SizeType Index, ArgsType&&... Args)
		{
			InsertUninitializedImpl(Index, 1);
			new(GetData() + Index) ElementType(Forward<ArgsType>(Args)...);
		}

		template <typename... ArgsType>
		FORCEINLINE ElementType& EmplaceAt_GetRef(SizeType Index, ArgsType&&... Args)
		{
			InsertUninitializedImpl(Index, 1);
			ElementType* Ptr = GetData() + Index;
			new(Ptr) ElementType(Forward<ArgsType>(Args)...);
			return *Ptr;
		}

		FORCEINLINE SizeType Add(ElementType&& Item)
		{
			CheckAddress(&Item);
			return Emplace(MoveTempIfPossible(Item));
		}

		FORCEINLINE SizeType Add(const ElementType& Item)
		{
			CheckAddress(&Item);
			return Emplace(Item);
		}

		FORCEINLINE ElementType& Add_GetRef(ElementType&& Item)
		{
			CheckAddress(&Item);
			return Emplace_GetRef(MoveTempIfPossible(Item));
		}

		FORCEINLINE ElementType& Add_GetRef(const ElementType& Item)
		{
			CheckAddress(&Item);
			return Emplace_GetRef(Item);
		}

		SizeType AddZeroed(SizeType Count = 1)
		{
			const SizeType Index = AddUninitialized(Count);
			Memzero((uint8*)AllocatorInstance.GetAllocation() + Index * sizeof(ElementType), Count * sizeof(ElementType));
			return Index;
		}

		ElementType& AddZeroed_GetRef()
		{
			const SizeType Index = AddUninitialized(1);
			ElementType* Ptr = GetData() + Index;
			Memzero(Ptr, sizeof(ElementType));
			return *Ptr;
		}

		SizeType AddDefaulted(SizeType Count = 1)
		{
			const SizeType Index = AddUninitialized(Count);
			DefaultConstructItems<ElementType>((uint8*)AllocatorInstance.GetAllocation() + Index * sizeof(ElementType), Count);
			return Index;
		}

		ElementType& AddDefaulted_GetRef()
		{
			const SizeType Index = AddUninitialized(1);
			ElementType* Ptr = GetData() + Index;
			DefaultConstructItems<ElementType>(Ptr, 1);
			return *Ptr;
		}

	private:

		template <typename ArgsType>
		SizeType AddUniqueImpl(ArgsType&& Args)
		{
			SizeType Index;
			if (Find(Args, Index))
			{
				return Index;
			}

			return Add(Forward<ArgsType>(Args));
		}

	public:

		FORCEINLINE SizeType AddUnique(ElementType&& Item) { return AddUniqueImpl(MoveTempIfPossible(Item)); }

		FORCEINLINE SizeType AddUnique(const ElementType& Item) { return AddUniqueImpl(Item); }

		FORCEINLINE void Reserve(SizeType Number)
		{
			checkSlow(Number >= 0);
			if (Number > ArrayMax)
			{
				ResizeTo(Number);
			}
		}

		void Init(const ElementType& Element, SizeType Number)
		{
			Empty(Number);
			for (SizeType Index = 0; Index < Number; ++Index)
			{
				new(*this) ElementType(Element);
			}
		}

		SizeType RemoveSingle(const ElementType& Item)
		{
			SizeType Index = Find(Item);
			if (Index == INDEX_NONE)
			{
				return 0;
			}

			auto* RemovePtr = GetData() + Index;

			// Destruct items that match the specified Item.
			DestructItems(RemovePtr, 1);
			const SizeType NextIndex = Index + 1;
			RelocateConstructItems<ElementType>(RemovePtr, RemovePtr + 1, ArrayNum - (Index + 1));

			// Update the array count
			--ArrayNum;

			// Removed one item
			return 1;
		}

		SizeType Remove(const ElementType& Item)
		{
			CheckAddress(&Item);

			// Element is non-const to preserve compatibility with existing code with a non-const operator==() member function
			return RemoveAll([&Item](ElementType& Element) { return Element == Item; });
		}

		template <class PREDICATE_CLASS>
		SizeType RemoveAll(const PREDICATE_CLASS& Predicate)
		{
			const SizeType OriginalNum = ArrayNum;
			if (!OriginalNum)
			{
				return 0; // nothing to do, loop assumes one item so need to deal with this edge case here
			}

			SizeType WriteIndex = 0;
			SizeType ReadIndex = 0;
			bool NotMatch = !Predicate(GetData()[ReadIndex]); // use a ! to guarantee it can't be anything other than zero or one
			do
			{
				SizeType RunStartIndex = ReadIndex++;
				while (ReadIndex < OriginalNum && NotMatch == !Predicate(GetData()[ReadIndex]))
				{
					ReadIndex++;
				}
				SizeType RunLength = ReadIndex - RunStartIndex;
				checkSlow(RunLength > 0);
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

			ArrayNum = WriteIndex;
			return OriginalNum - ArrayNum;
		}

		template <class PREDICATE_CLASS>
		void RemoveAllSwap(const PREDICATE_CLASS& Predicate, bool bAllowShrinking = true)
		{
			for (SizeType ItemIndex = 0; ItemIndex < Num();)
			{
				if (Predicate((*this)[ItemIndex]))
				{
					RemoveAtSwap(ItemIndex, 1, bAllowShrinking);
				}
				else
				{
					++ItemIndex;
				}
			}
		}

		SizeType RemoveSingleSwap(const ElementType& Item, bool bAllowShrinking = true)
		{
			SizeType Index = Find(Item);
			if (Index == INDEX_NONE)
			{
				return 0;
			}

			RemoveAtSwap(Index, 1, bAllowShrinking);

			// Removed one item
			return 1;
		}

		SizeType RemoveSwap(const ElementType& Item)
		{
			CheckAddress(&Item);

			const SizeType OriginalNum = ArrayNum;
			for (SizeType Index = 0; Index < ArrayNum; Index++)
			{
				if ((*this)[Index] == Item)
				{
					RemoveAtSwap(Index--);
				}
			}
			return OriginalNum - ArrayNum;
		}

		FORCEINLINE void SwapMemory(SizeType FirstIndexToSwap, SizeType SecondIndexToSwap)
		{
			Memswap(
				(uint8*)AllocatorInstance.GetAllocation() + (sizeof(ElementType)*FirstIndexToSwap),
				(uint8*)AllocatorInstance.GetAllocation() + (sizeof(ElementType)*SecondIndexToSwap),
				sizeof(ElementType)
			);
		}

		FORCEINLINE void Swap(SizeType FirstIndexToSwap, SizeType SecondIndexToSwap)
		{
			check((FirstIndexToSwap >= 0) && (SecondIndexToSwap >= 0));
			check((ArrayNum > FirstIndexToSwap) && (ArrayNum > SecondIndexToSwap));
			if (FirstIndexToSwap != SecondIndexToSwap)
			{
				SwapMemory(FirstIndexToSwap, SecondIndexToSwap);
			}
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

#if TARRAY_RANGED_FOR_CHECKS
		typedef TCheckedPointerIterator<      ElementType, SizeType> RangedForIteratorType;
		typedef TCheckedPointerIterator<const ElementType, SizeType> RangedForConstIteratorType;
#else
		typedef       ElementType* RangedForIteratorType;
		typedef const ElementType* RangedForConstIteratorType;
#endif

	public:

#if TARRAY_RANGED_FOR_CHECKS
		FORCEINLINE RangedForIteratorType      begin() { return RangedForIteratorType(ArrayNum, GetData()); }
		FORCEINLINE RangedForConstIteratorType begin() const { return RangedForConstIteratorType(ArrayNum, GetData()); }
		FORCEINLINE RangedForIteratorType      end() { return RangedForIteratorType(ArrayNum, GetData() + Num()); }
		FORCEINLINE RangedForConstIteratorType end() const { return RangedForConstIteratorType(ArrayNum, GetData() + Num()); }
#else
		FORCEINLINE RangedForIteratorType      begin() { return GetData(); }
		FORCEINLINE RangedForConstIteratorType begin() const { return GetData(); }
		FORCEINLINE RangedForIteratorType      end() { return GetData() + Num(); }
		FORCEINLINE RangedForConstIteratorType end() const { return GetData() + Num(); }
#endif

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

#if defined(_MSC_VER) && !defined(__clang__)
	private:
		FORCENOINLINE const ElementType& DebugGet(SizeType Index) const
		{
			return GetData()[Index];
		}
#endif

	private:

		FORCENOINLINE void ResizeGrow(SizeType OldNum)
		{
			ArrayMax = AllocatorInstance.CalculateSlackGrow(ArrayNum, ArrayMax, sizeof(ElementType));
			AllocatorInstance.ResizeAllocation(OldNum, ArrayMax, sizeof(ElementType));
		}
		FORCENOINLINE void ResizeShrink()
		{
			const SizeType NewArrayMax = AllocatorInstance.CalculateSlackShrink(ArrayNum, ArrayMax, sizeof(ElementType));
			if (NewArrayMax != ArrayMax)
			{
				ArrayMax = NewArrayMax;
				check(ArrayMax >= ArrayNum);
				AllocatorInstance.ResizeAllocation(ArrayNum, ArrayMax, sizeof(ElementType));
			}
		}
		FORCENOINLINE void ResizeTo(SizeType NewMax)
		{
			if (NewMax)
			{
				NewMax = AllocatorInstance.CalculateSlackReserve(NewMax, sizeof(ElementType));
			}
			if (NewMax != ArrayMax)
			{
				ArrayMax = NewMax;
				AllocatorInstance.ResizeAllocation(ArrayNum, ArrayMax, sizeof(ElementType));
			}
		}
		FORCENOINLINE void ResizeForCopy(SizeType NewMax, SizeType PrevMax)
		{
			if (NewMax)
			{
				NewMax = AllocatorInstance.CalculateSlackReserve(NewMax, sizeof(ElementType));
			}
			if (NewMax > PrevMax)
			{
				AllocatorInstance.ResizeAllocation(0, NewMax, sizeof(ElementType));
				ArrayMax = NewMax;
			}
			else
			{
				ArrayMax = PrevMax;
			}
		}

		/**
		 * @fn template <typename OtherElementType, typename OtherSizeType> void TArray::CopyToEmpty(const OtherElementType* OtherData, OtherSizeType OtherNum, SizeType PrevMax, SizeType ExtraSlack)
		 *
		 * @brief 将对方数组的内容拷贝给自己
		 *
		 * @param  OtherData  源数组首地址
		 * @param  OtherNum   源数组长度
		 * @param  PrevMax    之前分配的大小
		 * @param  ExtraSlack 预留的空间
		 */
		template <typename OtherElementType, typename OtherSizeType>
		void CopyToEmpty(const OtherElementType* OtherData, OtherSizeType OtherNum, SizeType PrevMax, SizeType ExtraSlack)
		{
			SizeType NewNum = (SizeType)OtherNum;
			checkf((OtherSizeType)NewNum == OtherNum, TEXT("Invalid number of elements to add to this array type: %llu"), (unsigned long long)NewNum);

			check(ExtraSlack >= 0);
			ArrayNum = NewNum;
			if (OtherNum || ExtraSlack || PrevMax)
			{
				ResizeForCopy(NewNum + ExtraSlack, PrevMax);
				ConstructItems<ElementType>(GetData(), OtherData, OtherNum);
			}
			else
			{
				ArrayMax = AllocatorInstance.GetInitialCapacity();
			}
		}

	protected:
		ElementAllocatorType AllocatorInstance;
		SizeType             ArrayNum;
		SizeType             ArrayMax;

		//-------------------------------------------实现堆-------------------------------------------
	public:

		template <class PREDICATE_CLASS>
		FORCEINLINE void Heapify(const PREDICATE_CLASS& Predicate)
		{
			TDereferenceWrapper<ElementType, PREDICATE_CLASS> PredicateWrapper(Predicate);
			::Fuko::Algo::Heapify(*this, PredicateWrapper);
		}

		void Heapify()
		{
			Heapify(TLess<ElementType>());
		}

		template <class PREDICATE_CLASS>
		SizeType HeapPush(ElementType&& InItem, const PREDICATE_CLASS& Predicate)
		{
			// 在尾部添加元素，然后向上检索堆
			Add(MoveTempIfPossible(InItem));
			TDereferenceWrapper<ElementType, PREDICATE_CLASS> PredicateWrapper(Predicate);
			SizeType Result = ::Fuko::Algo::Impl::HeapSiftUp(GetData(), 0, Num() - 1, FIdentityFunctor(), PredicateWrapper);

			return Result;
		}

		template <class PREDICATE_CLASS>
		SizeType HeapPush(const ElementType& InItem, const PREDICATE_CLASS& Predicate)
		{
			// 在尾部添加元素，然后向上检索堆
			Add(InItem);
			TDereferenceWrapper<ElementType, PREDICATE_CLASS> PredicateWrapper(Predicate);
			SizeType Result = ::Fuko::Algo::Impl::HeapSiftUp(GetData(), 0, Num() - 1, FIdentityFunctor(), PredicateWrapper);

			return Result;
		}

		SizeType HeapPush(ElementType&& InItem)
		{
			return HeapPush(MoveTempIfPossible(InItem), TLess<ElementType>());
		}

		SizeType HeapPush(const ElementType& InItem)
		{
			return HeapPush(InItem, TLess<ElementType>());
		}

		template <class PREDICATE_CLASS>
		void HeapPop(ElementType& OutItem, const PREDICATE_CLASS& Predicate, bool bAllowShrinking = true)
		{
			OutItem = MoveTemp((*this)[0]);
			RemoveAtSwap(0, 1, bAllowShrinking);

			TDereferenceWrapper< ElementType, PREDICATE_CLASS> PredicateWrapper(Predicate);
			::Fuko::Algo::Impl::HeapSiftDown(GetData(), 0, Num(), FIdentityFunctor(), PredicateWrapper);
		}

		void HeapPop(ElementType& OutItem, bool bAllowShrinking = true)
		{
			HeapPop(OutItem, TLess<ElementType>(), bAllowShrinking);
		}

		template <class PREDICATE_CLASS>
		void VerifyHeap(const PREDICATE_CLASS& Predicate)
		{
			check(Algo::IsHeap(*this, Predicate));
		}

		template <class PREDICATE_CLASS>
		void HeapPopDiscard(const PREDICATE_CLASS& Predicate, bool bAllowShrinking = true)
		{
			RemoveAtSwap(0, 1, bAllowShrinking);
			TDereferenceWrapper< ElementType, PREDICATE_CLASS> PredicateWrapper(Predicate);
			::Fuko::Algo::Impl::HeapSiftDown(GetData(), 0, Num(), FIdentityFunctor(), PredicateWrapper);
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

		template <class PREDICATE_CLASS>
		void HeapRemoveAt(SizeType Index, const PREDICATE_CLASS& Predicate, bool bAllowShrinking = true)
		{
			RemoveAtSwap(Index, 1, bAllowShrinking);

			TDereferenceWrapper< ElementType, PREDICATE_CLASS> PredicateWrapper(Predicate);
			::Fuko::Algo::Impl::HeapSiftDown(GetData(), Index, Num(), FIdentityFunctor(), PredicateWrapper);
			::Fuko::Algo::Impl::HeapSiftUp(GetData(), 0, FMath::Min(Index, Num() - 1), FIdentityFunctor(), PredicateWrapper);
		}

		void HeapRemoveAt(SizeType Index, bool bAllowShrinking = true)
		{
			HeapRemoveAt(Index, TLess< ElementType >(), bAllowShrinking);
		}

		template <class PREDICATE_CLASS>
		void HeapSort(const PREDICATE_CLASS& Predicate)
		{
			TDereferenceWrapper<ElementType, PREDICATE_CLASS> PredicateWrapper(Predicate);
			::Fuko::Algo::HeapSort(*this, PredicateWrapper);
		}

		void HeapSort()
		{
			HeapSort(TLess<ElementType>());
		}

		const ElementAllocatorType& GetAllocatorInstance() const { return AllocatorInstance; }
		ElementAllocatorType& GetAllocatorInstance() { return AllocatorInstance; }
	};

}

// TypeTraits
namespace Fuko
{
	template <typename InElementType, typename Allocator>
	struct TIsZeroConstructType<TArray<InElementType, Allocator>>
	{
		enum { Value = TAllocatorTraits<Allocator>::IsZeroConstruct };
	};

	template <typename InElementType, typename Allocator>
	struct TContainerTraits<TArray<InElementType, Allocator> > : public TContainerTraitsBase<TArray<InElementType, Allocator> >
	{
		static_assert(TAllocatorTraits<Allocator>::SupportsMove, "TArray no longer supports move-unaware allocators");
		enum { MoveWillEmptyContainer = TAllocatorTraits<Allocator>::SupportsMove };
	};

	template <typename T, typename Allocator>
	struct TIsContiguousContainer<TArray<T, Allocator>>
	{
		enum { Value = true };
	};
	template <typename T> struct TIsTArray { enum { Value = false }; };

	template <typename InElementType, typename Allocator> struct TIsTArray<               TArray<InElementType, Allocator>> { enum { Value = true }; };
	template <typename InElementType, typename Allocator> struct TIsTArray<const          TArray<InElementType, Allocator>> { enum { Value = true }; };
	template <typename InElementType, typename Allocator> struct TIsTArray<      volatile TArray<InElementType, Allocator>> { enum { Value = true }; };
	template <typename InElementType, typename Allocator> struct TIsTArray<const volatile TArray<InElementType, Allocator>> { enum { Value = true }; };
}

// Operater new
template <typename T, typename Allocator> void* operator new(size_t Size, Fuko::TArray<T, Allocator>& Array)
{
	check(Size == sizeof(T));
	const auto Index = Array.AddUninitialized(1);
	return &Array[Index];
}
template <typename T, typename Allocator> void* operator new(size_t Size, Fuko::TArray<T, Allocator>& Array, typename Fuko::TArray<T, Allocator>::SizeType Index)
{
	check(Size == sizeof(T));
	Array.InsertUninitialized(Index);
	return &Array[Index];
}
