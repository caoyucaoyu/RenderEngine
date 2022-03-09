#pragma once

#include "stdafx.h"
#include "D3dUtil.h"
#include "GameTimer.h"
#include "MathHelper.h"
#include "UploadBuffer.h"
#include "FrameResource.h"

using Microsoft::WRL::ComPtr;

class Mesh
{
public:
	Mesh() {};
	Mesh(MapItem MeshData)
	{
		MeshName = MeshData.MeshName;
		Vertices = MeshData.Vertices;
		Indices = MeshData.Indices;
	}
	std::string MeshName;
	std::vector<Vertex> Vertices;
	std::vector<uint16_t> Indices;
};

class RenderMesh : public Mesh
{
public:
	RenderMesh() {};
	RenderMesh(MapItem MeshData):Mesh(MeshData)
	{
		//Vertices Indices ��С
		IbByteSize = (UINT)MeshData.Indices.size() * sizeof(std::uint16_t);
		VbByteSize = (UINT)MeshData.Vertices.size() * sizeof(Vertex);

		//�任����		----------------------------------------------------------------------------------------------------------------------------
		//Float3 S = MeshData.Scale3D; Float3 L = MeshData.Location; Float4 R = MeshData.Rotation;
		//DirectX::XMVECTORF32 g_XMIdentityR3 = { { { R.X, R.Y, R.Z, R.W } } };//auto x=DirectX::XMQuaternionIdentity();
		//auto Quat = DirectX::XMMatrixRotationQuaternion(g_XMIdentityR3);
		//XMStoreFloat4x4(&Location_Matrix, DirectX::XMMatrixTranslation(L.X, L.Y, L.Z));
		//XMStoreFloat4x4(&Rotation_Matrix, Quat);
		//XMStoreFloat4x4(&Scale3D_Matrix, DirectX::XMMatrixScaling(S.X, S.Y, S.Z));
		//XMStoreFloat4x4(&World_Matrix, Quat * DirectX::XMMatrixScaling(S.X, S.Y, S.Z) * DirectX::XMMatrixTranslation(L.X, L.Y, L.Z));			
	};
	RenderMesh(Mesh x)
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




	//	----------------------------------------------------------------------------------------------------------------------------
	//DirectX::XMFLOAT4X4 World_Matrix = MathHelper::Identity4x4();
	//DirectX::XMFLOAT4X4 Location_Matrix = MathHelper::Identity4x4();
	//DirectX::XMFLOAT4X4 Rotation_Matrix = MathHelper::Identity4x4();
	//DirectX::XMFLOAT4X4 Scale3D_Matrix = MathHelper::Identity4x4();
};
