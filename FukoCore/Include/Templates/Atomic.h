#pragma once
#include "CoreConfig.h"
#include "CoreType.h"
#include <xatomic0.h>

namespace Fuko
{
	enum EMemoryOrder
	{
		// 非原子的操作
		Relaxed = std::memory_order_relaxed,

		// 原子操作
		SequentiallyConsistent = std::memory_order_seq_cst
	};

	template<typename T>
	class TAtomic
	{
		TAtomic(TAtomic&&) = delete;
		TAtomic(const TAtomic&) = delete;
		TAtomic& operator=(TAtomic&&) = delete;
		TAtomic& operator=(const TAtomic&) = delete;
	public:
		FORCEINLINE TAtomic() = default;
		constexpr TAtomic(T Arg) : _Atomic(Arg) {}

		FORCEINLINE operator T() const
		{
			return this->Load();
		}

		FORCEINLINE T operator=(T Value)
		{
			this->Exchange(Value);
			return Value;
		}

		FORCEINLINE T Load(EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent) const
		{
			return _Atomic.load((std::memory_order)Order);
		}

		FORCEINLINE void Store(T Value, EMemoryOrder Order = EMemoryOrder::SequentiallyConsistent)
		{
			_Atomic.store(Value, (std::memory_order)Order);
		}

		FORCEINLINE T Exchange(T Value)
		{
			return _Atomic.exchange(Value);
		}

		FORCEINLINE bool CompareExchange(T& Expected, T Value)
		{
			return _Atomic.compare_exchange_strong(Expected, Value);
		}
	private:
		std::atomic<T>		_Atomic;
	};

}
