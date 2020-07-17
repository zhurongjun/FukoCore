#pragma once
#include <CoreConfig.h>
#include <CoreType.h>
#include "LockPolicy.h"

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
		using SizeType = TAlloc::USizeType;
		TArray<T*, TAlloc>		m_PtrPool;
		TArray<void*,TAlloc>	m_Blocks;
		SizeType				m_BlockSize;
		
		void _AllocBlock(SizeType BlockNum = 1)
		{
			// alloc memory
			for (int i = 0; i < BlockNum; ++i)
			{
				T* NewBlock = nullptr;
				NewBlock = m_PtrPool.GetAllocator().Reserve(NewBlock, m_BlockSize, 0);
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
			, m_Blocks(BlockSize)
		{
			_AllocBlock(InitBlockNum);
		}
		~TPool()
		{
			for (void* Ptr : m_Blocks)
			{
				m_Blocks.GetAllocator().Free<T>((T*)Ptr);
			}
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
	class TPool<T, NoLock, TAlloc> final
	{
		using SizeType = TAlloc::USizeType;
		TArray<T*, TAlloc>		m_PtrPool;
		TArray<void*, TAlloc>	m_Blocks;
		SizeType				m_BlockSize;
		TLockPolicy				m_LockPolicy;

		void _AllocBlock(SizeType BlockNum = 1)
		{
			// alloc memory
			for (int i = 0; i < BlockNum; ++i)
			{
				T* NewBlock = nullptr;
				NewBlock = m_PtrPool.GetAllocator().Reserve(NewBlock, m_BlockSize, 0);
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
			, m_Blocks(BlockSize)
		{
			_AllocBlock(InitBlockNum);
		}
		~TPool()
		{
			for (void* Ptr : m_Blocks)
			{
				m_Blocks.GetAllocator().Free<T>((T*)Ptr);
			}
		}

		bool IsInPool(void* Memory)
		{
			for (void* Ptr : m_Blocks)
			{
				if (Memory >= Ptr && Memory < (void*)((T*)Ptr + m_BlockSize)) return true;
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
