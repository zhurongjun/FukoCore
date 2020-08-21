#pragma once
#include <Gfx.h>
#include "D3D12Config.h"

// system 
namespace Fuko::Gfx
{	
	class D3DAdapter : public IGfxAdapter
	{
		ComPtr<IDXGIAdapter1>	m_Adapter;
		DXGI_ADAPTER_DESC1		m_Desc;
	public:
		D3DAdapter(ComPtr<IDXGIAdapter1> InAdapter);

		const TCHAR* Name() override { return m_Desc.Description; }
		uint64	LocalMemory() override { return m_Desc.DedicatedVideoMemory; }
		uint64	SharedMemory() override { return m_Desc.SharedSystemMemory; }
		bool	IsSoftware() override { return m_Desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE; }
		bool	IsDedicate() override { return !IsSoftware(); }

		IDXGIAdapter1* GetAdapter() { return m_Adapter.Get(); }
	};

	class D3DGfxSystem final : public IGfxSystem
	{
		ComPtr<IDXGIFactory4>	m_Factory;
		TArray<D3DAdapter>		m_Adapters;
	public:
		D3DGfxSystem();
		~D3DGfxSystem();

		bool Init(GfxSystemDesc InDesc) override;
		bool ShutDown() override;

		uint32 AdapterNum() override { return m_Adapters.Num(); }
		IGfxAdapter* GetAdapter(uint32 Index) override { return &m_Adapters[Index]; }

		SP<IGfxDevice> CreateDevice(IGfxAdapter* InAdapter) override;
	};
}

// device 
namespace Fuko::Gfx
{
	class D3DDevice : public IGfxDevice
	{
		ComPtr<ID3D12Device>	m_Device;
		ComPtr<IDXGIFactory4>	m_Factory;
	public:
		D3DDevice(ComPtr<IDXGIFactory4> InFactory, ComPtr<ID3D12Device> InDevice);
		~D3DDevice();

		FORCEINLINE ID3D12Device* Device() { return m_Device.Get(); }
	};
}