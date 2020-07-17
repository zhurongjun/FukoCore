#include <Templates/Align.h>
#include <Math/MathUtility.h>
#include <CoreMinimal/Assert.h>
#include <Memory/MemoryOps.h>
#include <Memory/MemoryPolicy.h>

namespace Fuko
{
	IAllocator* DefaultAllocator()
	{
		static HeapAllocator s_HeapAllocator;
		return &s_HeapAllocator;
	}

}
