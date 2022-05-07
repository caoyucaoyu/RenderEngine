#pragma once
#include "GPUMeshBuffer.h"

class DX12GPUMeshBuffer : public GPUMeshBuffer
{
public:
	DX12GPUMeshBuffer() {};
	~DX12GPUMeshBuffer() {};

	//由源数据构建Mesh资源
	DX12GPUMeshBuffer(MeshRead MeshData):GPUMeshBuffer(MeshData)
	{
		/*Vertices = MeshData.Vertices;
		Indices = MeshData.Indices;
		VbByteSize = (UINT)MeshData.Vertices.size() * sizeof(Vertex);
		IbByteSize = (UINT)MeshData.Indices.size() * sizeof(std::uint16_t);*/
	}
	void SetData(StaticMesh StaticMeshData)
	{
		Vertices = StaticMeshData.Vertices;
		Indices = StaticMeshData.Indices;
		VbByteSize = (UINT)StaticMeshData.Vertices.size() * sizeof(Vertex);
		IbByteSize = (UINT)StaticMeshData.Indices.size() * sizeof(std::uint16_t);
	};

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



