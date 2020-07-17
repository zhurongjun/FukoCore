#pragma once
#include <CoreConfig.h>
#include <CoreType.h>
#include "LockPolicy.h"
#include "Array.h"

// Forward 
namespace Fuko
{
	template<typename T, typename TLockPolicy = NoLock, typename TAlloc = PmrAlloc>
	class TPool;
}

// TPool NoLock 
namespace Fuko
{
	template<typename T, typename TAlloc>
	class TPool<T, NoLock, TAlloc> final
	{
		using SizeType = typename TAlloc::USizeType;
		TArray<T*, TAlloc>		m_PtrPool;
		TArray<T*,TAlloc>	m_Blocks;
		SizeType				m_BlockSize;
		
		void _AllocBlock(SizeType BlockNum = 1)
		{
			// alloc memory
			for (int i = 0; i < BlockNum; ++i)
			{
				T* NewBlock = nullptr;
				m_BlockSize = m_PtrPool.GetAllocator().Reserve(NewBlock, m_BlockSize);
				m_Blocks.Add(NewBlock);
				for (int j = 0; j < m_BlockSize; ++j)
				{
					m_PtrPool.Add(NewBlock + j);
				}
			}
		}
	public:
		TPool(SizeType BlockSize, SizeType InitBlockNum = 1, const TAlloc& InAlloc = TAlloc())
			: m_Blocks(InitBlockNum > 4 ? InitBlockNum : 4, InAlloc)
			, m_PtrPool(InitBlockNum * BlockSize, InAlloc)
			, m_BlockSize(BlockSize)
		{
			_AllocBlock(InitBlockNum);
		}
		~TPool()
		{
			for (T*& Ptr : m_Blocks)
			{
				m_Blocks.GetAllocator().Free<T>(Ptr);
			}
		}

		bool IsInPool(void* Memory)
		{
			for (T* Ptr : m_Blocks)
			{
				if (Memory >= Ptr && Memory < (void*)(Ptr + m_BlockSize)) return true;
			}
			return false;
		}

		T* Alloc() 
		{ 
			if (m_PtrPool.IsEmpty()) _AllocBlock();
			return m_PtrPool.Pop(false);
		}
		void Free(T* Ptr) 
		{ 
			m_PtrPool.Push(Ptr); 
		}
		
		template<typename...Ts>
		T* New(Ts&&...Args)
		{
			T* RetPtr = Alloc();
			return new(RetPtr) T(std::forward<Ts>(Args)...);
		}
		void Delete(T* Ptr)
		{
			Ptr->~T();
			Free(Ptr);
		}
	};
}

// TPool Locked
namespace Fuko
{
	template<typename T,typename TLockPolicy, typename TAlloc>
	class TPool final
	{
		using SizeType = typename TAlloc::USizeType;
		TArray<T*, TAlloc>		m_PtrPool;
		TArray<T*, TAlloc>		m_Blocks;
		SizeType				m_BlockSize;
		TLockPolicy				m_LockPolicy;

		void _AllocBlock(SizeType BlockNum = 1)
		{
			// alloc memory
			for (int i = 0; i < BlockNum; ++i)
			{
				T* NewBlock = nullptr;
				m_BlockSize = m_PtrPool.GetAllocator().Reserve(NewBlock, m_BlockSize);
				m_Blocks.Add(NewBlock);
				for (int j = 0; j < m_BlockSize; ++j)
				{
					m_PtrPool.Add(NewBlock + j);
				}
			}
		}
	public:
		TPool(SizeType BlockSize, SizeType InitBlockNum = 1, const TAlloc& InAlloc = TAlloc())
			: m_Blocks(InitBlockNum > 4 ? InitBlockNum : 4, InAlloc)
			, m_PtrPool(InitBlockNum * BlockSize, InAlloc)
			, m_BlockSize(BlockSize)
		{
			_AllocBlock(InitBlockNum);
		}
		~TPool()
		{
			for (T*& Ptr : m_Blocks)
			{
				m_Blocks.GetAllocator().Free<T>(Ptr);
			}
		}

		bool IsInPool(void* Memory)
		{
			for (T* Ptr : m_Blocks)
			{
				if (Memory >= Ptr && Memory < (Ptr + m_BlockSize)) return true;
			}
			return false;
		}

		T* Alloc()
		{
			std::lock_guard<TLockPolicy>	Lck(m_LockPolicy);
			if (m_PtrPool.IsEmpty()) _AllocBlock();
			return m_PtrPool.Pop(false);
		}
		void Free(T* Ptr)
		{
			std::lock_guard<TLockPolicy>	Lck(m_LockPolicy);
			m_PtrPool.Push(Ptr);
		}

		template<typename...Ts>
		T* New(Ts&&...Args)
		{
			T* RetPtr = Alloc();
			return new(RetPtr) T(std::forward<Ts>(Args)...);
		}
		void Delete(T* Ptr)
		{
			Ptr->~T();
			Free(Ptr);
		}
	};
}
