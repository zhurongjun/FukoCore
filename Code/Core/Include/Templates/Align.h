#pragma once
#include <inttypes.h>
#include "CoreConfig.h"
#include "TypeTraits.h"

// 向上对齐, Alignment必须是2的次方
template <typename T>
FORCEINLINE constexpr T Align(T Val, uint64_t Alignment)
{
	static_assert(std::is_integral_v<T> || std::is_pointer_v<T>, "Align expects an integer or pointer type");

	return (T)(((uint64_t)Val + Alignment - 1) & ~(Alignment - 1));
}

// 向下对齐, Alignment必须是2的次方
template <typename T>
FORCEINLINE constexpr T AlignDown(T Val, uint64_t Alignment)
{
	static_assert(std::is_integral_v<T> || std::is_pointer_v<T>, "AlignDown expects an integer or pointer type");

	return (T)(((uint64_t)Val) & ~(Alignment - 1));
}

// 是否是对齐的, Alignment必须是2的次方
template <typename T>
FORCEINLINE constexpr bool IsAligned(T Val, uint64_t Alignment)
{
	static_assert(std::is_integral_v<T> || std::is_pointer_v<T>, "IsAligned expects an integer or pointer type");

	return !((uint64_t)Val & (Alignment - 1));
}

// 任意对齐, Alignment可以是任意值
template <typename T>
FORCEINLINE constexpr T AlignArbitrary(T Val, uint64_t Alignment)
{
	static_assert(std::is_integral_v<T> || std::is_pointer_v<T>, "AlignArbitrary expects an integer or pointer type");

	return (T)((((uint64_t)Val + Alignment - 1) / Alignment) * Alignment);
}

// 对齐的空间占位符
template<int32_t Size, int32_t Alignment>
struct TAlignedBytes
{
	static_assert(Alignment == 1 || Alignment == 2 || Alignment == 4 || Alignment == 8 || Alignment == 16, "Don't use invalid Alignment");
	struct alignas(Alignment) PlaceHolder
	{
		uint8_t Pad[Size];
	};
	PlaceHolder Data;
};

// Element占位符
template<typename T, int N = 1>
struct TStorage : public TAlignedBytes<sizeof(T) * N, alignof(T)> {};

// 手动构造析构的Warpper 
template<typename T, bool AutoDestory = false>
class TLazyObject;

template<typename T>
class TLazyObject<T, false>
{
	TStorage<T>		m_Storage;
public:
	TLazyObject() = default;
	
	template<typename...Ts>
	void New(Ts&&...Args)
	{
		new(&m_Storage)T(std::forward<Ts>(Args)...);
	}
	void Delete()
	{
		((T*)&m_Storage)->~T();
	}

	T& operator*() const { return *(T*)&m_Storage; }
	T* operator->() const { return (T*)&m_Storage; }
	operator T*() const { return (T*)&m_Storage; }
};
