#pragma once
#include "GfxConfig.h"
#include "SwapChain.h"

// Gfx system 
namespace Fuko::Gfx 
{
	struct IGfxDevice;

	struct GFX_API IGfxAdapter
	{
		virtual ~IGfxAdapter() {}

		virtual const TCHAR* Name() = 0;
		virtual uint64	LocalMemory() = 0;
		virtual uint64	SharedMemory() = 0;
		virtual bool	IsSoftware() = 0;
		virtual bool	IsDedicate() = 0;
	};

	struct GFX_API GfxSystemDesc
	{
		bool	EnableDebugLayer;
	};
	struct GFX_API IGfxSystem
	{
		virtual ~IGfxSystem() {}

		virtual bool Init(GfxSystemDesc InDesc) = 0;
		virtual bool ShutDown() = 0;

		virtual uint32 AdapterNum() = 0;
		virtual IGfxAdapter* GetAdapter(uint32 Index) = 0;

		virtual SP<IGfxDevice> CreateDevice(IGfxAdapter* InAdapter) = 0;
	};

	GFX_API IGfxSystem* GetGfxSystem();
}

// Gfx device 
namespace Fuko::Gfx 
{
	struct GFX_API IGfxDevice
	{
		virtual ~IGfxDevice() {}

		// Create Allocator 
		// Create DescHeap 
		// Create Resource 
		// Create PSO 
		// Create CmdQueue 
		// Create CmdBuffer 
		// Create SwapChain 
	};
}
