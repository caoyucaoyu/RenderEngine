#pragma once
#include "Head.h"
#include "D3dUtil.h"
#include "GameTimer.h"

using Microsoft::WRL::ComPtr;
using namespace std;
using namespace DirectX;

class D3DApp
{
protected:
	D3DApp();
	virtual ~D3DApp();
	//D3DApp(const D3DApp& rhs) = delete;
	//D3DApp& operator=(const D3DApp& rhs) = delete;

public:
	static D3DApp* GetApp();

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
	bool InitWindow(HINSTANCE hInstance);
	bool InitDirect3D();
	int Run();

	virtual void Draw() = 0;
	virtual void Update() = 0;
	virtual void OnResize();
	virtual bool Init(HINSTANCE hInstance);

	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView();
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView();
	virtual LRESULT MsgProc(HWND Window, UINT Msg, WPARAM wParam, LPARAM lParam);

protected:
	virtual void OnMouseDown(WPARAM btnState, int x, int y) { }
	virtual void OnMouseUp(WPARAM btnState, int x, int y) { }
	virtual void OnMouseMove(WPARAM btnState, int x, int y) { }

protected:
	static D3DApp* App;

	GameTimer Timer;

	HWND HMainWnd;

	bool IsAppPaused = false; 
	bool IsMinimized = false; 
	bool IsMaximized = false;
	bool IsResizing = false;
	bool IsFullscreenState = false;

	ComPtr<IDXGIFactory> DxgiFactory;
	ComPtr<ID3D12Device> D3dDevice;
	ComPtr<ID3D12Fence> Fence;
	UINT RtvDescriptorSize;
	UINT DsvDescriptorSize; 
	UINT CbvSrvUavDescriptorSize; 
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS MsaaQualityLevels;
	ComPtr<ID3D12CommandQueue> CommandQueue;
	ComPtr<ID3D12GraphicsCommandList> CommandList;
	ComPtr<ID3D12CommandAllocator> CommandListAlloc;
	int ClientWidth = 1280;
	int ClientHight = 720;
	ComPtr<ID3D12DescriptorHeap> DsvHeap;
	ComPtr<ID3D12DescriptorHeap> RtvHeap;
	static const int SwapChainBufferCount = 2;
	ComPtr<ID3D12Resource> SwapChainBuffer[SwapChainBufferCount];
	ComPtr<ID3D12Resource> DepthStencilBuffer;
	ComPtr<IDXGISwapChain> SwapChain;
	UINT64 CurrentFence = 0;
	int CurrBackBuffer = 0;
	D3D12_VIEWPORT ScreenViewport;
	D3D12_RECT ScissorRect;
};

