#pragma once
#include <CoreConfig.h>
#include <CoreType.h>
#include <Algo/BinarySearch.h>
#include <Algo/Rotate.h>
#include <Templates/Functor.h>

namespace Fuko::Algo
{
	template <class T, class TSize, class TPred>
	void Merge(T* First, const TSize Mid, const TSize Num, TPred&& Pred)
	{
		TSize AStart = 0;
		TSize BStart = Mid;

		while (AStart < BStart && BStart < Num)
		{
			// Index after the last value == First[BStart]
			TSize NewAOffset = Impl::UpperBoundInternal(First + AStart, BStart - AStart, First[BStart], NoMap(), std::forward<TPred>(Pred));
			AStart += NewAOffset;

			if (AStart >= BStart) // done
				break;

			// Index of the first value == First[AStart]
			TSize NewBOffset = Impl::LowerBoundInternal(First + BStart, Num - BStart, First[AStart], NoMap(), std::forward<TPred>(Pred));
			Rotate(First, AStart, BStart + NewBOffset, NewBOffset);
			BStart += NewBOffset;
			AStart += NewBOffset + 1;
		}
	}

	template<class T, class TSize, class TPred, int MinMergeSubgroupSize = 2>
	void StableSort(T* First, const TSize Num, TPred&& Pred)
	{
		TSize SubgroupStart = 0;

		if constexpr (MinMergeSubgroupSize > 1)
		{
			if constexpr (MinMergeSubgroupSize > 2)
			{
				// First pass with simple bubble-sort.
				do
				{
					TSize GroupEnd = FMath::Min(SubgroupStart + MinMergeSubgroupSize, Num);
					do
					{
						for (TSize It = SubgroupStart; It < GroupEnd - 1; ++It)
						{
							if (Pred(First[It + 1], First[It]))
							{
								Swap(First[It], First[It + 1]);
							}
						}
						GroupEnd--;
					} while (GroupEnd - SubgroupStart > 1);

					SubgroupStart += MinMergeSubgroupSize;
				} while (SubgroupStart < Num);
			}
			else
			{
				for (TSize Subgroup = 0; Subgroup < Num; Subgroup += 2)
				{
					if (Subgroup + 1 < Num && Pred(First[Subgroup + 1], First[Subgroup]))
					{
						Swap(First[Subgroup], First[Subgroup + 1]);
					}
				}
			}
		}

		TSize SubgroupSize = MinMergeSubgroupSize;
		while (SubgroupSize < Num)
		{
			SubgroupStart = 0;
			do
			{
				Merge(
					First + SubgroupStart,
					SubgroupSize,
					FMath::Min(SubgroupSize << 1, Num - SubgroupStart),
					std::forward<TPred>(Pred));
				SubgroupStart += SubgroupSize << 1;
			} while (SubgroupStart < Num);

			SubgroupSize <<= 1;
		}
	}
}
