#include <String/Name.h>
#include <Containers/Set.h>
#include <Templates/Pair.h>

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
	uint32 GetTypeHash(const NameElement& Element) { return Crc::StrCrc32(Element.NamePtr, Element.NameLen); }

	constexpr uint32 NamePageSize = 4_Kb;
	class NamePool
	{
		TArray<TPair<void*, uint32>>		m_AllPages;
	public:
		void* RequireName(uint32 NameLen)
		{
			for (TPair<void*, uint32>& It : m_AllPages)
			{
				if (It.Value + NameLen < NamePageSize)
				{
					void* ReturnPtr = ((uint8*)It.Key + It.Value);
					It.Value += NameLen;
					return ReturnPtr;
				}
			}
			TPair<void*, uint32> NewPage(nullptr, NameLen);
			NewPage.Key = DefaultAllocator()->Alloc(NamePageSize);
			m_AllPages.Add(NewPage);
			return NewPage.Key;
		}
		~NamePool()
		{
			for (TPair<void*, uint32>& It : m_AllPages)
			{
				DefaultAllocator()->Free(It.Key);
			}
		}
	};

	TSet<NameElement>		g_NameTable(2048);
	NamePool				g_NamePool;
}

namespace Fuko
{
	Name::Name(const TCHAR* InStr)
	{
		auto Len = TCString<TCHAR>::Strlen(InStr);
		auto NameSize = (Len + 1) * sizeof(TCHAR);
		NameElement Element = { InStr,Len };
		uint32 NameHash = GetTypeHash(Element);
		
		// find name 
		NameElement* ExistElement = g_NameTable.FindByHash(NameHash, Element);
		if (ExistElement != nullptr)
		{
			m_Ptr = ExistElement->NamePtr;
			return;
		}

		// alloc name storage 
		void * NameStorage = g_NamePool.RequireName(NameSize);
		Memcpy(NameStorage, InStr, NameSize);

		// add to set
		Element.NamePtr =(TCHAR*) NameStorage;
		m_Ptr = Element.NamePtr;
		g_NameTable.AddByHash(NameHash, Element);
	}

	Name::Name(const TCHAR* InStr, uint32 NameHash)
	{
		auto Len = TCString<TCHAR>::Strlen(InStr);
		auto NameSize = (Len + 1) * sizeof(TCHAR);
		NameElement Element = { InStr,Len };
		check(NameHash == GetTypeHash(Element));

		// find name 
		NameElement* ExistElement = g_NameTable.FindByHash(NameHash, Element);
		if (ExistElement != nullptr)
		{
			m_Ptr = ExistElement->NamePtr;
			return;
		}

		// alloc name storage 
		void * NameStorage = g_NamePool.RequireName(NameSize);
		Memcpy(NameStorage, InStr, NameSize);

		// add to set
		Element.NamePtr = (TCHAR*)NameStorage;
		m_Ptr = Element.NamePtr;
		g_NameTable.EmplaceNoCheck(NameHash, Element);
	}

}
