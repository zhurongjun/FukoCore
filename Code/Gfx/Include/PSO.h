#pragma once
#include "GfxConfig.h"
#include "DeviceChild.h"

namespace Fuko::Gfx
{
	struct IGraphicsPSO : public DeviceChild
	{
		IGraphicsPSO(IGfxDevice* InDevice) : DeviceChild(InDevice) {}
		virtual ~IGraphicsPSO() {}
	};

	struct IComputePSO : public DeviceChild
	{
		IGraphicsPSO(IGfxDevice* InDevice) : DeviceChild(InDevice) {}
		virtual ~IComputePSO() {}
	};
}
