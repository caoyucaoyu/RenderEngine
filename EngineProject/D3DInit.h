#pragma once
#include "D3DApp.h"
#include "MathHelper.h"
#include "UploadBuffer.h"
#include "FrameResource.h"

const int NumFrameResources = 3;

struct MeshGeometry
{
	//Mesh
	ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
	ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;
	ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
	ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;
	ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
	ComPtr<ID3DBlob> IndexBufferCPU = nullptr;

	UINT VertexByteStride = sizeof(Vertex);
	UINT VertexBufferByteSize = 0;//sizeof(Vertex) * 8;
	UINT IndexBufferByteSize = 0;

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView()const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
		vbv.StrideInBytes = VertexByteStride;
		vbv.SizeInBytes = VertexBufferByteSize;

		return vbv;
	}

	D3D12_INDEX_BUFFER_VIEW IndexBufferView()const
	{
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
		ibv.Format = DXGI_FORMAT_R16_UINT;
		ibv.SizeInBytes = IndexBufferByteSize;

		return ibv;
	}
};


class D3DInit : public D3DApp
{
public:
	D3DInit();
	~D3DInit();
	virtual bool Init(HINSTANCE hInstance) override;
	virtual void Draw()override;
	virtual void Update()override;
	virtual void OnResize()override;
protected:
	void BuildFrameResource();
	void BuildDescriptorHeaps();
	void BuildConstantBuffers();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildGeometry();
	void BuildPSO();

private:
	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y) override;
	void UpdateCamera();

private:
	ComPtr<ID3D12DescriptorHeap> CbvHeap;
	std::unique_ptr<UploadBuffer<ObjectConstants>> ObjectCB = nullptr;
	ComPtr<ID3D12RootSignature> RootSignature = nullptr;
	ComPtr<ID3DBlob> MvsByteCode = nullptr;
	ComPtr<ID3DBlob> MpsByteCode = nullptr;
	std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;
	ComPtr<ID3D12PipelineState> PSO = nullptr;

	XMFLOAT4X4 World = MathHelper::Identity4x4();
	XMFLOAT4X4 View = MathHelper::Identity4x4();
	XMFLOAT4X4 Proj_Matrix = MathHelper::Identity4x4();

	float Theta = 1.5f * XM_PI;
	float Phi = XM_PIDIV4;
	float Radius = 500.0f;

	POINT LastMousePos;

	XMFLOAT3 EyePos = {0.0f,0.0f,0.0f};
	XMFLOAT4X4 View_Matrix = MathHelper::Identity4x4();

	std::vector<std::unique_ptr<FrameResource>> FrameResources;
	FrameResource* CurrFrameResource = nullptr;
	int CurrFrameResourceIndex = 0;

private:
	//Util
	ComPtr<ID3D12Resource> CreateDefaultBuffer(ID3D12Device* Device, ID3D12GraphicsCommandList* CmdList, const void* InitData, UINT64 ByteSize, ComPtr<ID3D12Resource>& UploadBuffer);
	//Util
	ComPtr<ID3DBlob> CompileShader(const std::wstring& Filename, const D3D_SHADER_MACRO* Defines, const std::string& EntryPoint, const std::string& Target);

	std::unique_ptr<MeshGeometry> BoxGeo = nullptr;

	UINT VbByteSize; 
	UINT IbByteSize; 
	vector<Vertex> outVertexBuffer;
	vector<uint16_t> outIndexBuffer;

};
