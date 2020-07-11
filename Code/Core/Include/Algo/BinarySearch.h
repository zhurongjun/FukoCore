#pragma once
#include <CoreConfig.h>
#include <CoreType.h>
#include <Templates/Functor.h>
#include <Templates/UtilityTemp.h>

namespace Fuko::Algo::Impl
{
	// 查找下界 
	template <typename T, typename TS, typename TV, typename TProj, typename TPred>
	FORCEINLINE TS LowerBoundInternal(T* First, const TS Num, const TV& Value, TProj&& Proj, TPred&& Pred)
	{
		// 查找范围起点 
		TS Start = 0;
		// 查找范围大小 
		TS Size = Num;

		while (Size > 0)
		{
			const TS LeftoverSize = Size % 2;
			Size = Size / 2;

			const TS CheckIndex = Start + Size;	// 中位 
			const TS StartIfLess = CheckIndex + LeftoverSize;	// 如果谓语成立的下一个起始点 
			
			Start = Pred(Proj(First[CheckIndex]), Value) ? StartIfLess : Start;	// 更新起点 
		}
		return Start;
	}

	// 查找上界
	template <typename T, typename TS, typename TV, typename TProj, typename TPred>
	FORCEINLINE TS UpperBoundInternal(T* First, const TS Num, const TV& Value, TProj&& Proj, TPred&& Pred)
	{
		TS Start = 0;
		TS Size = Num;

		while (Size > 0)
		{
			const TS LeftoverSize = Size % 2;
			Size = Size / 2;

			const TS CheckIndex = Start + Size;
			const TS StartIfLess = CheckIndex + LeftoverSize;

			Start = !Pred(Value, Proj(First[CheckIndex])) ? StartIfLess : Start;
		}

		return Start;
	}
}

namespace Fuko::Algo
{
	// 查找下界 
	template <typename TR, typename TV, typename TPred = TLess<>, typename TProj = NoMap>
	FORCEINLINE auto LowerBound(TR& Range, const TV& Value,
		TPred&& Pred = TPred(), TProj&& Proj = TProj()) -> decltype(GetNum(Range))
	{
		return Impl::LowerBoundInternal(GetData(Range), GetNum(Range), Value, 
			std::forward<TProj>(Proj), std::forward<TPred>(Pred));
	}

	// 查找上界
	template <typename TR, typename TV, typename TPred = TLess<>, typename TProj = NoMap>
	FORCEINLINE auto UpperBound(TR& Range, const TV& Value, 
		TPred&& Pred = TPred(), TProj&& Proj = TProj()) -> decltype(GetNum(Range))
	{
		return Impl::UpperBoundInternal(GetData(Range), GetNum(Range), Value, 
			std::forward<TProj>(Proj), std::forward<TPred>(Pred));
	}

	// 二分查找
	template <typename TR, typename TV, typename TPred = TLess<>, typename TProj = NoMap>
	FORCEINLINE auto BinarySearch(TR& Range, const TV& Value,
		TPred&& Pred = TPred(), TProj&& Proj = TProj())
	{
		using TS = decltype(GetNum(Range));
		auto CheckIndex = LowerBound(Range, Value, Pred, Proj);
		if (CheckIndex < GetNum(Range))
		{
			// 使用相反的谓语进行一下测试，假设使用的Functor是Less<>() 
			// 未找到的情况有三种，小于序列中所有值，大于序列中所有值，值在其中但是序列中没有 
			// 对于大于序列中所有值的情况，Index == Num，在上一层就已经筛掉了 
			// 对于小于序列中所有值的情况，Index == 0， 且此次谓语测试为true 
			// 对于在序列范围中的情况，Index上的值大于Value，则此次谓语测试为true 
			// 对于已经找到的情况，此次谓语测试为false 
			if (!Pred(Proj(Value), Proj(GetData(Range)[CheckIndex])))
			{
				return CheckIndex;
			}
		}
		return (TS)INDEX_NONE;
	}
}