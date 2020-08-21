#pragma once
#include "GfxConfig.h"
#include "DeviceChild.h"

namespace Fuko::Gfx
{
	struct IRootSignature : public DeviceChild
	{
		IRootSignature(IGfxDevice* InDevice) : DeviceChild(InDevice) {}
		virtual ~IRootSignature() {}

	};

	struct IRootArgument
	{
		virtual ~IRootArgument() {}

	};
}
