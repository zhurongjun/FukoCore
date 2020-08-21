#pragma once
#include "D3D12Resources.h"
#include "D3DGfx.h"

// Texture 
namespace Fuko::Gfx
{
	D3DResource::D3DResource(D3DDevice* InDevice)
		: GPUResource(InDevice)
	{
	}

	D3DResource::~D3DResource()
	{
		if (m_Resource)
		{
			m_Resource->Release();
			m_Resource = nullptr;
		}
	}

	void* D3DResource::Map(uint32 InSubResIndex)
	{
		void* Data;
		if (FAILED(m_Resource->Map(InSubResIndex, nullptr, &Data)))
		{
			return nullptr;
		}
		return Data;
	}

	bool D3DResource::Unmap(uint32 InSubResIndex)
	{
		m_Resource->Unmap(InSubResIndex, nullptr);
		return true;
	}

	void* D3DResource::MapRange(uint32 InSubResIndex, size_t InBegin, size_t InEnd)
	{
		D3D12_RANGE Range = { InBegin,InEnd };
		void* Data;
		if (FAILED(m_Resource->Map(InSubResIndex, &Range, &Data)))
		{
			return nullptr;
		}
		return Data;
	}

	bool D3DResource::UnmapRange(uint32 InSubResIndex, size_t InBegin, size_t InEnd)
	{
		D3D12_RANGE Range = { InBegin,InEnd };
		m_Resource->Unmap(InSubResIndex, &Range);
		return true;
	}

	bool D3DResource::Write(uint32 InSubResIndex, const void* InSrc, uint32 InRowPitch, uint32 InDepthPitch, const Box<uint32>& DestBox)
	{
		D3D12_BOX Box;
		Box.left = DestBox.Left;
		Box.right = DestBox.Right;
		Box.top = DestBox.Top;
		Box.bottom = DestBox.Bottom;
		Box.front = DestBox.Front;
		Box.back = DestBox.Back;
		m_Resource->WriteToSubresource(InSubResIndex, &Box, InSrc, InRowPitch, InDepthPitch);
		return true;
	}

	bool D3DResource::Read(uint32 InSubResIndex, void* InDst, uint32 InRowPitch, uint32 InDepthPitch, const Box<uint32>& SrcBox)
	{
		D3D12_BOX Box;
		Box.left = SrcBox.Left;
		Box.right = SrcBox.Right;
		Box.top = SrcBox.Top;
		Box.bottom = SrcBox.Bottom;
		Box.front = SrcBox.Front;
		Box.back = SrcBox.Back;
		m_Resource->ReadFromSubresource(InDst, InRowPitch, InDepthPitch, InSubResIndex, &Box);
		return true;
	}

}