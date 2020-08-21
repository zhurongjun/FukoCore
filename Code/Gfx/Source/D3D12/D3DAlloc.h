#pragma once
#include <GPUAlloc.h>

// D3DGPUAllocator 
namespace Fuko::Gfx
{
	class D3DGPUAllocator : public GPUAllocator
	{
		ID3D12Heap*		m_Heap;
	public:
		

	};
}
