#pragma once
#include "GfxConfig.h"
#include "DeviceChild.h"

namespace Fuko::Gfx
{
	struct ICmdBuffer : public DeviceChild
	{
		ICmdBuffer(IGfxDevice* InDevice) : DeviceChild(InDevice) {}
		virtual ~ICmdBuffer() {}
	};
}
