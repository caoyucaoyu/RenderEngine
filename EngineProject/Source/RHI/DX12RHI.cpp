#include "stdafx.h"
#include "Engine.h"
#include "DX12RHI.h"
using Microsoft::WRL::ComPtr;

DX12RHI::DX12RHI()
{

}

DX12RHI::~DX12RHI()
{

}

void DX12RHI::Init()
{
	ComPtr<ID3D12Debug> DebugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController))))
	{
		DebugController->EnableDebugLayer();
	}

	CreateDevice();
	CreateFence();
	GetDescriptorSize();
	SetMSAA();
	CreateCommandObject();
	CreateSwapChain();

	OutputDebugStringA("DX12 Init Success\n");
}

void DX12RHI::Flush()
{
	CurrentFence++;

	CommandQueue->Signal(Fence.Get(), CurrentFence);//添加命令：设置新围栏点
	if (Fence->GetCompletedValue() < CurrentFence)//等该点前的全部完成
	{
		HANDLE EventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		Fence->SetEventOnCompletion(CurrentFence, EventHandle);//若命中即执行到Signal，激发事件

		WaitForSingleObject(EventHandle, INFINITE);
		CloseHandle(EventHandle);
	}
}

void DX12RHI::DrawInstanced(UINT DrawIndexCount)
{
	CommandList->DrawInstanced(DrawIndexCount, 1, 0, 0);
}

void DX12RHI::ResizeWindow(UINT32 Width, UINT32 Height)
{

}

void DX12RHI::CreateDevice()
{
	//Create IDXGIFactory
	CreateDXGIFactory1(IID_PPV_ARGS(&DxgiFactory));

	//Create Hardware Device.
	D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&D3dDevice));
}

void DX12RHI::CreateFence()
{
	D3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence));
}

void DX12RHI::GetDescriptorSize()
{
	RtvDescriptorSize = D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	DsvDescriptorSize = D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	CbvSrvUavDescriptorSize = D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void DX12RHI::SetMSAA()
{
	MsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	MsaaQualityLevels.SampleCount = 1;
	MsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	MsaaQualityLevels.NumQualityLevels = 0;
	D3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &MsaaQualityLevels, sizeof(MsaaQualityLevels));
	assert(MsaaQualityLevels.NumQualityLevels > 0);
}

void DX12RHI::CreateCommandObject()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	D3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&CommandQueue));
	D3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(CommandListAlloc.GetAddressOf()));
	D3dDevice->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		CommandListAlloc.Get(),
		nullptr,
		IID_PPV_ARGS(CommandList.GetAddressOf()));
	CommandList->Close();
}

void DX12RHI::CreateSwapChain()
{
	Window* wd = Engine::Get()->GetWindow();
	HMainWnd = wd->GetWnd();

	SwapChain.Reset();
	DXGI_SWAP_CHAIN_DESC Scd;
	Scd.BufferDesc.Width = wd->GetWidth();
	Scd.BufferDesc.Height = wd->GetHeight();

	Scd.BufferDesc.RefreshRate.Numerator = 60;
	Scd.BufferDesc.RefreshRate.Denominator = 1;
	Scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//BianLiang: DXGI_FORMAT mBackBufferFormat
	Scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	Scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	Scd.SampleDesc.Count = 1;//m4xMsaaState ? 4 : 1
	Scd.SampleDesc.Quality = 0;  //m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	Scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	Scd.BufferCount = SwapChainBufferCount;
	Scd.OutputWindow = HMainWnd;
	Scd.Windowed = true;
	Scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	Scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ThrowIfFailed(DxgiFactory->CreateSwapChain(CommandQueue.Get(), &Scd, SwapChain.GetAddressOf()));
}