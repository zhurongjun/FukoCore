#pragma once
#include "D3D12Config.h"
#include <Resources.h>

// resource  
namespace Fuko::Gfx
{
	class D3DDevice;
	class D3DResource : public GPUResource
	{
	protected:
		D3DResource(D3DDevice* InDevice);
		~D3DResource();

		// Disable copy
		D3DResource(const D3DResource& other) = delete;
		D3DResource(const D3DResource&& other) = delete;
		D3DResource& operator=(D3DResource& other) = delete;

		// Build & Destroy 



		// GPUResource API
		void*	Map(uint32 InSubResIndex) override;
		bool	Unmap(uint32 InSubResIndex) override;
		void*	MapRange(uint32 InSubResIndex, size_t InBegin, size_t InEnd) override;
		bool	UnmapRange(uint32 InSubResIndex, size_t InBegin, size_t InEnd) override;

		bool	Write(uint32 InSubResIndex, const void* InSrc,
			uint32 InRowPitch, uint32 InDepthPitch, const Box<uint32>& DestBox) override;
		bool	Read(uint32 InSubResIndex, void* InDst,
			uint32 InRowPitch, uint32 InDepthPitch, const Box<uint32>& SrcBox) override;

	protected:
		ID3D12Resource*			m_Resource;
	};
}