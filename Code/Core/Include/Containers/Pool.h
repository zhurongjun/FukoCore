#pragma once
#include <CoreConfig.h>
#include <CoreType.h>
#include "LockPolicy.h"
#include "Array.h"
#include "ContainerFwd.h"

// TPool NoLock 
namespace Fuko
{
	template<typename T, typename TAlloc>
	class TPool<T, NoLock, TAlloc> final
	{
		using SizeType = typename TAlloc::USizeType;

		union Node
		{
			T		Data;		// when we alloc, we use data 
			Node*	Next;		// when we use free list, we use linked list  
		};
		static constexpr SizeType NodeSize = sizeof(Node);

		TArray<Node*,TAlloc>		m_Blocks;
		Node*						m_FreeList;
		SizeType					m_BlockSize;
		
		//=================================Begin help function=================================
		FORCEINLINE void _LinkToFreeList(Node* InNode)
		{
			InNode->Next = m_FreeList;
			m_FreeList = InNode;
		}
		FORCEINLINE Node* _PopFreeList()
		{
			Node* Ret = m_FreeList;
			m_FreeList = m_FreeList->Next;
			return Ret;
		}
		FORCEINLINE void _AllocBlock(SizeType BlockNum = 1)
		{
			// alloc memory
			for (int i = 0; i < BlockNum; ++i)
			{
				Node* NewBlock = nullptr;
				m_BlockSize = m_Blocks.GetAllocator().Reserve(NewBlock, m_BlockSize);
				m_Blocks.Add(NewBlock);
				for (int j = 0; j < m_BlockSize; ++j)
				{
					_LinkToFreeList(NewBlock);
					++NewBlock;
				}
			}
		}
		//==================================End help function==================================
	public:
		TPool(SizeType BlockSize, SizeType InitBlockNum = 1, const TAlloc& InAlloc = TAlloc())
			: m_Blocks(InitBlockNum > 4 ? InitBlockNum : 4, InAlloc)
			, m_FreeList(nullptr)
			, m_BlockSize(BlockSize)
		{
			_AllocBlock(InitBlockNum);
		}
		~TPool()
		{
			for (Node*& Ptr : m_Blocks)
			{
				m_Blocks.GetAllocator().Free<Node>(Ptr);
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
			if (!m_FreeList) _AllocBlock();
			return (T*)_PopFreeList();
		}
		void Free(T* Ptr) { _LinkToFreeList((Node*)Ptr); }
		
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
		
		union Node
		{
			T		Data;		// when we alloc, we use data 
			Node*	Next;		// when we use free list, we use linked list  
		};
		
		TArray<Node*, TAlloc>	m_Blocks;
		Node*					m_FreeList;
		SizeType				m_BlockSize;
		TLockPolicy				m_LockPolicy;


		//=================================Begin help function=================================
		FORCEINLINE void _LinkToFreeList(Node* InNode)
		{
			InNode->Next = m_FreeList;
			m_FreeList = InNode;
		}
		FORCEINLINE Node* _PopFreeList()
		{
			Node* Ret = m_FreeList;
			m_FreeList = m_FreeList->Next;
			return Ret;
		}
		FORCEINLINE void _AllocBlock(SizeType BlockNum = 1)
		{
			// alloc memory
			for (int i = 0; i < BlockNum; ++i)
			{
				Node* NewBlock = nullptr;
				m_BlockSize = m_Blocks.GetAllocator().Reserve(NewBlock, m_BlockSize);
				m_Blocks.Add(NewBlock);
				for (int j = 0; j < m_BlockSize; ++j)
				{
					_LinkToFreeList(NewBlock);
					++NewBlock;
				}
			}
		}
		//==================================End help function==================================
	public:
		TPool(SizeType BlockSize, SizeType InitBlockNum = 1, const TAlloc& InAlloc = TAlloc())
			: m_Blocks(InitBlockNum > 4 ? InitBlockNum : 4, InAlloc)
			, m_FreeList(nullptr)
			, m_BlockSize(BlockSize)
		{
			_AllocBlock(InitBlockNum);
		}
		~TPool()
		{
			for (Node*& Ptr : m_Blocks)
			{
				m_Blocks.GetAllocator().Free<Node>(Ptr);
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
		void ValidateData()
		{
			Node* CurHead = m_FreeList;
			Node* LastHead;

			SizeType FullElementNum = m_Blocks.Num() * m_BlockSize;
			SizeType Count = 0;

			char* BlockFlagArr = new char[FullElementNum];
			Memzero(BlockFlagArr, FullElementNum);

			while (CurHead)
			{
				bool found = false;
				for (int i = 0; i < m_Blocks.Num(); ++i)
				{
					Node* CurBlock = m_Blocks[i];
					if (CurHead >= CurBlock && CurHead < (CurBlock + m_BlockSize))
					{
						found = true;
						++BlockFlagArr[i*m_BlockSize + (CurHead - CurBlock)];
						break;
					}
				}
				check(found);

				++Count;
				LastHead = CurHead;
				CurHead = CurHead->Next;
			}
			check(Count == FullElementNum);
			for (int i = 0; i < FullElementNum; ++i)
			{
				check(BlockFlagArr[i] == 1);
			}
		}

		T* Alloc()
		{
			std::lock_guard<TLockPolicy>	Lck(m_LockPolicy);
			if (!m_FreeList) _AllocBlock();
			return (T*)_PopFreeList();
		}
		void Free(T* Ptr)
		{
			std::lock_guard<TLockPolicy>	Lck(m_LockPolicy);
			_LinkToFreeList((Node*)Ptr);
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

// TPool Lockfree
namespace Fuko
{
	template<typename T, typename TAlloc>
	class TPool<T, LockFree, TAlloc> final
	{
		using SizeType = typename TAlloc::USizeType;

		union Node
		{
			T					Data;		// when we alloc, we use data 
			std::atomic<Node*>	Next;		// when we use free list, we use linked list  
		};
		static constexpr SizeType NodeSize = sizeof(Node);

		TArray<Node*, TAlloc>	m_Blocks;
		std::atomic<Node*>		m_Head;
		std::atomic<Node*>		m_Tail;
		std::mutex				m_AllocMtx;
		SizeType				m_BlockSize;

		//=================================Begin help function=================================
		FORCEINLINE void _AllocBlock(SizeType BlockNum = 1)
		{
			// alloc memory
			for (int i = 0; i < BlockNum; ++i)
			{
				Node* NewBlock = nullptr;
				m_BlockSize = m_Blocks.GetAllocator().Reserve(NewBlock, m_BlockSize);
				m_Blocks.Add(NewBlock);
				for (int j = 0; j < m_BlockSize; ++j)
				{
					// Link to free list 
					Node* CurTail = m_Tail.load();
					NewBlock->Next.store(nullptr);
					while (!m_Tail.compare_exchange_strong(CurTail, NewBlock));
					if (CurTail) CurTail->Next.store(NewBlock);
					if (m_Head.load() == nullptr) m_Head.store(NewBlock);
					++NewBlock;
				}
			}
		}
		//==================================End help function==================================
	public:
		SizeType Count = 0;
		TPool(SizeType BlockSize, SizeType InitBlockNum = 1, const TAlloc& InAlloc = TAlloc())
			: m_Blocks(InitBlockNum > 4 ? InitBlockNum : 4, InAlloc)
			, m_Head(nullptr)
			, m_Tail(nullptr)
			, m_BlockSize(BlockSize)
		{
			auto Lck = std::lock_guard(m_AllocMtx);
			_AllocBlock(InitBlockNum);
		}
		~TPool()
		{
			for (Node*& Ptr : m_Blocks)
			{
				m_Blocks.GetAllocator().Free<Node>(Ptr);
			}
		}

		bool IsInPool(void* Memory)
		{
			for (Node* Ptr : m_Blocks)
			{
				if (Memory >= Ptr && Memory < (Ptr + m_BlockSize)) return true;
			}
			return false;
		}
		void ValidateData()
		{
			Node* CurHead = m_Head.load();
			Node* CurTail = m_Tail.load();
			Node* LastHead;

			SizeType FullElementNum = m_Blocks.Num() * m_BlockSize;
			SizeType Count = 0;

			char* BlockFlagArr = new char[FullElementNum];
			Memzero(BlockFlagArr, FullElementNum);

			while (CurHead)
			{
				bool found = false;
				for (int i = 0; i < m_Blocks.Num(); ++i)
				{
					Node* CurBlock = m_Blocks[i];
					if (CurHead >= CurBlock && CurHead < (CurBlock + m_BlockSize))
					{
						found = true;
						++BlockFlagArr[i*m_BlockSize + (CurHead - CurBlock)];
						break;
					}
				}
				check(found);

				++Count;
				LastHead = CurHead;
				CurHead = CurHead->Next.load();
			}
			check(LastHead == CurTail);
			check(Count == FullElementNum);
			for (int i = 0; i < FullElementNum; ++i)
			{
				check(BlockFlagArr[i] == 1);
			}
		}

		T* Alloc()
		{
			Node* CurHead = m_Head.load();
			Node* CurTail = m_Tail.load();
			
			// need alloc new block  
		TRY_ALLOC:
			if (!CurHead || !CurTail || CurHead == CurTail)
			{
				std::lock_guard<std::mutex> Lck(m_AllocMtx);
				CurHead = m_Head.load();
				CurTail = m_Tail.load();
				// check again, we only alloc once
				if (!CurHead || !CurTail || CurHead == CurTail)
				{
					_AllocBlock();
					CurHead = m_Head.load();
					CurTail = m_Tail.load();
				}
			}

			// now pop 
			Node* NewHead;
			SizeType LoopCount;
			// wait link write
			do
			{
				NewHead = CurHead->Next.load();
				if (!NewHead) ++Count;
			} while (NewHead == nullptr);
			// get a head element 
			while (!m_Head.compare_exchange_strong(CurHead, NewHead))
			{
				CurTail = m_Tail.load();
				if (CurHead == CurTail) goto TRY_ALLOC;
				do
				{
					NewHead = CurHead->Next.load();
					if (!NewHead) ++Count;
				} while (NewHead == nullptr);
			}
			// now pop and break link 
			CurHead->Next.store(nullptr);
			return (T*)CurHead;
		}
		void Free(T* Ptr)
		{
			check(IsInPool(Ptr));
			Node* CurTail = m_Tail.load();
			Node* NewTail = (Node*)Ptr;
			
			NewTail->Next.store(nullptr);
			while (!m_Tail.compare_exchange_strong(CurTail, NewTail));
			check(CurTail->Next.load() == nullptr);
			CurTail->Next.store(NewTail);
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