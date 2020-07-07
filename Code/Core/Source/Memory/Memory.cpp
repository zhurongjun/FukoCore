#include "Memory/Memory.h"
#include "Templates/Align.h"
#include "Math/MathUtility.h"
#include "CoreMinimal/Assert.h"
#include "Memory/Allocators.h"

namespace Fuko
{
	IAllocator* DefaultAllocator()
	{
		static HeapAllocator s_HeapAllocator;
		return &s_HeapAllocator;
	}
}
