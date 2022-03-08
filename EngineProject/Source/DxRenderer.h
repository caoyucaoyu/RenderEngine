#pragma once
#include "FrameResource.h"
#include "Mesh.h"
#include "Camera.h"
#include <dxgi.h>

using Microsoft::WRL::ComPtr;
using DirectX::XMConvertToRadians;
const int FrameResourcesCount = 3;

class DxRenderer
{
public:
	DxRenderer();
	~DxRenderer(){};
	void Run();

	bool Init();
	bool InitDirect3D();
	bool InitDraw();
	void Update(const GameTimer& Gt);
	void Draw();
	void CreateDevice();
	void CreateFence();
	void GetDescriptorSize();
	void SetMSAA();
	void CreateCommandObject();
	void CreateSwapChain();
	void CreateDescriptorHeap();
	void CreateRTV();
	void CreateDSV();
	void FlushCommandQueue();
	void CreateViewPortAndScissorRect();
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView();
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView();


	void BuildFrameResource();
	void BuildDescriptorHeaps();
	void BuildConstantBuffers();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildGeometry();
	void BuildPSO();



protected:
	ComPtr<IDXGIFactory> DxgiFactory;
	ComPtr<ID3D12Device> D3dDevice;
	ComPtr<ID3D12Fence> Fence;
	ComPtr<ID3D12CommandQueue> CommandQueue;
	ComPtr<ID3D12GraphicsCommandList> CommandList;
	ComPtr<ID3D12CommandAllocator> CommandListAlloc;
	ComPtr<IDXGISwapChain> SwapChain;
	ComPtr<ID3D12DescriptorHeap> DsvHeap;
	ComPtr<ID3D12DescriptorHeap> RtvHeap;
	static const int SwapChainBufferCount = 2;
	ComPtr<ID3D12Resource> SwapChainBuffers[2];//SwapChainBufferCount
	ComPtr<ID3D12Resource> DepthStencilBuffer;


	D3D12_VIEWPORT ScreenViewport;
	D3D12_RECT ScissorRect;

	UINT RtvDescriptorSize;
	UINT DsvDescriptorSize;
	UINT CbvSrvUavDescriptorSize;

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS MsaaQualityLevels;
	int ClientWidth = 1280;
	int ClientHight = 720;
	HWND HMainWnd;

	UINT64 CurrentFence = 0;
	int CurrBackBuffer = 0;


	std::vector<std::unique_ptr<FrameResource>> FrameResources;
	int DescriptorsNum = 0;
	ComPtr<ID3D12DescriptorHeap> CbvHeap;
	ComPtr<ID3D12RootSignature> RootSignature;
	ComPtr<ID3DBlob> MvsByteCode[2];
	ComPtr<ID3DBlob> MpsByteCode[2];
	std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;
	std::vector<ComPtr<ID3D12PipelineState>> PSOs;

	FrameResource* CurrFrameResource = nullptr;
	int CurrFrameResourceIndex = 0;

private:
	int DrawCount = 0;
	std::vector<std::shared_ptr<MeshGeometry>> DrawList;
};

