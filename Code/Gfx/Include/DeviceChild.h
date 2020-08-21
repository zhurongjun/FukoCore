#pragma once

namespace Fuko::Gfx
{
	struct IGfxDevice;
	struct DeviceChild
	{
		IGfxDevice* Device() { return m_Device; }
	protected:
		DeviceChild(IGfxDevice* InDevice) : m_Device(InDevice) {}
	protected:
		IGfxDevice*		m_Device;
	};
}
