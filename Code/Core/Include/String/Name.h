#pragma once
#include <CoreConfig.h>
#include <CoreType.h>
#include <Containers/ContainerFwd.h>
#include <Memory/MemoryPolicy.h>
#include <Templates/TypeHash.h>
#include <String/Name.h>
#include <Containers/Set.h>
#include <Templates/Pair.h>
#include <String/Name.h>
#include <String/CString.h>

// Name Element
namespace Fuko
{
	template<typename T>
	struct TNameElement
	{
		const T*	NamePtr = nullptr;
		uint32		NameLen = 0;

		bool operator==(const TNameElement& Rhs) const
		{
			if (NameLen != Rhs.NameLen) return false;
			if (NamePtr == Rhs.NamePtr) return true;
			return TCString<T>::Strcmp(NamePtr, Rhs.NamePtr) == 0;
		}
	};
	template<typename T>
	uint32 GetTypeHash(const TNameElement<T>& Element) { return Crc::StrCrc32(Element.NamePtr, Element.NameLen); }
}

// Name Pool
namespace Fuko
{
	// Alloc 4KB per grow 
	constexpr uint32 NamePageSize = 4_Kb;
	// if free memory lower then 64, we will skip it for faster search
	constexpr uint32 MinFreeSize = 64;	

	// grow only pool 
	class NamePool
	{
		TArray<TPair<void*, uint32>, BaseAlloc>		m_AllPages;
		std::mutex		m_Mutex;
		uint32			m_LastFreeIndex;
	public:
		FORCEINLINE NamePool() : m_LastFreeIndex(0) {}
		FORCEINLINE ~NamePool() { for (TPair<void*, uint32>& It : m_AllPages) delete It.Key; }

		FORCEINLINE void* RequireName(uint32 NameLen)
		{
			std::lock_guard<std::mutex> Lck(m_Mutex);

			// Find free memory 
			for (int i = m_LastFreeIndex; i < m_AllPages.Num(); ++i)
			{
				auto& Element = m_AllPages[i];
				auto SizeAfterAlloc = Element.Value + NameLen;
				auto FreeSize = NamePageSize - SizeAfterAlloc;

				// can alloc 
				if (SizeAfterAlloc < NamePageSize)
				{
					void* ReturnPtr = ((uint8*)Element.Key + Element.Value);
					Element.Value = SizeAfterAlloc;
					// update last free index 
					if (FreeSize <= MinFreeSize) m_LastFreeIndex = i;
					return ReturnPtr;
				}
			}

			// Alloc new page
			TPair<void*, uint32> NewPage(nullptr, NameLen);
			NewPage.Key = new uint8[NamePageSize];
			m_AllPages.Add(NewPage);

			return NewPage.Key;
		}
	};
}

// Name
namespace Fuko
{
	template<typename T>
	class TName
	{
		using NameElement = TNameElement<T>;
		const NameElement*	m_Ptr;

		// global name table and global name pool 
		static TSet<NameElement, BaseAlloc>	s_NameTable;
		static NamePool						s_NamePool;
	public:
		TName(const T* InStr = TSTR(""));

		// copy construct & assign 
		FORCEINLINE TName(const TName&) = default;
		FORCEINLINE TName(TName&&) = default;
		FORCEINLINE TName& operator=(const TName&) = default;
		FORCEINLINE TName& operator=(TName&&) = default;
		
		// compare 
		FORCEINLINE bool operator==(const TName& Other) { return m_Ptr == Other.m_Ptr; }
		FORCEINLINE bool operator!=(const TName& Other) { return m_Ptr != Other.m_Ptr; }

		// compare with raw ptr 
		FORCEINLINE bool operator==(const T* Str);
		FORCEINLINE friend bool operator==(const T* Lhs, const TName& Rhs);

		// name info 
		FORCEINLINE uint32 Len() const { return m_Ptr->NameLen; }
		FORCEINLINE const T* Data() const { return m_Ptr->NamePtr; }
		FORCEINLINE const T* operator*() const { return m_Ptr->NamePtr; }
	};
}

// Implement 
namespace Fuko
{
	template<typename T>
	inline TSet<TNameElement<T>, BaseAlloc> TName<T>::s_NameTable(2048);

	template<typename T>
	inline NamePool TName<T>::s_NamePool;

	template<typename T>
	FORCEINLINE bool TName<T>::operator==(const T* Str)
	{

		auto Len = TCString<T>::Strlen(Str);
		if (Len != m_Ptr->NameLen) return false;
		return TCString<T>::Strcmp(m_Ptr->NamePtr, Str) == 0;
	}

	template<typename T>
	FORCEINLINE bool operator==(const T* Lhs, const TName<T>& Rhs)
	{
		auto Len = TCString<T>::Strlen(Lhs);
		if (Len != Rhs.m_Ptr->NameLen) return false;
		return TCString<T>::Strcmp(Rhs.m_Ptr->NamePtr, Lhs) == 0;
	}

	template<typename T>
	TName<T>::TName(const T* InStr)
	{
		auto Len = TCString<T>::Strlen(InStr);
		auto NameSize = (Len + 1) * sizeof(T);
		NameElement Element = { InStr,Len };
		uint32 NameHash = GetTypeHash(Element);

		// find name 
		NameElement* ExistElement = s_NameTable.FindByHash(NameHash, Element);
		if (ExistElement != nullptr)
		{
			m_Ptr = ExistElement;
			return;
		}

		// alloc name storage 
		void* NameStorage = s_NamePool.RequireName(NameSize);
		Memcpy(NameStorage, InStr, NameSize);

		// add to set
		Element.NamePtr = (T*)NameStorage;
		auto Id = s_NameTable.EmplaceNoCheck(NameHash, Element);
		m_Ptr = &s_NameTable[Id];
	}

	template<typename T>
	FORCEINLINE uint32 GetTypeHash(const TName<T>& Element)
	{
		return PointerHash(Element.Data());
	}

	using Name = TName<TCHAR>;
}