#pragma once

/**
 * @struct TDereferenceWrapper
 *
 * @brief 在调用谓语时解除指针
 *
 * @tparam T			   谓语参数类型
 * @tparam PREDICATE_CLASS 谓语类型
 */
template<typename T, class PREDICATE_CLASS>
struct TDereferenceWrapper
{
	const PREDICATE_CLASS& Predicate;

	TDereferenceWrapper(const PREDICATE_CLASS& InPredicate)
		: Predicate(InPredicate) {}

	FORCEINLINE bool operator()(T& A, T& B) { return Predicate(A, B); }
	FORCEINLINE bool operator()(const T& A, const T& B) const { return Predicate(A, B); }
};
template<typename T, class PREDICATE_CLASS>
struct TDereferenceWrapper<T*, PREDICATE_CLASS>
{
	const PREDICATE_CLASS& Predicate;

	TDereferenceWrapper(const PREDICATE_CLASS& InPredicate)
		: Predicate(InPredicate) {}

	FORCEINLINE bool operator()(T* A, T* B) const
	{
		return Predicate(*A, *B);
	}
};

/**
 * @struct TArrayRange
 *
 * @brief 提供调用算法的包装
 *
 */
template <typename T>
struct TArrayRange
{
	TArrayRange(T* InPtr, int32 InSize)
		: Begin(InPtr)
		, Size(InSize)
	{
	}

	T* GetData() const { return Begin; }
	int32 Num() const { return Size; }

private:
	T* Begin;
	int32 Size;
};
template <typename T>
struct TIsContiguousContainer< TArrayRange<T> >
{
	enum { Value = true };
};

// 排序
template<class T, class PREDICATE_CLASS>
void Sort(T* First, const int32 Num, const PREDICATE_CLASS& Predicate)
{
	TArrayRange<T> ArrayRange(First, Num);
	::Fuko::Algo::Sort(ArrayRange, TDereferenceWrapper<T, PREDICATE_CLASS>(Predicate));
}
template<class T, class PREDICATE_CLASS>
void Sort(T** First, const int32 Num, const PREDICATE_CLASS& Predicate)
{
	TArrayRange<T*> ArrayRange(First, Num);
	::Fuko::Algo::Sort(ArrayRange, TDereferenceWrapper<T*, PREDICATE_CLASS>(Predicate));
}

template<class T>
void Sort(T* First, const int32 Num)
{
	TArrayRange<T> ArrayRange(First, Num);
	::Fuko::Algo::Sort(ArrayRange, TDereferenceWrapper<T, TLess<T> >(TLess<T>()));
}

template<class T>
void Sort(T** First, const int32 Num)
{
	TArrayRange<T*> ArrayRange(First, Num);
	::Fuko::Algo::Sort(ArrayRange, TDereferenceWrapper<T*, TLess<T> >(TLess<T>()));
}

// 最大公约数计算算法 
class FEuclidDivisionGCD
{
public:
	// 辗转相除法求最大公约数 
	static int32 GCD(int32 A, int32 B)
	{
		while (B != 0)
		{
			int32 Temp = B;
			B = A % B;
			A = Temp;
		}

		return A;
	}
};

// 合并策略中用于旋转的部分
template <class TGCDPolicy>
class TJugglingRotation
{
public:
	template <class T>
	static void Rotate(T* First, const int32 From, const int32 To, const int32 Amount)
	{
		if (Amount == 0)
		{
			return;
		}

		auto Num = To - From;
		auto GCD = TGCDPolicy::GCD(Num, Amount);
		auto CycleSize = Num / GCD;

		for (int32 Index = 0; Index < GCD; ++Index)
		{
			T BufferObject = MoveTemp(First[From + Index]);
			int32 IndexToFill = Index;

			for (int32 InCycleIndex = 0; InCycleIndex < CycleSize; ++InCycleIndex)
			{
				IndexToFill = (IndexToFill + Amount) % Num;
				Swap(First[From + IndexToFill], BufferObject);
			}
		}
	}
};
// 归并排序的合并策略
template <class TRotationPolicy>
class TRotationInPlaceMerge
{
public:
	template <class T, class PREDICATE_CLASS>
	static void Merge(T* First, const int32 Mid, const int32 Num, const PREDICATE_CLASS& Predicate)
	{
		int32 AStart = 0;
		int32 BStart = Mid;

		while (AStart < BStart && BStart < Num)
		{
			// Index after the last value == First[BStart]
			int32 NewAOffset = (int32)::Fuko::Impl::UpperBoundInternal(First + AStart, BStart - AStart, First[BStart], FIdentityFunctor(), Predicate);
			AStart += NewAOffset;

			if (AStart >= BStart) // done
				break;

			// Index of the first value == First[AStart]
			int32 NewBOffset = (int32)::Fuko::Impl::LowerBoundInternal(First + BStart, Num - BStart, First[AStart], FIdentityFunctor(), Predicate);
			TRotationPolicy::Rotate(First, AStart, BStart + NewBOffset, NewBOffset);
			BStart += NewBOffset;
			AStart += NewBOffset + 1;
		}
	}
};

// 归并排序模板
template <class TMergePolicy, int32 MinMergeSubgroupSize = 2>
class TMergeSort
{
public:
	template<class T, class PREDICATE_CLASS>
	static void Sort(T* First, const int32 Num, const PREDICATE_CLASS& Predicate)
	{
		int32 SubgroupStart = 0;

		if constexpr (MinMergeSubgroupSize > 1)
		{
			if constexpr (MinMergeSubgroupSize > 2)
			{
				// First pass with simple bubble-sort.
				do
				{
					int32 GroupEnd = FMath::Min(SubgroupStart + MinMergeSubgroupSize, Num);
					do
					{
						for (int32 It = SubgroupStart; It < GroupEnd - 1; ++It)
						{
							if (Predicate(First[It + 1], First[It]))
							{
								Exchange(First[It], First[It + 1]);
							}
						}
						GroupEnd--;
					} while (GroupEnd - SubgroupStart > 1);

					SubgroupStart += MinMergeSubgroupSize;
				} while (SubgroupStart < Num);
			}
			else
			{
				for (int32 Subgroup = 0; Subgroup < Num; Subgroup += 2)
				{
					if (Subgroup + 1 < Num && Predicate(First[Subgroup + 1], First[Subgroup]))
					{
						Swap(First[Subgroup], First[Subgroup + 1]);
					}
				}
			}
		}

		int32 SubgroupSize = MinMergeSubgroupSize;
		while (SubgroupSize < Num)
		{
			SubgroupStart = 0;
			do
			{
				TMergePolicy::Merge(
					First + SubgroupStart,
					SubgroupSize,
					FMath::Min(SubgroupSize << 1, Num - SubgroupStart),
					Predicate);
				SubgroupStart += SubgroupSize << 1;
			} while (SubgroupStart < Num);

			SubgroupSize <<= 1;
		}
	}
};

// 归并排序实现
template<class T, class PREDICATE_CLASS>
void StableSortInternal(T* First, const int32 Num, const PREDICATE_CLASS& Predicate)
{
	TMergeSort<TRotationInPlaceMerge<TJugglingRotation<FEuclidDivisionGCD> > >::Sort(First, Num, Predicate);
}

template<class T, class PREDICATE_CLASS>
void StableSort(T* First, const int32 Num, const PREDICATE_CLASS& Predicate)
{
	StableSortInternal(First, Num, TDereferenceWrapper<T, PREDICATE_CLASS>(Predicate));
}