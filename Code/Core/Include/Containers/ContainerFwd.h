#pragma once

// allocator 
namespace Fuko
{
	class PmrAlloc;
	class BaseAlloc;
	template<typename FallBackAlloc = PmrAlloc>
	class TBlockAlloc;
}

// lock policy
namespace Fuko
{
	struct NoLock;
	struct LockFree;
	struct MutexLock;
	template<int YieldTime = -1>
	struct TSpinLock;
}


// container
namespace Fuko
{
	template<typename T, typename Alloc = PmrAlloc>
	class TArray;

	template<typename Alloc = PmrAlloc>
	class TBitArray;

	template<typename T, typename TLockPolicy = NoLock, typename TAlloc = PmrAlloc>
	class TPool;

	template<typename T, bool bInAllowDuplicateKeys = false>
	struct DefaultKeyFuncs;

	template<typename T, typename Alloc = PmrAlloc, typename KeyFuncs = DefaultKeyFuncs<T>>
	class TSet;

	template<typename TK, typename TV, bool bInAllowDuplicateKeys>
	struct TDefaultMapKeyFuncs;

	template<typename KeyType, typename ValueType
		, typename Alloc = PmrAlloc
		, typename KeyFuncs = TDefaultMapKeyFuncs<KeyType, ValueType, false>>
		class TMap;

	template<typename KeyType, typename ValueType
		, typename Alloc = PmrAlloc
		, typename KeyFuncs = TDefaultMapKeyFuncs<KeyType, ValueType, true>>
		class TMultiMap;

	template<typename T, typename TLockPolicy = NoLock, typename Alloc = PmrAlloc>
	class TRingQueue;

	template<typename T, typename Alloc = PmrAlloc>
	class TSparseArray;
}