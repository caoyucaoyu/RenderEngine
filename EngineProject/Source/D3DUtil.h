#pragma once

#include "stdafx.h"

#include <windowsx.h>
#include <comdef.h>
#include "MathHelper.h"
#include <DirectXMath.h>

using Microsoft::WRL::ComPtr;

struct Float3
{
	float X;
	float Y;
	float Z;
};

struct Float4
{
	float X;
	float Y;
	float Z;
	float W;
};

struct Vertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT4 Color;
	DirectX::XMFLOAT4 Normal;
};

struct MeshInfo
{
	std::string MeshName;
	Float3 Location;
	Float4 Rotation;
	Float3 Scale3D;
	std::vector<Vertex> Vertices;
	std::vector<uint16_t> Indices;
};

struct ObjectConstants
{
	DirectX::XMFLOAT4X4 Location_M = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 Rotation_M = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 Scale3D_M = MathHelper::Identity4x4();
	//glm::mat4 World_M = MathHelper::GIdentity4x4();
};

struct PassConstants
{
	DirectX::XMFLOAT4X4 ViewProj_M = MathHelper::Identity4x4();
	float Time;
	//glm::mat4 ViewProj_M = MathHelper::GIdentity4x4();
};

struct TimeConstants
{
	float Time;
};


class D3DUtil
{
public:
	static UINT CalcConstantBufferByteSize(UINT byteSize)
	{
		return (byteSize + 255) & ~255;//..
	}

	static void ReadMeshFile(const std::string& filePath, std::vector<Vertex>& outVertexBuffer, std::vector<uint16_t>& outIndexBuffer)
	{

		std::ifstream readFile;
		readFile.open(filePath, std::ios::in | std::ios::binary);
		if (!readFile)
		{
			OutputDebugStringA("Error opening file");
			return;
		}

		int fileHeadSize = 4;
		int fileHeadByteSize = fileHeadSize * sizeof(int);
		std::vector<int>fileHead;
		fileHead.resize(fileHeadSize);

		std::vector<char>fileHeadBuffer;
		fileHeadBuffer.resize(fileHeadByteSize);

		readFile.read(fileHeadBuffer.data(), fileHeadByteSize);
		memcpy(fileHead.data(), fileHeadBuffer.data(), fileHeadByteSize);

		int vbSize = fileHead[0];
		int vbByteSize = fileHead[1];
		int ibSize = fileHead[2];
		int ibByteSize = fileHead[3];

		std::vector<char>fullFileBuffer;
		fullFileBuffer.resize(fileHeadByteSize + vbByteSize + ibByteSize);

		readFile.clear();

		readFile.seekg(std::ios::beg);
		readFile.read(fullFileBuffer.data(), fileHeadByteSize + vbByteSize + ibByteSize);

		struct Vertex3
		{
			float X;
			float Y;
			float Z;
		};

		std::vector<Vertex3> Vb;
		std::vector<int> Ib;

		Vb.resize(vbSize);
		Ib.resize(ibSize);

		memcpy(Vb.data(), fullFileBuffer.data() + fileHeadByteSize, vbByteSize);
		memcpy(Ib.data(), fullFileBuffer.data() + fileHeadByteSize + vbByteSize, ibByteSize);

		for (const auto& ib : Ib)
		{
			uint16_t i = ib;
			outIndexBuffer.push_back(i);
		}

		for (int i = 0; i < Vb.size(); i++)
		{
			Vertex3 vb = Vb[i];
			Vertex v;
			v.Pos.x = vb.X;
			v.Pos.y = vb.Y;
			v.Pos.z = vb.Z;
			v.Color = DirectX::XMFLOAT4(float(i + 1) / Vb.size() * 2, float(i + 1) / Vb.size(), 0, 1.0f);
			outVertexBuffer.push_back(v);
		}

		readFile.close();
	}

	static void ReadMapFile(const std::string& filePath, std::vector<MeshInfo>& MeshData)
	{
		std::ifstream readFile;
		readFile.open(filePath, std::ios::in | std::ios::binary);
		if (!readFile)
		{
			OutputDebugStringA("Error opening file When ReadFile \n");
			return;
		}
		OutputDebugStringA("Opening file When ReadFile \n");


		int Num;
		readFile.read((char*)&Num, sizeof(int));

		std::stringstream ss;
		ss << Num << "\n";

		for (int j = 0; j < Num; j++)
		{
			MeshInfo Mesh;

			//Transform
			readFile.read((char*)&Mesh.Location, sizeof(Float3));
			readFile.read((char*)&Mesh.Rotation, sizeof(Float4));
			readFile.read((char*)&Mesh.Scale3D, sizeof(Float3));
			//ss<< Mesh.Rotation.X << "   " << Mesh.Rotation.Y << "   " << Mesh.Rotation.Z << "   "<< Mesh.Rotation.W<<"\n";

			//Vertex
			int VCount;
			readFile.read((char*)&VCount, sizeof(int));

			for (int i = 0; i < VCount; i++)
			{
				Float3 VertexPosition;
				readFile.read((char*)&VertexPosition, sizeof(Float3));
				Float4 Normal;
				readFile.read((char*)&Normal, sizeof(Float4));
				Vertex Vert;
				Vert.Pos.x = VertexPosition.X; Vert.Pos.y = VertexPosition.Y; Vert.Pos.z = VertexPosition.Z;
				//Vert.Color = DirectX::XMFLOAT4(float(i + 1) / VCount * 2, float(i + 1) / VCount, 0, 1.0f);
				Vert.Normal.x = Normal.X; Vert.Normal.y = Normal.Y; Vert.Normal.z = Normal.Z; Vert.Normal.w = Normal.W;
				Vert.Color = DirectX::XMFLOAT4(Normal.X,Normal.Y,Normal.Z,1.0f);

				Mesh.Vertices.push_back(Vert);				
			}

			//Index
			int ICount;
			readFile.read((char*)&ICount, sizeof(int));
			for (int i = 0; i < ICount; i++)
			{
				int Ind;
				readFile.read((char*)&Ind, sizeof(int));
				uint16_t in = Ind;
				Mesh.Indices.push_back(in);
				//ss<<in<<" ";
			}

			MeshData.push_back(Mesh);
		}

		OutputDebugStringA(ss.str().c_str());
		readFile.close();
	}

	static ComPtr<ID3D12Resource> CreateDefaultBuffer(ID3D12Device* Device, ID3D12GraphicsCommandList* CmdList, const void* InitData, UINT64 ByteSize, ComPtr<ID3D12Resource>& UploadBuffer)
	{
		ComPtr<ID3D12Resource> DefaultBuffer;

		//Create Default Buffer
		Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(ByteSize),
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(DefaultBuffer.GetAddressOf()));

		//Create Upload Heap,  for Copy CPU into Default Buffer
		Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(ByteSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(UploadBuffer.GetAddressOf()));

		D3D12_SUBRESOURCE_DATA SubResourceData = {};
		SubResourceData.pData = InitData;
		SubResourceData.RowPitch = ByteSize;
		SubResourceData.SlicePitch = SubResourceData.RowPitch;

		CmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(DefaultBuffer.Get(),
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_COPY_DEST));
		UpdateSubresources<1>(CmdList, DefaultBuffer.Get(), UploadBuffer.Get(), 0, 0, 1, &SubResourceData);
		CmdList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(DefaultBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

		return DefaultBuffer;
	}

	static ComPtr<ID3DBlob> CompileShader(const std::wstring& Filename, const D3D_SHADER_MACRO* Defines, const std::string& EntryPoint, const std::string& Target)
	{
		ComPtr<ID3DBlob> ByteCode;
		ComPtr<ID3DBlob> Errors;
		D3DCompileFromFile(Filename.c_str(), Defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, EntryPoint.c_str(), Target.c_str(), 0, 0, &ByteCode, &Errors);
		return ByteCode;
	}
};


inline std::wstring AnsiToWString(const std::string& str)
{
	WCHAR buffer[512];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
	return std::wstring(buffer);
}

class DxException
{
public:
	DxException() = default;
	DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);

	std::wstring ToString()const;

	HRESULT ErrorCode = S_OK;
	std::wstring FunctionName;
	std::wstring Filename;
	int LineNumber = -1;
};

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                              \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = AnsiToWString(__FILE__);                       \
    if(FAILED(hr__)) { throw DxException(hr__, L#x, wfn, __LINE__); } \
}
#endif
