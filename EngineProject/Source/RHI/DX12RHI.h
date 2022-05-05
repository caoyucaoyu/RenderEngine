#pragma once
#include "RHI.h"

using Microsoft::WRL::ComPtr;

class Window;
class DescriptorHeap;
class FrameResource;
const int FrameResourcesCount = 3;

class DX12RHI : public RHI
{
public:
	DX12RHI();
	~DX12RHI();

public:
	virtual void Init()override;

	virtual void BeginFrame() override;
	virtual void EndFrame() override;

	virtual void CreateFrameResource() override;
	virtual void UpdateFrameResource() override;

	virtual void FlushCommandQueue()override;
	virtual void DrawInstanced(UINT DrawIndexCount) override;
	virtual void ResizeWindow(UINT32 Width, UINT32 Height) override;

	virtual void SetRenderTargetBegin() override;
	virtual void SetRenderTargetEnd() override;

	virtual void SetGraphicsPipeline() override;//...........................................................
	virtual void IASetMeshBuffer(GPUMeshBuffer* GPUMeshbuffer) override;

public:
	void ExecuteCommandList();//将待执行的命令列表加入GPU的命令队列
	void ResetCommandList(ComPtr<ID3D12CommandAllocator> CommandListAllocator);

private:
	void PrepareBufferHeap();


private:
	void CreateDevice();
	void CreateFence();
	void GetDescriptorSize();
	void SetMSAA();
	void CreateCommandObject();
	void CreateSwapChain();

	void CreateRtvAndDsvDescriptorHeaps();
	void CreateRTV();
	void CreateDSV();
	void CreateViewPortAndScissorRect();

	void CreatCbvSrvUavHeap();
	
	//只读
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

private:
	Window* Wd;
	ComPtr<IDXGIFactory> DxgiFactory;
	ComPtr<ID3D12Device> D3dDevice;
	ComPtr<ID3D12Fence> Fence;
	ComPtr<ID3D12CommandQueue> CommandQueue;
	ComPtr<ID3D12GraphicsCommandList> CommandList;
	ComPtr<ID3D12CommandAllocator> CommandListAllocator;
	ComPtr<IDXGISwapChain> SwapChain;
	UINT RtvDescriptorSize;
	UINT DsvDescriptorSize;
	UINT CbvSrvUavDescriptorSize;
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS MsaaQualityLevels;
	HWND HMainWnd;
	static const int SwapChainBufferCount = 2;
	ComPtr<ID3D12Resource> SwapChainBuffers[2];
	ComPtr<ID3D12Resource> DepthStencilBuffer;

	std::unique_ptr<DescriptorHeap> RtvHeap;
	std::unique_ptr<DescriptorHeap> DsvHeap;
	std::unique_ptr<DescriptorHeap> CbvSrvUavHeap;

	UINT64 CurrentFence = 0;
	UINT64 CurrBackBuffer = 0;

	D3D12_VIEWPORT ScreenViewport;
	D3D12_RECT ScissorRect;

	std::vector<std::unique_ptr<FrameResource>> FrameResources;
	FrameResource* CurrFrameResource = nullptr;
	int CurrFrameResourceIndex = 0;


};
