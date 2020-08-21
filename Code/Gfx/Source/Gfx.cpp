#include <Gfx.h>

#include "D3D12/D3DGfx.h"
namespace Fuko::Gfx
{
	GFX_API IGfxSystem* GetGfxSystem()
	{
		static D3DGfxSystem System;
		return &System;
	}
}