#pragma once
#include "GfxConfig.h"

namespace Fuko::Gfx
{
	// pixel format 
	enum class EPixelFmt
	{
		// No Format 
		UNKNOWN ,

		// RGBA 32
		RGBA32_TYPELESS ,
		RGBA32_FLOAT ,
		RGBA32_UINT ,
		RGBA32_SINT ,

		// RGB 32 
		RGB32_TYPELESS ,
		RGB32_FLOAT ,
		RGB32_UINT ,
		RGB32_SINT ,

		// RGBA 16 
		RGBA16_TYPELESS ,
		RGBA16_FLOAT ,
		RGBA16_UNORM ,
		RGBA16_UINT ,
		RGBA16_SNORM ,
		RGBA16_SINT ,

		// RG 32 
		RG32_TYPELESS ,
		RG32_FLOAT ,
		RG32_UINT ,
		RG32_SINT ,

		// Misc with useless 
		R32G8X24_TYPELESS ,
		D32_FLOAT_S8X24_UINT ,
		R32_FLOAT_X8X24_TYPELESS ,
		X32_TYPELESS_G8X24_UINT ,
		
		// Misc on byte 
		R10G10B10A2_TYPELESS ,
		R10G10B10A2_UNORM ,
		R10G10B10A2_UINT ,
		R11G11B10_FLOAT ,

		// RGBA 8
		RGBA8_TYPELESS ,
		RGBA8_UNORM ,
		RGBA8_UNORM_SRGB ,
		RGBA8_UINT ,
		RGBA8_SNORM ,
		RGBA8_SINT ,

		// RG 16 
		RG16_TYPELESS ,
		RG16_FLOAT ,
		RG16_UNORM ,
		RG16_UINT ,
		RG16_SNORM ,
		RG16_SINT ,

		// R32 
		R32_TYPELESS ,
		D32_FLOAT ,
		R32_FLOAT ,
		R32_UINT ,
		R32_SINT ,

		// R24 
		R24G8_TYPELESS ,
		D24_UNORM_S8_UINT ,
		R24_UNORM_X8_TYPELESS ,
		X24_TYPELESS_G8_UINT ,

		// RG 8 
		RG8_TYPELESS ,
		RG8_UNORM ,
		RG8_UINT ,
		RG8_SNORM ,
		RG8_SINT ,

		// R 16
		R16_TYPELESS ,
		R16_FLOAT ,
		D16_UNORM ,
		R16_UNORM ,
		R16_UINT ,
		R16_SNORM ,
		R16_SINT ,

		// R 8
		R8_TYPELESS ,
		R8_UNORM ,
		R8_UINT ,
		R8_SNORM ,
		R8_SINT ,

		// Other 
		A8_UNORM ,
		R1_UNORM ,
		R9G9B9E5_SHAREDEXP ,
		R8G8_B8G8_UNORM ,
		G8R8_G8B8_UNORM ,
	};

	// blend step operator 
	enum class EBlendFactor
	{
		ZERO ,				// Out = 0
		ONE ,				// Out = In
		SRC_COLOR ,			// Out = In * Src
		INV_SRC_COLOR ,		// Out = In * (1 - Src)
		SRC_ALPHA ,			// Out = In * (Src.A)
		INV_SRC_ALPHA ,		// Out = In * (1 - Src.A)
		DEST_ALPHA ,		// Out = In * Dst.A
		INV_DEST_ALPHA ,	// Out = In * (1 - Dst.A)
		DEST_COLOR ,		// Out = In * Dst
		INV_DEST_COLOR ,	// Out = In * (1 - Dst)
		SRC_ALPHA_SAT ,		// Out = In * float4(min(Src.A, 1 - Dst.A).rrr,1)
		BLEND_FACTOR ,		// Set by graphics api
		INV_BLEND_FACTOR ,	// 1 - (Op set by graphics api) 

		// dual-source color blending 
		SRC1_COLOR ,		// Out = In * Src
		INV_SRC1_COLOR ,	// Out = In * (1 - Src)
		SRC1_ALPHA,			// Out = In * Src.A 
		INV_SRC1_ALPHA		// Out = In * (1 - Src.A) 
	};
	enum class EBlendOp
	{
		ADD ,			// Out = A + B 
		SUBTRACT ,		// Out = B - A
		REV_SUBTRACT ,	// Out = A - B
		MIN ,			// Out = min(A, B) 
		MAX ,			// Out = max(A, B)
	};
	enum class ELogicOp
	{
		CLEAR ,			// Out = 0
		SET ,			// Out = 1
		COPY ,			// Out = Src 
		COPY_INVERTED ,	// Out = ~Src
		NOOP ,			// Out = Dst
		INVERT ,		// Out = !Dst
		AND ,			// Out = Src & Dst
		NAND ,			// Out = ~(Src & Dst)
		OR ,			// Out = Src | Dst
		NOR ,			// Out = ~(Src | Dst)
		XOR ,			// Out = Src ^ Dst
		EQUIV ,			// Out = ~(Src ^ Dst)
		AND_REVERSE ,	// Out = Src & ~Dst
		AND_INVERTED ,	// Out = ~Src & Dst
		OR_REVERSE ,	// Out = Src | ~Dst
		OR_INVERTED		// Out = ~Src | Dst
	};

	// primitive Topology 
	enum class EPrimitiveTopology
	{
		POINT_LIST ,
		LINE_LIST ,
		LINE_STRIP ,
		TRIANGLE_LIST ,
		TRIANGLE_STRIP ,
		LINE_LIST_ADJACENCY ,
		LINE_STRIP_ADJACENCY,
		TRIANGLE_LIST_ADJACENCY,
		TRIANGLE_STRIP_ADJACENCY,
		PATCH_LIST,
	};

	// rasterize 
	enum class EFillMode
	{
		WIREFRAME ,
		SOLID 
	};
	enum class ECullMode
	{
		NONE ,
		FRONT ,
		BACK 
	};

	// sample 
	enum class EFilter
	{
		POINT ,
		LINER ,
		ANISOTROPIC ,
	};
	enum class EAddrMode
	{
		WRAP ,
		MIRROR ,
		CLAMP ,
		BORDER ,
		MIRROR_ONCE 

	};
	enum class ECompareOp
	{
		NEVER ,
		LESS ,
		EQUAL ,
		LESS_EQUAL ,
		GREATER ,
		NOT_EQUAL ,
		GREATER_EQUAL ,
		ALWAYS 
	};

	// stencil op 
	enum class EStencilOp
	{
		KEEP,		// Out = Dst
		ZERO,		// Out = 0
		REPLACE,	// Out = Src (Src is a const value set from api)
		INCR_SAT,	// Out = Clamp(Dst + 1, 0, MaxStencil)
		DECR_SAT,	// Out = Clamp(Dst - 1, 0, MaxStencil)
		INVERT,		// Out = MaxStencil - Dst
		INCR,		// Out = Dst + 1
		DECR		// Out = Dst - 1
	};

	// index buffer type 
	enum class EIndexBufType
	{
		UINT16 ,
		UINT32 ,
	};
}