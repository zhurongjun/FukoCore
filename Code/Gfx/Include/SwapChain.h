#pragma once
#include "GfxConfig.h"
#include "DeviceChild.h"

namespace Fuko::Gfx
{
	struct ISwapChain : public DeviceChild
	{
		ISwapChain(IGfxDevice* InDevice) : DeviceChild(InDevice) {}
		virtual ~ISwapChain() {}
		
	};
}
