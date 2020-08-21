#include "D3DGfx.h"
#include "D3DGfx.h"
#include "D3DGfx.h"
#include "D3DGfx.h"

// adapter 
namespace Fuko::Gfx
{
	D3DAdapter::D3DAdapter(ComPtr<IDXGIAdapter1> InAdapter)
		: m_Adapter(InAdapter)
	{
		always_check(m_Adapter.Get() != nullptr);

		m_Adapter->GetDesc1(&m_Desc);
	}
}

// Gfx system 
namespace Fuko::Gfx
{
	D3DGfxSystem::D3DGfxSystem()
		: m_Adapters(8)
		, m_Factory()
	{ }

	D3DGfxSystem::~D3DGfxSystem()
	{
		ShutDown();
	}

	bool D3DGfxSystem::Init(GfxSystemDesc InDesc)
	{
		// Enable debug layer 
		if (InDesc.EnableDebugLayer)
		{
			ComPtr<ID3D12Debug> debugController;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
			{
				debugController->EnableDebugLayer();
			}
			else
			{
				F_LOG(Error, LogGfx, TSTR("Unable to enable D3D12 debug validation layer"));
				return false;
			}
		}

		// Create Factory 
		{
			if (FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&m_Factory))))
			{
				F_LOG(Error, LogGfx, TSTR("Unable to create DXGI Factory"));
			}
		}

		// Collect Adapters 
		{
			ComPtr<IDXGIAdapter1>	pAdapter;

			for (uint32_t Idx = 0; DXGI_ERROR_NOT_FOUND != m_Factory->EnumAdapters1(Idx, &pAdapter); ++Idx)
			{
				m_Adapters.Emplace(pAdapter);
			}
		}
		return true;
	}

	bool D3DGfxSystem::ShutDown()
	{
		m_Adapters.Empty();
		m_Factory.Reset();
		return true;
	}

	SP<IGfxDevice> D3DGfxSystem::CreateDevice(IGfxAdapter * InAdapter)
	{
		ComPtr<ID3D12Device> Device;
		D3DAdapter* DxAdapter = (D3DAdapter*)InAdapter;

		if (FAILED(D3D12CreateDevice(DxAdapter->GetAdapter(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&Device))))
		{
			F_LOG(Error, LogGfx, TSTR("Unable to create Device"));
			return SP<IGfxDevice>();
		}
		return MakeSP<D3DDevice>(m_Factory, Device);
	}
}

// Gfx device 
namespace Fuko::Gfx
{
	D3DDevice::D3DDevice(ComPtr<IDXGIFactory4> InFactory, ComPtr<ID3D12Device> InDevice)
		: m_Factory(InFactory)
		, m_Device(InDevice)
	{
	}

	D3DDevice::~D3DDevice()
	{
	}

}
