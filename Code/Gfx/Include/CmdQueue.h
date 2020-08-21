#pragma once
#include "GfxConfig.h"
#include "DeviceChild.h"

namespace Fuko::Gfx
{
	struct ICmdQueue : public DeviceChild
	{
		ICmdQueue(IGfxDevice* InDevice) : DeviceChild(InDevice) {}
		virtual ~ICmdQueue(IGfxDevice* InDevice) : DeviceChild(InDevice) {}

	};
}
