#pragma once
#include "RHI.h"

using Microsoft::WRL::ComPtr;

class DX12RHI : public RHI
{
public:
	DX12RHI();
	~DX12RHI();

public:
	virtual void Init()override;
	virtual void Flush()override;
	virtual void DrawInstanced(UINT DrawIndexCount) override;
	virtual void ResizeWindow(UINT32 Width, UINT32 Height) override;

private:
	void CreateDevice();
	void CreateFence();
	void GetDescriptorSize();
	void SetMSAA();
	void CreateCommandObject();
	void CreateSwapChain();

private:
	ComPtr<IDXGIFactory> DxgiFactory;
	ComPtr<ID3D12Device> D3dDevice;
	ComPtr<ID3D12Fence> Fence;
	ComPtr<ID3D12CommandQueue> CommandQueue;
	ComPtr<ID3D12GraphicsCommandList> CommandList;
	ComPtr<ID3D12CommandAllocator> CommandListAlloc;
	ComPtr<IDXGISwapChain> SwapChain;
	UINT RtvDescriptorSize;
	UINT DsvDescriptorSize;
	UINT CbvSrvUavDescriptorSize;
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS MsaaQualityLevels;
	HWND HMainWnd;
	static const int SwapChainBufferCount = 2;

	UINT64 CurrentFence = 0;

	D3D12_VIEWPORT ScreenViewport;
	D3D12_RECT ScissorRect;

};
