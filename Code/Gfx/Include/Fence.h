#pragma once
#include "GfxConfig.h"
#include "DeviceChild.h"

namespace Fuko::Gfx
{
	struct IFence : public DeviceChild
	{
		IFence(IGfxDevice* InDevice) : DeviceChild(InDevice) {}
		virtual ~IFence() {}
	};
}
