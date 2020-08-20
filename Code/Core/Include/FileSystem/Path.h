#pragma once 
#include <CoreConfig.h>
#include <CoreType.h>
#include <String/String.h>
#include <String/Name.h>

namespace Fuko
{
	class Path
	{
	public:
		String Encode(TCHAR Separator = 0, bool WithRoot = false);
		bool IsSubPath(const Path& InPath);

		FORCEINLINE Name Root() { return m_Root; }
		FORCEINLINE uint32 NumNodes() { return m_NodeNum; }
		FORCEINLINE Name GetNodes(uint32 Index) { return m_Nodes[Index]; }
		FORCEINLINE Name FileName() { return m_FileName; }
	private:
		Name			m_Root;
		Name			m_FileName;
		Name			m_Nodes[64];
		uint32			m_NodeNum;

		uint32			m_Hash;
	};
}
