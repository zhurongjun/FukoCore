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
		const TCHAR* NamePtr = nullptr;
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

		FORCEINLINE bool operator==(const Name& Other) { return m_Ptr == Other.m_Ptr; }
		FORCEINLINE bool operator!=(const Name& Other) { return m_Ptr != Other.m_Ptr; }
	};
}

#define MAKE_NAME(Str) ([]{ static constexpr auto Hash = L##Str##_HashLen; return Name(L##Str, Hash);})()

