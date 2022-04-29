#pragma once

#include "stdafx.h"
#include "GameTimer.h"
#include "MathHelper.h"
#include "UploadBuffer.h"
#include "FrameResource.h"
#include "AssetsManager.h"

using Microsoft::WRL::ComPtr;

class Mesh
{
public:
	Mesh() {};
	Mesh(MeshRead MeshData)//由源数据构建Mesh资源
	{
		MeshName = MeshData.MeshName;
		Vertices = MeshData.Vertices;
		Indices = MeshData.Indices;
	}
	std::string MeshName;
	std::vector<Vertex> Vertices;
	std::vector<uint16_t> Indices;
};



class DXMesh : public Mesh
{
public:
	DXMesh() {};
	DXMesh(MeshRead MeshData):Mesh(MeshData)
	{
		//Vertices Indices 大小
		IbByteSize = (UINT)MeshData.Indices.size() * sizeof(std::uint16_t);
		VbByteSize = (UINT)MeshData.Vertices.size() * sizeof(Vertex);
	};
	DXMesh(Mesh x)
	{
		MeshName = x.MeshName;
		Vertices = x.Vertices;
		Indices = x.Indices;

		IbByteSize = (UINT)x.Indices.size() * sizeof(std::uint16_t);
		VbByteSize = (UINT)x.Vertices.size() * sizeof(Vertex);
	};

	UINT VbByteSize;
	UINT IbByteSize;

	//Mesh
	ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
	ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;
	ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
	ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;
	ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
	ComPtr<ID3DBlob> IndexBufferCPU = nullptr;

	UINT VertexByteStride = sizeof(Vertex);

public:
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView()const
	{
		D3D12_VERTEX_BUFFER_VIEW Vbv;
		Vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
		Vbv.StrideInBytes = VertexByteStride;
		Vbv.SizeInBytes = VbByteSize;

		return Vbv;
	}

	D3D12_INDEX_BUFFER_VIEW IndexBufferView()const
	{
		D3D12_INDEX_BUFFER_VIEW Ibv;
		Ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
		Ibv.Format = DXGI_FORMAT_R16_UINT;
		Ibv.SizeInBytes = IbByteSize;

		return Ibv;
	}

	void DisposeUploaders()
	{
		VertexBufferUploader = nullptr;
		IndexBufferUploader = nullptr;
	}

	void BuildDefaultBuffer(ID3D12Device* D3dDevice, ID3D12GraphicsCommandList* CommandList)
	{
		D3DCreateBlob(VbByteSize, &VertexBufferCPU);
		D3DCreateBlob(IbByteSize, &IndexBufferCPU);

		CopyMemory(IndexBufferCPU->GetBufferPointer(), Indices.data(), IbByteSize);
		CopyMemory(VertexBufferCPU->GetBufferPointer(), Vertices.data(), VbByteSize);

		VertexBufferGPU = D3DUtil::CreateDefaultBuffer(D3dDevice, CommandList, Vertices.data(), VbByteSize, VertexBufferUploader);
		IndexBufferGPU = D3DUtil::CreateDefaultBuffer(D3dDevice, CommandList, Indices.data(), IbByteSize, IndexBufferUploader);
	}

};
