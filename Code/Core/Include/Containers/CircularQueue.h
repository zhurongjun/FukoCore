#pragma once
#include "Templates/Atomic.h"

namespace Fuko
{
	template<typename ElementType> class TCircularQueue
	{
	public:
		explicit TCircularQueue(uint32 CapacityPlusOne)
			: Buffer(CapacityPlusOne)
			, Head(0)
			, Tail(0)
		{ }

	public:
		uint32 Count() const
		{
			int32 Count = Tail.Load() - Head.Load();

			if (Count < 0)
			{
				Count += Buffer.Capacity();
			}

			return (uint32)Count;
		}

		bool Dequeue(ElementType& OutElement)
		{
			const uint32 CurrentHead = Head.Load();

			if (CurrentHead != Tail.Load())
			{
				OutElement = std::move(Buffer[CurrentHead]);
				Head.Store(Buffer.GetNextIndex(CurrentHead));

				return true;
			}

			return false;
		}

		bool Dequeue()
		{
			const uint32 CurrentHead = Head.Load();

			if (CurrentHead != Tail.Load())
			{
				Head.Store(Buffer.GetNextIndex(CurrentHead));

				return true;
			}

			return false;
		}

		void Empty()
		{
			Head.Store(Tail.Load());
		}

		bool Enqueue(const ElementType& Element)
		{
			const uint32 CurrentTail = Tail.Load();
			uint32 NewTail = Buffer.GetNextIndex(CurrentTail);

			if (NewTail != Head.Load())
			{
				Buffer[CurrentTail] = Element;
				Tail.Store(NewTail);

				return true;
			}

			return false;
		}

		bool Enqueue(ElementType&& Element)
		{
			const uint32 CurrentTail = Tail.Load();
			uint32 NewTail = Buffer.GetNextIndex(CurrentTail);

			if (NewTail != Head.Load())
			{
				Buffer[CurrentTail] = std::move(Element);
				Tail.Store(NewTail);

				return true;
			}

			return false;
		}

		FORCEINLINE bool IsEmpty() const
		{
			return (Head.Load() == Tail.Load());
		}

		bool IsFull() const
		{
			return (Buffer.GetNextIndex(Tail.Load()) == Head.Load());
		}

		bool Peek(ElementType& OutItem) const
		{
			const uint32 CurrentHead = Head.Load();

			if (CurrentHead != Tail.Load())
			{
				OutItem = Buffer[CurrentHead];

				return true;
			}

			return false;
		}

		const ElementType* Peek() const
		{
			const uint32 CurrentHead = Head.Load();

			if (CurrentHead != Tail.Load())
			{
				return &Buffer[CurrentHead];
			}

			return nullptr;
		}

	private:
		TCircularBuffer<ElementType> Buffer;
		TAtomic<uint32> Head;
		TAtomic<uint32> Tail;
	};

}
