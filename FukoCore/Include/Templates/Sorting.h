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


