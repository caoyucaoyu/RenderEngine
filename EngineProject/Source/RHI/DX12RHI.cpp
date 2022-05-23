#include "stdafx.h"
#include "Engine.h"
#include "DX12RHI.h"

#include "DescriptorHeap.h"
#include "RenderScene.h"

#include "DX12GPUCommonBuffer.h"
#include "DX12Pipeline.h"
#include "DX12Shader.h"
#include "DX12GPUTexture.h"
#include "DX12GPURenderTarget.h"

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
	CreatCbvSrvUavHeap();
	CreateFrameResource();

	//创建RenderTarget
	CreateBackRenderTerget();

	OutputDebugStringA("SS DX12 Init Success\n");
}

void DX12RHI::ResizeWindow(UINT32 Width, UINT32 Height)
{
	//std::cout << "DX12 Resize Window\n" << std::endl;
	OutputDebugStringA("SS DX12 Resize Window\n");
	assert(D3dDevice);
	assert(SwapChain);
	assert(CommandListAllocator);

	FlushCommandQueue();
	ResetCommandList(CommandListAllocator);

	//
	ResetBuffers();
	SwapChain->ResizeBuffers(SwapChainBufferCount, Width, Height, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
	CurrentBackBufferIndex = 0;
	//
	CreateRTV();
	//
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
	CurrentBackBufferIndex = (CurrentBackBufferIndex + 1) % SwapChainBufferCount;

	CurrFrameResource->Fence = ++CurrentFence;
	CommandQueue->Signal(Fence.Get(), CurrentFence);
}

void DX12RHI::CreateFrameResource()
{
	FrameResources.resize(FrameResourcesCount);
	for (int i = 0; i < FrameResourcesCount; i++)
	{
		FrameResources[i] = std::make_unique<FrameResource>(D3dDevice.Get());
	}
}

void DX12RHI::RebuildFrameResource(RenderScene* MRenderScene)
{

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

int DX12RHI::GetCurFrameResourceIdx()
{
	return CurrFrameResourceIndex;
}

int DX12RHI::GetFrameResourceCount()
{
	return FrameResourcesCount;
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

void DX12RHI::DrawIndexedInstanced(UINT DrawIndexCount)
{
	CommandList->DrawIndexedInstanced(DrawIndexCount, 1, 0, 0, 0);
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


void DX12RHI::SetRenderTargetBegin()
{
	//后台缓冲资源从呈现状态转换到渲染目标状态
	CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GetCurrentBackBuffer().Get(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	//设置视口和裁剪矩形
	CommandList->RSSetViewports(1, &ScreenViewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);

	//清除后台缓冲区和深度缓冲区，并赋值
	CommandList->ClearRenderTargetView(CurrentBackBufferView(), DirectX::Colors::Black, 0, nullptr);
	CommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	////指定将要渲染的缓冲区，指定RTV和DSV
	CommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());
}

void DX12RHI::SetRenderTargetEnd()
{
	//后台缓冲区的状态改成呈现状态
	CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GetCurrentBackBuffer().Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
}


void DX12RHI::SetGraphicsPipeline(Pipeline* NPipeline)
{
	//DX12Pipeline* InDX12Pipeline = static_cast<DX12Pipeline*>(NPipeline);
	//CommandList->SetPipelineState(InDX12Pipeline->GetPipelineState().Get());

	//DX12Shader* PipelineShader = static_cast<DX12Shader*>(InDX12Pipeline->GetShader());
	//CommandList->SetGraphicsRootSignature(PipelineShader->GetRootSignatureDx().Get());

	CommandList->SetPipelineState(PSOs[1].Get());
	CommandList->SetGraphicsRootSignature(RootSignature.Get());
}


void DX12RHI::SetRenderResourceTable(int RootParameterIndex, UINT32 HeapOffset)
{
	auto CbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(CbvSrvUavHeap->GetCurrentHeap()->GetGPUDescriptorHandleForHeapStart());
	CbvHandle.Offset(HeapOffset, CbvSrvUavDescriptorSize);

	CommandList->SetGraphicsRootDescriptorTable(RootParameterIndex, CbvHandle);
}

void DX12RHI::IASetMeshBuffer(GPUMeshBuffer* GPUMeshbuffer)
{
	DX12GPUMeshBuffer* DX12Meshbuffer = static_cast<DX12GPUMeshBuffer*>(GPUMeshbuffer);
	CommandList->IASetVertexBuffers(0, 1, &DX12Meshbuffer->VertexBufferView());
	CommandList->IASetIndexBuffer(&DX12Meshbuffer->IndexBufferView());
	CommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

GPUMeshBuffer* DX12RHI::CreateMeshBuffer()
{
	return new DX12GPUMeshBuffer();
}

void DX12RHI::UpdateMeshBuffer(GPUMeshBuffer* GpuMeshBuffer)
{
	ResetCommandList(CommandListAllocator);

	DX12GPUMeshBuffer* DX12GpuMeshBuffer = static_cast<DX12GPUMeshBuffer*>(GpuMeshBuffer);
	DX12GpuMeshBuffer->BuildDefaultBuffer(D3dDevice.Get(),CommandList.Get());
	
	ExecuteCommandList();
	FlushCommandQueue();
}

GPUCommonBuffer* DX12RHI::CreateCommonBuffer(UINT ElementCount, bool IsConstantBuffer, UINT ElementByteSize)
{
	DX12GPUCommonBuffer* CommonBuffer = new DX12GPUCommonBuffer(D3dDevice.Get(), ElementCount, IsConstantBuffer, ElementByteSize);

	auto HandleAndOffset = CbvSrvUavHeap->Allocate(ElementCount);

	CommonBuffer->SetBufferHandle(HandleAndOffset.Handle);
	CommonBuffer->SetHandleOffset(HandleAndOffset.Offset);

	for (size_t index = 0; index < ElementCount; index++)
	{
		//Create View
		D3D12_GPU_VIRTUAL_ADDRESS CBAddress = CommonBuffer->Resource()->GetGPUVirtualAddress();
		CBAddress+=index* CommonBuffer->GetElementByteSize();

		D3D12_CONSTANT_BUFFER_VIEW_DESC CbvDesc;
		CbvDesc.BufferLocation = CBAddress;
		CbvDesc.SizeInBytes = CommonBuffer->GetElementByteSize();
		D3dDevice->CreateConstantBufferView(&CbvDesc, CommonBuffer->GetHandle());
	}

	return CommonBuffer;
}

void DX12RHI::UpdateCommonBuffer(GPUCommonBuffer* GpuCommonBuffer, std::shared_ptr<void> Data,int elementIndex)
{
	GpuCommonBuffer->CopyData(elementIndex,Data);//??? 0?
}

void DX12RHI::AddCommonBuffer(int FrameSourceIndex,std::string CommonBufferTag, GPUCommonBuffer* GpuCommonBuffer)
{
	FrameResources[FrameSourceIndex]->CommonBuffers.push_back(GpuCommonBuffer);
}

GPUTexture* DX12RHI::CreateTexture(std::string TextureName, std::wstring FileName)
{
	ResetCommandList(CommandListAllocator);

	DX12GPUTexture* RTexture = new DX12GPUTexture();
	RTexture->Name = TextureName;
	RTexture->Filename = FileName;

	DirectX::CreateDDSTextureFromFile12(D3dDevice.Get(), CommandList.Get(),
		RTexture->Filename.c_str(),//将wstring转成wChar_t
		RTexture->TResource, RTexture->UploadHeap);

	auto HandleAndOffset = CbvSrvUavHeap->Allocate(1);
	RTexture->SetHandleOffset(HandleAndOffset.Offset);

	//SRV堆中子SRV的句柄
	auto Handle = HandleAndOffset.Handle;
	//SRV描述结构体
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;//采样后分量顺序不改变
	srvDesc.Format = RTexture->TResource->GetDesc().Format;//视图的默认格式
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2D贴图
	srvDesc.Texture2D.MostDetailedMip = 0;//细节最详尽的mipmap层级为0
	srvDesc.Texture2D.MipLevels = RTexture->TResource->GetDesc().MipLevels;//mipmap 层级数量
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;//可访问的mipmap最小层级数为0
	//创建SRV
	D3dDevice->CreateShaderResourceView(RTexture->TResource.Get(), &srvDesc, Handle);

	ExecuteCommandList();
	FlushCommandQueue();
	return RTexture;
}

GPUTexture* DX12RHI::CreateTexture(std::string TextureName)
{
	DX12GPUTexture* RTexture = new DX12GPUTexture();
	RTexture->Name = TextureName;

	auto HandleAndOffset = CbvSrvUavHeap->Allocate(1);
	RTexture->SetHandleOffset(HandleAndOffset.Offset);

	//创建SRV 可以在shader中采样深度图
	//SRV堆中子SRV的句柄
	auto Handle = HandleAndOffset.Handle;
	//SRV描述结构体
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;//采样后分量顺序不改变
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;//视图的默认格式
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2D贴图
	srvDesc.Texture2D.MostDetailedMip = 0;//细节最详尽的mipmap层级为0
	srvDesc.Texture2D.MipLevels = 1;//RTexture->TResource->GetDesc().MipLevels;//mipmap 层级数量
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;//可访问的mipmap最小层级数为0
	//创建SRV
	D3dDevice->CreateShaderResourceView(RTexture->TResource.Get(), &srvDesc, Handle);


	return RTexture;
}

GPURenderTarget* DX12RHI::CreateRenderTarget(std::string RTName, UINT W, UINT H)
{	
	return new DX12GPURenderTarget(RTName, W, H);
}

GPURenderTargetBuffer* DX12RHI::CreateRenderTargetBuffer(RTBufferType Type, UINT W, UINT H)
{
	//创建Resource 并创建View
	auto Buffer = new DX12GPURenderTargetBuffer(Type, W, H);
	Buffer->CreateResource(D3dDevice.Get());

	FAllocation Allocation;
	if(Type == RTBufferType::Color)
		Allocation = RtvHeap->Allocate(1);
	else if (Type == RTBufferType::DepthStencil)
		Allocation = DsvHeap->Allocate(1);
	
	Buffer->SetHandleOffset(Allocation.Offset);
	Buffer->CreateView(D3dDevice.Get(), Allocation);
	return Buffer;
}

void DX12RHI::XXX()
{
	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildPSO();
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
	MsaaQuality4X = MsaaQualityLevels.NumQualityLevels;
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

void DX12RHI::ResetBuffers()
{
	//for (int i = 0; i < SwapChainBufferCount; ++i)
	//{
	//	SwapChainBuffers[i].Reset();
	//}
	//DepthStencilBuffer.Reset();
	//
	//if (!BackBufferRT)
	//{
	//	return;
	//}

	DX12GPURenderTargetBuffer* RTBufferdx;
	for (int i = 0; i < SwapChainBufferCount; ++i)
	{
		GPURenderTargetBuffer* BackBuffer = BackBufferRT->GetColorBuffer(i);
		if (BackBuffer != nullptr)
			BackBuffer->ResetResource();

		//需要重新创建Resource
		//需要从堆里取消分配
		RTBufferdx = static_cast<DX12GPURenderTargetBuffer*>(BackBuffer);
		RTBufferdx->CreateResource(D3dDevice.Get());
		RtvHeap->Deallocate(RTBufferdx->GetHandleOffset(), 1);
	}

	GPURenderTargetBuffer* DSBuffer = BackBufferRT->GetDepthStencilBuffer();
	if (DSBuffer != nullptr)
		DSBuffer->ResetResource();

	RTBufferdx = static_cast<DX12GPURenderTargetBuffer*>(DSBuffer);
	RTBufferdx->CreateResource(D3dDevice.Get());
	DsvHeap->Deallocate(RTBufferdx->GetHandleOffset(), 1);
}

void DX12RHI::CreateRTV()
{
	//Create RenderTargetView
	for (UINT i = 0; i < SwapChainBufferCount; i++)
	{	
		auto RTBuffer = static_cast<DX12GPURenderTargetBuffer*>(BackBufferRT->GetColorBuffer(i));
		SwapChain->GetBuffer(i, IID_PPV_ARGS(&RTBuffer->GetResource()));

		auto Allocation = RtvHeap->Allocate(1);
		RTBuffer->SetHandleOffset(Allocation.Offset);
		RTBuffer->CreateView(D3dDevice.Get(), Allocation);
		//
		//
		//
		//SwapChain->GetBuffer(i, IID_PPV_ARGS(&SwapChainBuffers[i]));
		//auto RtvAllocation = RtvHeap->Allocate(1);
		//D3dDevice->CreateRenderTargetView(SwapChainBuffers[i].Get(), nullptr, RtvAllocation.Handle);
	}

}

void DX12RHI::CreateDSV()
{
	//Create Depth/Stencil Resource Then Create View
	//
	//D3D12_RESOURCE_DESC DepthStencilDesc;
	//DepthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	//DepthStencilDesc.Alignment = 0;
	//DepthStencilDesc.Width = Wd->GetWidth();
	//DepthStencilDesc.Height = Wd->GetHeight();
	//DepthStencilDesc.DepthOrArraySize = 1;
	//DepthStencilDesc.MipLevels = 1;
	//DepthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	//DepthStencilDesc.SampleDesc.Count = 1;
	//DepthStencilDesc.SampleDesc.Quality = 0;
	//DepthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	//DepthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	//
	//D3D12_CLEAR_VALUE OptClear;
	//OptClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//DXGI_FORMAT DepthStencilFormat
	//OptClear.DepthStencil.Depth = 1.0f;
	//OptClear.DepthStencil.Stencil = 0;
	//D3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
	//	D3D12_HEAP_FLAG_NONE,
	//	&DepthStencilDesc,
	//	D3D12_RESOURCE_STATE_DEPTH_WRITE,
	//	&OptClear,
	//	IID_PPV_ARGS(DepthStencilBuffer.GetAddressOf()));
	//
	//DepthStencilBuffer->SetName(L"MyDSBuffer");
	//
	//D3D12_DEPTH_STENCIL_VIEW_DESC DsvDesc;
	//DsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	//DsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	//DsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//DXGI_FORMAT mDepthStencilFormat
	//DsvDesc.Texture2D.MipSlice = 0;
	//
	//auto DsvAllocation = DsvHeap->Allocate(1);
	//D3dDevice->CreateDepthStencilView(DepthStencilBuffer.Get(), &DsvDesc, DsvAllocation.Handle);

	//new
	auto RTBuffer = static_cast<DX12GPURenderTargetBuffer*>(BackBufferRT->GetDepthStencilBuffer());
	FAllocation Allocation = DsvHeap->Allocate(1);
	RTBuffer->SetHandleOffset(Allocation.Offset);
	RTBuffer->CreateView(D3dDevice.Get(), Allocation);
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

void DX12RHI::CreateBackRenderTerget()
{
	BackBufferRT = CreateRenderTarget("BackBuffer", Wd->GetWidth(), Wd->GetHeight());

	//2个Buffer。AddBuff时创建了Resource创建了View
	BackBufferRT->AddColorBuffer(SwapChainBufferCount);
	BackBufferRT->AddDepthStencilBuffer();
}

void DX12RHI::CreatCbvSrvUavHeap()
{
	CbvSrvUavHeap = std::make_unique<DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3dDevice ,2048);
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12RHI::CurrentBackBufferView() const
{
	//return CD3DX12_CPU_DESCRIPTOR_HANDLE(
	//	RtvHeap->GetCurrentHeap()->GetCPUDescriptorHandleForHeapStart(),
	//	CurrentBackBufferIndex,
	//	RtvDescriptorSize);

	auto Buffer = BackBufferRT->GetColorBuffer(CurrentBackBufferIndex);
	auto Handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(RtvHeap->GetCurrentHeap()->GetCPUDescriptorHandleForHeapStart());
	Handle.Offset(Buffer->GetHandleOffset(), RtvDescriptorSize);
	return Handle;
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12RHI::DepthStencilView() const
{
	//return DsvHeap->GetCurrentHeap()->GetCPUDescriptorHandleForHeapStart();

	auto Buffer = BackBufferRT->GetDepthStencilBuffer();
	auto Handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(DsvHeap->GetCurrentHeap()->GetCPUDescriptorHandleForHeapStart());
	Handle.Offset(Buffer->GetHandleOffset(), DsvDescriptorSize);
	return Handle;
}

ComPtr<ID3D12Resource> DX12RHI::GetCurrentBackBuffer() const
{
	//return SwapChainBuffers[CurrentBackBufferIndex];

	DX12GPURenderTargetBuffer* RTBuffer = static_cast<DX12GPURenderTargetBuffer*>(BackBufferRT->GetColorBuffer(CurrentBackBufferIndex));
	return RTBuffer->GetResource();
}


void DX12RHI::BuildShadersAndInputLayout()
{
	MvsByteCode[0] = D3DUtil::CompileShader(L"Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
	MpsByteCode[0] = D3DUtil::CompileShader(L"Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");

	MvsByteCode[1] = D3DUtil::CompileShader(L"Shaders\\colorOffset.hlsl", nullptr, "VS", "vs_5_0");
	MpsByteCode[1] = D3DUtil::CompileShader(L"Shaders\\colorOffset.hlsl", nullptr, "PS", "ps_5_0");

	InputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		,{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }  //12
		,{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }      //16
	};
}

void DX12RHI::BuildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC PsoDesc;
	ZeroMemory(&PsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	PsoDesc.InputLayout = { InputLayout.data(), (UINT)InputLayout.size() };
	PsoDesc.pRootSignature = RootSignature.Get();
	PsoDesc.VS = { reinterpret_cast<BYTE*>(MvsByteCode[0]->GetBufferPointer()),MvsByteCode[0]->GetBufferSize() };
	PsoDesc.PS = { reinterpret_cast<BYTE*>(MpsByteCode[0]->GetBufferPointer()),MpsByteCode[0]->GetBufferSize() };

	PsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//PsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;  //线框模式

	PsoDesc.RasterizerState.FrontCounterClockwise = TRUE;
	PsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	PsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	PsoDesc.SampleMask = UINT_MAX;
	PsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	PsoDesc.NumRenderTargets = 1;
	PsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;//DXGI_FORMAT mBackBufferFormat
	PsoDesc.SampleDesc.Count = 1;//m4xMsaaState ? 4 : 1;
	PsoDesc.SampleDesc.Quality = 0;//m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	PsoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;//DXGI_FORMAT mDepthStencilFormat

	auto Pso = ComPtr<ID3D12PipelineState>();
	PSOs.push_back(Pso);
	D3dDevice->CreateGraphicsPipelineState(&PsoDesc, IID_PPV_ARGS(&PSOs[0]));

	//PsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;  //线框模式
	PsoDesc.VS = { reinterpret_cast<BYTE*>(MvsByteCode[1]->GetBufferPointer()),MvsByteCode[1]->GetBufferSize() };
	PsoDesc.PS = { reinterpret_cast<BYTE*>(MpsByteCode[1]->GetBufferPointer()),MpsByteCode[1]->GetBufferSize() };

	PSOs.push_back(Pso);
	D3dDevice->CreateGraphicsPipelineState(&PsoDesc, IID_PPV_ARGS(&PSOs[1]));
}

void DX12RHI::BuildRootSignature()
{
	CD3DX12_STATIC_SAMPLER_DESC pointClamp(1,	//着色器寄存器
		D3D12_FILTER_MIN_MAG_MIP_POINT,		//过滤器类型为POINT(常量插值)
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	//U方向上的寻址模式为CLAMP（钳位寻址模式）
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	//V方向上的寻址模式为CLAMP（钳位寻址模式）
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP);	//W方向上的寻址模式为CLAMP（钳位寻址模式）


	//RootSignature
	CD3DX12_ROOT_PARAMETER SlotRootParameter[5];//根参数

	//创建描述符表
	CD3DX12_DESCRIPTOR_RANGE CbvTable0;
	CbvTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);// 描述符类型 数量 寄存器槽号
	CD3DX12_DESCRIPTOR_RANGE CbvTable1;
	CbvTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
	CD3DX12_DESCRIPTOR_RANGE CbvTable2;//新添加
	CbvTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);

	CD3DX12_DESCRIPTOR_RANGE TexTable0;
	TexTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	CD3DX12_DESCRIPTOR_RANGE TexTable1;
	TexTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

	SlotRootParameter[0].InitAsDescriptorTable(1, &CbvTable0);
	SlotRootParameter[1].InitAsDescriptorTable(1, &CbvTable1);
	SlotRootParameter[2].InitAsDescriptorTable(1, &CbvTable2);//新添加的
	SlotRootParameter[3].InitAsDescriptorTable(1, &TexTable0, D3D12_SHADER_VISIBILITY_PIXEL);
	SlotRootParameter[4].InitAsDescriptorTable(1, &TexTable1, D3D12_SHADER_VISIBILITY_PIXEL);

	//SlotRootParameter[2].InitAsConstantBufferView(2);//新添加 之后 删掉的

	//SlotRootParameter[1].InitAsConstantBufferView(1);

	//
	//CD3DX12_ROOT_SIGNATURE_DESC RootSigDesc(
	//	3,//3个根参数
	//	SlotRootParameter,//根参数指针
	//	0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	auto StaticSamplers = GetStaticSamplers();
	CD3DX12_ROOT_SIGNATURE_DESC RootSigDesc(
		5,//3个根参数
		SlotRootParameter,//根参数指针
		(UINT)StaticSamplers.size(),
		StaticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);



	ComPtr<ID3DBlob> SerializedRootSig = nullptr;
	ComPtr<ID3DBlob> ErrorBlob = nullptr;
	D3D12SerializeRootSignature(&RootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, SerializedRootSig.GetAddressOf(), ErrorBlob.GetAddressOf());

	D3dDevice->CreateRootSignature(0, SerializedRootSig->GetBufferPointer(), SerializedRootSig->GetBufferSize(), IID_PPV_ARGS(&RootSignature));

}




std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> DX12RHI::GetStaticSamplers()
{
	// Applications usually only need a handful of samplers.  So just define them all up front
		// and keep them available as part of the root signature.  

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp };
}
