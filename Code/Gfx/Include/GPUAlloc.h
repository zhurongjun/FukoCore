#pragma once
#include "GfxConfig.h"
#include "DeviceChild.h"

namespace Fuko::Gfx
{
	// GpuResource Allocator 
	struct GPUAllocator : public DeviceChild
	{
		GPUAllocator(IGfxDevice* InDevice) : DeviceChild(InDevice) {}
		virtual ~GPUAllocator()

		size_t		Size() { return m_Size; }



	protected:
		size_t			m_Size;
		SP<IGfxDevice>	m_Device;
	};

	// DescriptorHeap 
	struct DescHeapAllocator : public DeviceChild
	{
		DescHeapAllocator(IGfxDevice* InDevice) : DeviceChild(InDevice) {}
		virtual ~DescHeapAllocator() {}


	protected:
		SP<IGfxDevice>	m_Device;
	};
}

