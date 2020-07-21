#pragma once
#include <CoreConfig.h>
#include <CoreType.h>
#include <Containers/ContainerFwd.h>
#include <Memory/MemoryPolicy.h>
#include <Templates/TypeHash.h>
#include "CString.h"

// Name Element
namespace Fuko
{
	struct NameElement
	{
		const TCHAR*	NamePtr = nullptr;
		uint32			NameLen = 0;

		bool operator==(const NameElement& Rhs) const
		{
			if (NameLen != Rhs.NameLen) return false;
			if (NamePtr == Rhs.NamePtr) return true;
			return TCString<TCHAR>::Strcmp(NamePtr, Rhs.NamePtr) == 0;
		}
	};
	uint32 GetTypeHash(const NameElement& Element) 
	{ 
		return Crc::StrCrc32(Element.NamePtr, Element.NameLen); 
	}
}
// Name
namespace Fuko
{
	class CORE_API Name
	{
		const NameElement*	m_Ptr;
	public:
		Name(const TCHAR* InStr = TSTR("")); 
		Name(const TCHAR* InStr, uint32 Hash);

		// copy construct & assign 
		Name(const Name&) = default;
		Name(Name&&) = default;
		Name& operator=(const Name&) = default;
		Name& operator=(Name&&) = default;

		// compare 
		FORCEINLINE bool operator==(const Name& Other) { return m_Ptr == Other.m_Ptr; }
		FORCEINLINE bool operator!=(const Name& Other) { return m_Ptr != Other.m_Ptr; }

		// compare with raw ptr 
		FORCEINLINE bool operator==(const TCHAR* Str) 
		{ 
			auto Len = TCString<TCHAR>::Strlen(Str);
			if (Len != m_Ptr->NameLen) return false;
			return TCString<TCHAR>::Strcmp(m_Ptr->NamePtr, Str) == 0; 
		}
		FORCEINLINE friend bool operator==(const TCHAR* Lhs, const Name& Rhs)
		{
			auto Len = TCString<TCHAR>::Strlen(Lhs);
			if (Len != Rhs.m_Ptr->NameLen) return false;
			return TCString<TCHAR>::Strcmp(m_Ptr->NamePtr, Str) == 0;
			return TCString<TCHAR>::Strcmp(Rhs.m_Ptr->NamePtr, Str) == 0;
		}

		// name info 
		FORCEINLINE uint32 Len() { return m_Ptr->NameLen; }
		FORCEINLINE const TCHAR* Data() { return m_Ptr->NamePtr; }
		FORCEINLINE const TCHAR* operator*() { return m_Ptr->NamePtr; }
	};
}

#define MAKE_NAME(Str) ([]{ static constexpr auto Hash = L##Str##_HashLen; return Name(L##Str, Hash);})()

