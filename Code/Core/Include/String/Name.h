#pragma once
#include <CoreConfig.h>
#include <CoreType.h>
#include <Containers/ContainerFwd.h>
#include <Memory/MemoryPolicy.h>
#include "CString.h"

// Name
namespace Fuko
{
	class CORE_API Name
	{
		const TCHAR*	m_Ptr;
	public:
		Name(const TCHAR* InStr = TSTR("")); 
		Name(const TCHAR* InStr, uint32 Hash);

		FORCEINLINE bool operator==(const Name& Other) { return m_Ptr == Other.m_Ptr; }
		FORCEINLINE bool operator!=(const Name& Other) { return m_Ptr != Other.m_Ptr; }
	};
}

#define MAKE_NAME(Str) ([]{ static constexpr auto Hash = L##Str##_HashLen; return Name(L##Str, Hash);})()

