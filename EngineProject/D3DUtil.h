#pragma once
#include "Head.h"
#include <windowsx.h>
#include <comdef.h>
#include "MathHelper.h"
#include <DirectXMath.h>

using namespace DirectX;
struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
	//Colors::Black
};

struct ObjectConstants
{
	XMFLOAT4X4 WorldViewProj = MathHelper::Identity4x4();
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

		for(const auto& ib : Ib)
		{
			uint16_t i=ib;
			outIndexBuffer.push_back(i);
		}

		for (int i=0;i<Vb.size();i++)
		{
			Vertex3 vb=Vb[i];
			Vertex v;
			v.Pos.x = vb.X;
			v.Pos.y = vb.Y;
			v.Pos.z = vb.Z;
			v.Color = XMFLOAT4(float(i+1)/ Vb.size()*2, float(i+1) / Vb.size(), 0,1.0f);
			outVertexBuffer.push_back(v);
		}

		readFile.close();
	}
};

