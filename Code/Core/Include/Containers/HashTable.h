#pragma once
#include <CoreConfig.h>
#include <CoreType.h>
#include <initializer_list>
#include <Math/MathUtility.h>
#include <Memory/Memory.h>
#include <Templates/UtilityTemp.h>

// Murmurhash
namespace Fuko
{
	static FORCEINLINE uint32 MurmurFinalize32(uint32 Hash)
	{
		Hash ^= Hash >> 16;
		Hash *= 0x85ebca6b;
		Hash ^= Hash >> 13;
		Hash *= 0xc2b2ae35;
		Hash ^= Hash >> 16;
		return Hash;
	}

	static FORCEINLINE uint64 MurmurFinalize64(uint64 Hash)
	{
		Hash ^= Hash >> 33;
		Hash *= 0xff51afd7ed558ccdull;
		Hash ^= Hash >> 33;
		Hash *= 0xc4ceb9fe1a85ec53ull;
		Hash ^= Hash >> 33;
		return Hash;
	}

	static FORCEINLINE uint32 Murmur32(std::initializer_list< uint32 > InitList)
	{
		uint32 Hash = 0;
		for (auto Element : InitList)
		{
			Element *= 0xcc9e2d51;
			Element = (Element << 15) | (Element >> (32 - 15));
			Element *= 0x1b873593;

			Hash ^= Element;
			Hash = (Hash << 13) | (Hash >> (32 - 13));
			Hash = Hash * 5 + 0xe6546b64;
		}

		return MurmurFinalize32(Hash);
	}
}

// StaicHashTable
namespace Fuko
{
	template< uint16 HashSize, uint16 IndexSize >
	class TStaticHashTable
	{
	public:
		TStaticHashTable();

		/**
		 * @fn void TStaticHashTable::Clear();
		 *
		 * @brief 清空哈希表
		 */
		void		Clear();

		/**
		 * @fn uint16 TStaticHashTable::First(uint16 Key) const;
		 *
		 * @brief 得到对应Key的Index链表中的第一个
		 *
		 * @param  Key Key值
		 *
		 * @returns 对应的Index首部
		 */
		uint16		First(uint16 Key) const;

		/**
		 * @fn uint16 TStaticHashTable::Next(uint16 Index) const;
		 *
		 * @brief 得到Index链表中的下一项
		 *
		 * @param  当前的Index
		 *
		 * @returns 下一个Index
		 */
		uint16		Next(uint16 Index) const;

		/**
		 * @fn bool TStaticHashTable::IsValid(uint16 Index) const;
		 *
		 * @brief 查询Index是否是合法的
		 *
		 * @param  Index 具体的Index
		 *
		 * @returns 是否是有效的Index
		 */
		bool		IsValid(uint16 Index) const;

		/**
		 * @fn void TStaticHashTable::Add(uint16 Key, uint16 Index);
		 *
		 * @brief Adds Key
		 *
		 * @param  Key   想要添加的Key
		 * @param  Index 具体的Index
		 */
		void		Add(uint16 Key, uint16 Index);

		/**
		 * @fn void TStaticHashTable::Remove(uint16 Key, uint16 Index);
		 *
		 * @brief 移除一个元素
		 *
		 * @param  Key   想要移除的Key
		 * @param  Index Key对应的具体Index
		 */
		void		Remove(uint16 Key, uint16 Index);

	protected:
		uint16		Hash[HashSize];
		uint16		NextIndex[IndexSize];
	};

	template< uint16 HashSize, uint16 IndexSize >
	FORCEINLINE TStaticHashTable< HashSize, IndexSize >::TStaticHashTable()
	{
		static_assert((HashSize & (HashSize - 1)) == 0, "Hash size must be power of 2");
		static_assert(IndexSize - 1 < 0xffff, "Index 0xffff is reserved");
		Clear();
	}

	template< uint16 HashSize, uint16 IndexSize >
	FORCEINLINE void TStaticHashTable< HashSize, IndexSize >::Clear()
	{
		Memset(Hash, 0xff, HashSize * 2);
	}

	template< uint16 HashSize, uint16 IndexSize >
	FORCEINLINE uint16 TStaticHashTable< HashSize, IndexSize >::First(uint16 Key) const
	{
		Key &= HashSize - 1;
		return Hash[Key];
	}

	template< uint16 HashSize, uint16 IndexSize >
	FORCEINLINE uint16 TStaticHashTable< HashSize, IndexSize >::Next(uint16 Index) const
	{
		check(Index < IndexSize);
		return NextIndex[Index];
	}

	template< uint16 HashSize, uint16 IndexSize >
	FORCEINLINE bool TStaticHashTable< HashSize, IndexSize >::IsValid(uint16 Index) const
	{
		return Index != 0xffff;
	}

	template< uint16 HashSize, uint16 IndexSize >
	FORCEINLINE void TStaticHashTable< HashSize, IndexSize >::Add(uint16 Key, uint16 Index)
	{
		check(Index < IndexSize);

		Key &= HashSize - 1;
		NextIndex[Index] = Hash[Key];
		Hash[Key] = Index;
	}

	template< uint16 HashSize, uint16 IndexSize >
	inline void TStaticHashTable< HashSize, IndexSize >::Remove(uint16 Key, uint16 Index)
	{
		check(Index < IndexSize);

		Key &= HashSize - 1;

		// 刚好在首部
		if (Hash[Key] == Index)
		{
			Hash[Key] = NextIndex[Index];
		}
		else
		{
			for (uint16 i = Hash[Key]; IsValid(i); i = NextIndex[i])
			{
				if (NextIndex[i] == Index)
				{
					// Next = Next->Next
					NextIndex[i] = NextIndex[Index];
					break;
				}
			}
		}
	}
}

// Dynamic HashTable
namespace Fuko
{
	class FHashTable
	{
	public:
		FHashTable(uint32 InHashSize = 1024, uint32 InIndexSize = 0);
		FHashTable(const FHashTable& Other);
		~FHashTable();

		void			Initialize(uint32 InHashSize = 1024, uint32 InIndexSize = 0);

		void			Clear();
		void			Free();
		CORE_API void	Resize(uint32 NewIndexSize);

		uint32			First(uint16 Key) const;
		uint32			Next(uint32 Index) const;
		bool			IsValid(uint32 Index) const;
		bool			Contains(uint16 Key) const;

		void			Add(uint16 Key, uint32 Index);
		void			Remove(uint16 Key, uint32 Index);

		CORE_API float	AverageSearch() const;

	protected:
		// Avoids allocating hash until first add
		static uint32	EmptyHash[1];

		uint32			HashSize;
		uint16			HashMask;
		uint32			IndexSize;

		uint32*			Hash;
		uint32*			NextIndex;
	};


	FORCEINLINE FHashTable::FHashTable(uint32 InHashSize, uint32 InIndexSize)
		: HashSize(0)
		, HashMask(0)
		, IndexSize(0)
		, Hash(EmptyHash)
		, NextIndex(NULL)
	{
		if (InHashSize > 0u)
		{
			Initialize(InHashSize, InIndexSize);
		}
	}

	FORCEINLINE FHashTable::FHashTable(const FHashTable& Other)
		: HashSize(0)
		, HashMask(0)
		, IndexSize(0)
		, Hash(EmptyHash)
		, NextIndex(NULL)
	{
		if (Other.HashSize > 0u)
		{
			Initialize(Other.HashSize, Other.IndexSize);

			check(HashSize == Other.HashSize);
			check(HashMask == Other.HashMask);
			check(IndexSize == Other.IndexSize);

			Memcpy(Hash, Other.Hash, HashSize * 4);
			Memcpy(NextIndex, Other.NextIndex, IndexSize * 4);
		}
	}

	FORCEINLINE void FHashTable::Initialize(uint32 InHashSize, uint32 InIndexSize)
	{
		check(HashSize == 0u);
		check(IndexSize == 0u);

		HashSize = InHashSize;
		IndexSize = InIndexSize;

		check(HashSize <= 0x10000);
		check(FMath::IsPowerOfTwo(HashSize));

		if (IndexSize)
		{
			HashMask = (uint16)(HashSize - 1);

			Hash = new uint32[HashSize];
			NextIndex = new uint32[IndexSize];

			Memset(Hash, 0xff, HashSize * 4);
		}
	}

	FORCEINLINE FHashTable::~FHashTable()
	{
		Free();
	}

	FORCEINLINE void FHashTable::Clear()
	{
		if (IndexSize)
		{
			Memset(Hash, 0xff, HashSize * 4);
		}
	}

	FORCEINLINE void FHashTable::Free()
	{
		if (IndexSize)
		{
			HashMask = 0;
			IndexSize = 0;

			delete[] Hash;
			Hash = EmptyHash;

			delete[] NextIndex;
			NextIndex = NULL;
		}
	}

	// First in hash chain
	FORCEINLINE uint32 FHashTable::First(uint16 Key) const
	{
		Key &= HashMask;
		return Hash[Key];
	}

	// Next in hash chain
	FORCEINLINE uint32 FHashTable::Next(uint32 Index) const
	{
		check(Index < IndexSize);
		check(NextIndex[Index] != Index); // check for corrupt tables
		return NextIndex[Index];
	}

	FORCEINLINE bool FHashTable::IsValid(uint32 Index) const
	{
		return Index != ~0u;
	}

	FORCEINLINE bool FHashTable::Contains(uint16 Key) const
	{
		return First(Key) != ~0u;
	}

	FORCEINLINE void FHashTable::Add(uint16 Key, uint32 Index)
	{
		if (Index >= IndexSize)
		{
			Resize(FMath::Max<uint32>(32u, FMath::RoundUpToPowerOfTwo(Index + 1)));
		}

		Key &= HashMask;
		NextIndex[Index] = Hash[Key];
		Hash[Key] = Index;
	}

	inline void FHashTable::Remove(uint16 Key, uint32 Index)
	{
		if (Index >= IndexSize)
		{
			return;
		}

		Key &= HashMask;

		if (Hash[Key] == Index)
		{
			// Head of chain
			Hash[Key] = NextIndex[Index];
		}
		else
		{
			for (uint32 i = Hash[Key]; IsValid(i); i = NextIndex[i])
			{
				if (NextIndex[i] == Index)
				{
					// Next = Next->Next
					NextIndex[i] = NextIndex[Index];
					break;
				}
			}
		}
	}
}

// Template HashTable
namespace Fuko
{
	template<typename InAllocator>
	class THashTable
	{
	public:
		using Allocator = InAllocator;

		using ElementAllocatorType = std::conditional_t<
			Allocator::NeedsElementType,
			typename Allocator::template ForElementType<uint32>,
			typename Allocator::ForAnyElementType
		>;

		explicit THashTable(uint32 InHashSize = 1024, uint32 InIndexSize = 0);
		THashTable(const THashTable& Other) = delete;
		THashTable(THashTable&& Other) { MoveAssign(std::move(Other)); }
		~THashTable();

		THashTable& operator=(const THashTable& Other) = delete;
		THashTable& operator=(THashTable&& Other) { return MoveAssign(std::move(Other)); }

		THashTable&		MoveAssign(THashTable&& Other);
		void			Clear();
		void			Resize(uint32 NewIndexSize);

		const uint32*	GetNextIndices() const { return (uint32*)NextIndex.GetAllocation(); }

		// Functions used to search
		uint32			First(uint16 Key) const;
		uint32			Next(uint32 Index) const;
		bool			IsValid(uint32 Index) const;
		bool			Contains(uint16 Key) const;

		void			Add(uint16 Key, uint32 Index);
		void			Remove(uint16 Key, uint32 Index);

	private:
		FORCEINLINE uint32 HashAt(uint32 Index) const { return ((uint32*)Hash.GetAllocation())[Index]; }
		FORCEINLINE uint32 NextIndexAt(uint32 Index) const { return ((uint32*)NextIndex.GetAllocation())[Index]; }
		FORCEINLINE uint32& HashAt(uint32 Index) { return ((uint32*)Hash.GetAllocation())[Index]; }
		FORCEINLINE uint32& NextIndexAt(uint32 Index) { return ((uint32*)NextIndex.GetAllocation())[Index]; }

		ElementAllocatorType	Hash;
		ElementAllocatorType	NextIndex;
		uint32					HashMask;
		uint32					IndexSize;
	};

	template<typename InAllocator>
	FORCEINLINE THashTable<InAllocator>::THashTable(uint32 InHashSize, uint32 InIndexSize)
		: HashMask(InHashSize - 1u)
		, IndexSize(InIndexSize)
	{
		check(InHashSize > 0u && InHashSize <= 0x10000);
		check(FMath::IsPowerOfTwo(InHashSize));

		Hash.ResizeAllocation(0, InHashSize, sizeof(uint32));
		Memset(Hash.GetAllocation(), 0xff, InHashSize * 4);

		if (IndexSize)
		{
			NextIndex.ResizeAllocation(0, IndexSize, sizeof(uint32));
		}
	}

	template<typename InAllocator>
	FORCEINLINE THashTable<InAllocator>::~THashTable()
	{
	}

	template<typename InAllocator>
	THashTable<InAllocator>& THashTable<InAllocator>::MoveAssign(THashTable&& Other)
	{
		Hash.MoveToEmpty(Other.Hash);
		NextIndex.MoveToEmpty(Other.NextIndex);
		HashMask = Other.HashMask;
		IndexSize = Other.IndexSize;
		Other.HashMask = 0u;
		Other.IndexSize = 0u;
		return *this;
	}

	template<typename InAllocator>
	FORCEINLINE void THashTable<InAllocator>::Clear()
	{
		if (IndexSize)
		{
			Memset(Hash.GetAllocation(), 0xff, (HashMask + 1u) * 4);
		}
	}

	// First in hash chain
	template<typename InAllocator>
	FORCEINLINE uint32 THashTable<InAllocator>::First(uint16 Key) const
	{
		Key &= HashMask;
		return HashAt(Key);
	}

	// Next in hash chain
	template<typename InAllocator>
	FORCEINLINE uint32 THashTable<InAllocator>::Next(uint32 Index) const
	{
		check(Index < IndexSize);
		const uint32 Next = NextIndexAt(Index);
		check(Next != Index); // check for corrupt tables
		return Next;
	}

	template<typename InAllocator>
	FORCEINLINE bool THashTable<InAllocator>::IsValid(uint32 Index) const
	{
		return Index != ~0u;
	}

	template<typename InAllocator>
	FORCEINLINE bool THashTable<InAllocator>::Contains(uint16 Key) const
	{
		return First(Key) != ~0u;
	}

	template<typename InAllocator>
	FORCEINLINE void THashTable<InAllocator>::Add(uint16 Key, uint32 Index)
	{
		if (Index >= IndexSize)
		{
			Resize(FMath::Max<uint32>(32u, FMath::RoundUpToPowerOfTwo(Index + 1)));
		}

		Key &= HashMask;
		NextIndexAt(Index) = HashAt(Key);
		HashAt(Key) = Index;
	}

	template<typename InAllocator>
	inline void THashTable<InAllocator>::Remove(uint16 Key, uint32 Index)
	{
		if (Index >= IndexSize)
		{
			return;
		}

		Key &= HashMask;
		if (HashAt(Key) == Index)
		{
			// Head of chain
			HashAt(Key) = NextIndexAt(Index);
		}
		else
		{
			for (uint32 i = HashAt(Key); IsValid(i); i = NextIndexAt(i))
			{
				if (NextIndexAt(i) == Index)
				{
					// Next = Next->Next
					NextIndexAt(i) = NextIndexAt(Index);
					break;
				}
			}
		}
	}

	template<typename InAllocator>
	void THashTable<InAllocator>::Resize(uint32 NewIndexSize)
	{
		if (NewIndexSize != IndexSize)
		{
			NextIndex.ResizeAllocation(IndexSize, NewIndexSize, sizeof(uint32));
			IndexSize = NewIndexSize;
		}
	}
}
