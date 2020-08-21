#pragma once
#include "GfxConfig.h"
#include "Enums.h"
#include "DeviceChild.h"

// enum & struct 
namespace Fuko::Gfx
{
	enum class EResType
	{
		BUFFER ,
		CONSTANT_BUF ,
		STRUCTED_BUF ,
		TEXTURE_1D ,
		TEXTURE_2D ,
		TEXTURE_3D ,
		TEXTURE_CUBE ,
	};

	enum class EResLocation
	{
		GPU ,
		UPLOAD ,
		READBACK ,
	};

	enum EResUsageFlag
	{
		NONE = 0 ,
		SHADER_RES			= 1 << 0 ,
		CONSTANT_BUF		= 1 << 1 ,
		UNORDERED_ACCESS	= 1 << 2 ,
		RENDER_TARGET		= 1 << 3 ,
		DEPTH_STENCIL		= 1 << 4 ,
		VERTEX_BUFFER		= 1 << 5 ,
		INDEX_BUFFER		= 1 << 6 ,
		STREAM_OUTPUT		= 1 << 7 ,
	};

	struct ResourceDesc
	{
		uint32			Width;
		uint32			Heigh;
		uint16			Depth;
		uint16			MipLevel;
		EResType		Type;
		EPixelFmt		Format;
		EResLocation	Location;
		uint32			UsageFlag;
		uint32			SampleCount;
		uint32			SampleQuality;

		FORCEINLINE void AsBuffer(uint32 InSize, EResLocation InLocation, uint32 InUsage)
		{
			Width = InSize;
			Heigh = Depth = 0;
			MipLevel = 1;
			Type = EResType::BUFFER;
			Format = EPixelFmt::UNKNOWN;
			Location = InLocation;
			UsageFlag = InUsage;
			SampleCount = 1;
			SampleQuality = 0;
		}

		FORCEINLINE void NoMutiSample()
		{
			SampleCount = 1;
			SampleQuality = 0;
		}
	};

	struct IGfxDevice;

}

// resource 
namespace Fuko::Gfx
{
	struct GPUResource : public DeviceChild
	{
		GPUResource(IGfxDevice* InDevice) : DeviceChild(InDevice) {}
		virtual ~GPUResource() {}

		SP<IGfxDevice>	Device() { return m_Device; }
		EResType		Type() { return m_Desc.Type; }

		virtual void*	Map(uint32 InSubResIndex) = 0;
		virtual bool	Unmap(uint32 InSubResIndex) = 0;
		virtual void*	MapRange(uint32 InSubResIndex, size_t InBegin, size_t InEnd) = 0;
		virtual bool	UnmapRange(uint32 InSubResIndex, size_t InBegin, size_t InEnd) = 0;

		virtual	bool	Write(uint32 InSubResIndex, const void* InSrc,
			uint32 InRowPitch, uint32 InDepthPitch, const Box<uint32>& DestBox) = 0;
		virtual bool	Read(uint32 InSubResIndex, void* InDst,
			uint32 InRowPitch, uint32 InDepthPitch, const Box<uint32>& SrcBox) = 0;

	protected:
		ResourceDesc	m_Desc;
	};
}
