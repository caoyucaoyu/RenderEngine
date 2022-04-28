#include "stdafx.h"
#include "Engine.h"
#include "DX12RHI.h"
#include "DX\DescriptorHeap.h"

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
	CreateRtvAndDsvDescriptorHeaps();

	//
	//CreateRTV();
	//CreateDSV();
	//CreateViewPortAndScissorRect();

	CreateFrameResource();
	CreatCbvSrvUavHeap();

	OutputDebugStringA("DX12 Init Success\n");
}

void DX12RHI::ResizeWindow(UINT32 Width, UINT32 Height)
{
	OutputDebugStringA("DX12 Resize Window\n");
	assert(D3dDevice);
	assert(SwapChain);
	assert(CommandListAllocator);

	FlushCommandQueue();
	ResetCommandList(CommandListAllocator);


	for (int i = 0; i < SwapChainBufferCount; ++i)
		SwapChainBuffers[i].Reset();
	DepthStencilBuffer.Reset();

	SwapChain->ResizeBuffers(SwapChainBufferCount, Width, Height, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
	CurrBackBuffer = 0;

	CreateRTV();
	CreateDSV();
	CreateViewPortAndScissorRect();


	ExecuteCommandList();
	FlushCommandQueue();
}


void DX12RHI::BeginFrame()
{
	auto CurrentAllocator = CurrFrameResource->CmdListAlloc;

	CurrentAllocator->Reset();

	ResetCommandList(CurrentAllocator.Get());
	
	PrepareBufferHeap();
}

void DX12RHI::EndFrame()
{
	ExecuteCommandList();

	SwapChain->Present(0, 0);
	CurrBackBuffer = (CurrBackBuffer + 1) % SwapChainBufferCount;

	CurrFrameResource->Fence = ++CurrentFence;
	CommandQueue->Signal(Fence.Get(), CurrentFence);
}

void DX12RHI::CreateFrameResource()
{
	FrameResources.resize(FrameResourcesCount);
	for (int i = 0; i < FrameResourcesCount; i++)
	{
		FrameResources[i] = std::make_unique<FrameResource>(D3dDevice.Get());
		//FrameResources[i]->Init(1, DrawCount, MaterialCount);
	}
}

void DX12RHI::UpdateFrameResource()
{
	//帧资源变下一帧
	CurrFrameResourceIndex = (CurrFrameResourceIndex + 1) % FrameResourcesCount;
	CurrFrameResource = FrameResources[CurrFrameResourceIndex].get();

	if (CurrFrameResource->Fence != 0 && Fence->GetCompletedValue() < CurrFrameResource->Fence)//当前帧不是第一次 且 GPU未完成此帧 等待
	{
		HANDLE EventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(Fence->SetEventOnCompletion(CurrFrameResource->Fence, EventHandle));
		WaitForSingleObject(EventHandle, INFINITE);
		CloseHandle(EventHandle);
	}
}

void DX12RHI::FlushCommandQueue()
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




void DX12RHI::ExecuteCommandList()
{
	CommandList->Close();
	ID3D12CommandList* CmdsLists[] = { CommandList.Get() };
	CommandQueue->ExecuteCommandLists(_countof(CmdsLists), CmdsLists);
}

void DX12RHI::ResetCommandList(ComPtr<ID3D12CommandAllocator> CommandListAllocator)
{
	ThrowIfFailed(CommandList->Reset(CommandListAllocator.Get(), nullptr));
}


void DX12RHI::PrepareBufferHeap()
{
	if (CbvSrvUavHeap->GetCurrentHeap() != nullptr)
	{
		ID3D12DescriptorHeap* DescriptorHeaps[] = { CbvSrvUavHeap->GetCurrentHeap() };
		CommandList->SetDescriptorHeaps(_countof(DescriptorHeaps), DescriptorHeaps);
	}
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
	D3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(CommandListAllocator.GetAddressOf()));
	D3dDevice->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		CommandListAllocator.Get(),
		nullptr,
		IID_PPV_ARGS(CommandList.GetAddressOf()));
	CommandList->Close();
}

void DX12RHI::CreateSwapChain()
{
	Wd = Engine::Get()->GetWindow();
	HMainWnd = Wd->GetWnd();

	SwapChain.Reset();
	DXGI_SWAP_CHAIN_DESC Scd;
	Scd.BufferDesc.Width = Wd->GetWidth();
	Scd.BufferDesc.Height = Wd->GetHeight();

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

void DX12RHI::CreateRtvAndDsvDescriptorHeaps()
{
	RtvHeap = std::make_unique<DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3dDevice, SwapChainBufferCount);
	DsvHeap = std::make_unique<DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3dDevice, 1);
}

void DX12RHI::CreateRTV()
{
	//Create RenderTargetView
	
	//CD3DX12_CPU_DESCRIPTOR_HANDLE RtvHeapHandle(RtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < SwapChainBufferCount; i++)
	{	
		SwapChain->GetBuffer(i, IID_PPV_ARGS(&SwapChainBuffers[i]));
		auto RtvAllocation = RtvHeap->Allocate(1);

		D3dDevice->CreateRenderTargetView(SwapChainBuffers[i].Get(), nullptr, RtvAllocation.Handle);
	}
}

void DX12RHI::CreateDSV()
{
	//Create Depth/Stencil View

	D3D12_RESOURCE_DESC DepthStencilDesc;
	DepthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	DepthStencilDesc.Alignment = 0;
	DepthStencilDesc.Width = Wd->GetWidth();
	DepthStencilDesc.Height = Wd->GetHeight();
	DepthStencilDesc.DepthOrArraySize = 1;
	DepthStencilDesc.MipLevels = 1;
	DepthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	DepthStencilDesc.SampleDesc.Count = 1;
	DepthStencilDesc.SampleDesc.Quality = 0;
	DepthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	DepthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE OptClear;
	OptClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//DXGI_FORMAT DepthStencilFormat
	OptClear.DepthStencil.Depth = 1.0f;
	OptClear.DepthStencil.Stencil = 0;
	D3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&DepthStencilDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&OptClear,
		IID_PPV_ARGS(DepthStencilBuffer.GetAddressOf()));

	DepthStencilBuffer->SetName(L"MyDSBuffer");

	D3D12_DEPTH_STENCIL_VIEW_DESC DsvDesc;
	DsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	DsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	DsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//DXGI_FORMAT mDepthStencilFormat
	DsvDesc.Texture2D.MipSlice = 0;

	auto DsvAllocation = DsvHeap->Allocate(1);
	D3dDevice->CreateDepthStencilView(DepthStencilBuffer.Get(), &DsvDesc, DsvAllocation.Handle);
}

void DX12RHI::CreateViewPortAndScissorRect()
{
	//Set ScreenV ScissorR
	ScreenViewport.TopLeftX = 0.0f;
	ScreenViewport.TopLeftY = 0.0f;
	ScreenViewport.Width = static_cast<float>(Wd->GetWidth());
	ScreenViewport.Height = static_cast<float>(Wd->GetHeight());
	ScreenViewport.MinDepth = 0.0f;
	ScreenViewport.MaxDepth = 1.0f;
	ScissorRect = { 0,0,Wd->GetWidth(),Wd->GetHeight() };
}

void DX12RHI::CreatCbvSrvUavHeap()
{
	CbvSrvUavHeap = std::make_unique<DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3dDevice);
}
