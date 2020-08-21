#pragma once
#include "GfxConfig.h"
#include "DeviceChild.h"

namespace Fuko::Gfx
{
	struct IRenderPass : public DeviceChild
	{
		IRenderPass(IGfxDevice* InDevice) : DeviceChild(InDevice) {}
		virtual ~IRenderPass() {}

	};
}
