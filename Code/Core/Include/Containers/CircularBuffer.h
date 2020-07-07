#pragma once

namespace Fuko
{
	template<typename ElementType> 
	class TCircularBuffer
	{
	public:
		explicit TCircularBuffer(uint32 Capacity)
		{
			check(Capacity > 0);
			check(Capacity <= 0xffffffffU);

			Elements.AddZeroed(FMath::RoundUpToPowerOfTwo(Capacity));
			IndexMask = Elements.Num() - 1;
		}

		TCircularBuffer(uint32 Capacity, const ElementType& InitialValue)
		{
			check(Capacity <= 0xffffffffU);

			Elements.Init(InitialValue, FMath::RoundUpToPowerOfTwo(Capacity));
			IndexMask = Elements.Num() - 1;
		}

	public:
		FORCEINLINE ElementType& operator[](uint32 Index)
		{
			return Elements[Index & IndexMask];
		}

		FORCEINLINE const ElementType& operator[](uint32 Index) const
		{
			return Elements[Index & IndexMask];
		}

	public:
		FORCEINLINE uint32 Capacity() const
		{
			return Elements.Num();
		}

		FORCEINLINE uint32 GetNextIndex(uint32 CurrentIndex) const
		{
			return ((CurrentIndex + 1) & IndexMask);
		}

		FORCEINLINE uint32 GetPreviousIndex(uint32 CurrentIndex) const
		{
			return ((CurrentIndex - 1) & IndexMask);
		}

	private:
		uint32 IndexMask;
		TArray<ElementType> Elements;
	};
}