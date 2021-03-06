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

	//´´˝¨RenderTarget
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

	ResetBuffers();
	SwapChain->ResizeBuffers(SwapChainBufferCount, Width, Height, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
	CurrentBackBufferIndex = 0;
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

void DX12RHI::UpdateFrameResource()
{
	//Öˇ×ĘÔ´±äĎÂŇ»Öˇ
	CurrFrameResourceIndex = (CurrFrameResourceIndex + 1) % FrameResourcesCount;
	CurrFrameResource = FrameResources[CurrFrameResourceIndex].get();

	if (CurrFrameResource->Fence != 0 && Fence->GetCompletedValue() < CurrFrameResource->Fence)//µ±Ç°Öˇ˛»ĘÇµÚŇ»´Î ÇŇ GPUÎ´ÍęłÉ´ËÖˇ µČ´ý
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

	CommandQueue->Signal(Fence.Get(), CurrentFence);//ĚíĽÓĂüÁîŁşÉčÖĂĐÂÎ§Ŕ¸µă

	if (Fence->GetCompletedValue() < CurrentFence)//µČ¸ĂµăÇ°µÄČ«˛żÍęłÉ
	{
		HANDLE EventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		Fence->SetEventOnCompletion(CurrentFence, EventHandle);//ČôĂüÖĐĽ´Ö´ĐĐµ˝SignalŁ¬Ľ¤·˘ĘÂĽţ

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


void DX12RHI::SetBackBufferBegin()
{
	//şóĚ¨»şłĺ×ĘÔ´´ÓłĘĎÖ×´Ě¬×Ş»»µ˝äÖČľÄż±ę×´Ě¬
	CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GetCurrentBackBuffer().Get(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	//ÉčÖĂĘÓżÚşÍ˛ĂĽôľŘĐÎ
	CommandList->RSSetViewports(1, &ScreenViewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);

	//ÇĺłýşóĚ¨»şłĺÇřşÍÉî¶Č»şłĺÇřŁ¬˛˘¸łÖµ
	CommandList->ClearRenderTargetView(CurrentBackBufferView(), DirectX::Colors::Black, 0, nullptr);
	CommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	////Ö¸¶¨˝«ŇŞäÖČľµÄ»şłĺÇřŁ¬Ö¸¶¨RTVşÍDSV
	CommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());
}

void DX12RHI::SetBackBufferEnd()
{
	//şóĚ¨»şłĺÇřµÄ×´Ě¬¸ÄłÉłĘĎÖ×´Ě¬
	CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GetCurrentBackBuffer().Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
}

void DX12RHI::SetRenderTargetBegin(GPURenderTarget* RenderTarget)
{
	auto DxRT = static_cast<DX12GPURenderTarget*>(RenderTarget);

	//ÉčÖĂĘÓżÚşÍ˛ĂĽôľŘĐÎ
	CommandList->RSSetViewports(1, &DxRT->Viewport);
	CommandList->RSSetScissorRects(1, &DxRT->ScissorRect);

	//RenderTarget CPU Descriptor Handle
	const D3D12_CPU_DESCRIPTOR_HANDLE* RtvHandleP = nullptr;
	auto RTBuffer = DxRT->GetColorBuffer(0);//ŐâŔďČˇ 0 Ł¬˝»»»Á´2¸öColorBufferĚŘĘâÇéżöŁ¬ÓĂÁËĚŘĘâşŻĘýĚŘ¶¨´¦ŔíŁ¬ĆäËű¶ĽĘÇŇ»¸öColorBufferŁ¬Óöµ˝¶ŕ¸öŇ˛żÉŇÔÔŮ¸Ä
	if (RTBuffer != nullptr)
	{
		auto RTBufferHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(RtvHeap->GetCurrentHeap()->GetCPUDescriptorHandleForHeapStart());
		RTBufferHandle.Offset(RTBuffer->GetHandleOffset(), RtvDescriptorSize);
		RtvHandleP = &RTBufferHandle;
	}

	//DepthStencil CPU Descriptor Handle
	const D3D12_CPU_DESCRIPTOR_HANDLE* DsvHandleP = nullptr;
	auto DSBuffer = DxRT->GetDepthStencilBuffer();
	if (DSBuffer != nullptr)
	{
		auto DSBufferHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(DsvHeap->GetCurrentHeap()->GetCPUDescriptorHandleForHeapStart());
		DSBufferHandle.Offset(DSBuffer->GetHandleOffset(), DsvDescriptorSize);
		DsvHandleP = &DSBufferHandle;
	}

	//Ö¸¶¨˝«ŇŞäÖČľµÄ»şłĺÇřŁ¬Ö¸¶¨RTVşÍDSV
	CommandList->OMSetRenderTargets(0, RtvHandleP, false, DsvHandleP);

	////ÇĺłýşóĚ¨»şłĺÇřşÍÉî¶Č»şłĺÇřŁ¬˛˘¸łÖµ
	if(RtvHandleP !=nullptr)
		CommandList->ClearRenderTargetView(*RtvHandleP, DirectX::Colors::Black, 0, nullptr);
	if(DsvHandleP !=nullptr)
		CommandList->ClearDepthStencilView(*DsvHandleP, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
}

void DX12RHI::SetRenderTargetBufferBegin(GPURenderTarget* RenderTarget)
{
	for (auto ColorBuffer : RenderTarget->GetColorBuffers())
	{
		auto DxColorBuffer = static_cast<DX12GPURenderTargetBuffer*>(ColorBuffer);

		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(DxColorBuffer->GetResource().Get(),
			D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET));
	}

	auto DSBuffer = RenderTarget->GetDepthStencilBuffer();
	if (DSBuffer)
	{
		auto DxDSBuffer = static_cast<DX12GPURenderTargetBuffer*>(DSBuffer);

		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(DxDSBuffer->GetResource().Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));
	}
}

void DX12RHI::SetRenderTargetBufferEnd(GPURenderTarget* RenderTarget)
{
	for (auto ColorBuffer : RenderTarget->GetColorBuffers())
	{
		auto DxColorBuffer = static_cast<DX12GPURenderTargetBuffer*>(ColorBuffer);

		//şóĚ¨»şłĺ×ĘÔ´´ÓłĘĎÖ×´Ě¬×Ş»»µ˝äÖČľÄż±ę×´Ě¬
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(DxColorBuffer->GetResource().Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON));
	}

	auto DSBuffer = RenderTarget->GetDepthStencilBuffer();
	if (DSBuffer)
	{
		auto DxDSBuffer = static_cast<DX12GPURenderTargetBuffer*>(DSBuffer);

		//×ĘÔ´´ÓĐ´Čë×´Ě¬×Ş»»µ˝Ö»¶ÁÄż±ę×´Ě¬
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(DxDSBuffer->GetResource().Get(),
			D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));
	}
}


void DX12RHI::SetGraphicsPipeline(Pipeline* NPipeline, int TemporaryType)
{
	//DX12Pipeline* InDX12Pipeline = static_cast<DX12Pipeline*>(NPipeline);
	//CommandList->SetPipelineState(InDX12Pipeline->GetPipelineState().Get());

	//DX12Shader* PipelineShader = static_cast<DX12Shader*>(InDX12Pipeline->GetShader());
	//CommandList->SetGraphicsRootSignature(PipelineShader->GetRootSignatureDx().Get());

	if (TemporaryType == 0)
		CommandList->SetPipelineState(PSOs[0].Get());
	else if (TemporaryType == 1)
		CommandList->SetPipelineState(PSOs[1].Get());
	else if (TemporaryType == 2)
		CommandList->SetPipelineState(PSOs[2].Get());

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
		RTexture->Filename.c_str(),//˝«wstring×ŞłÉwChar_t
		RTexture->TResource, RTexture->UploadHeap);

	auto HandleAndOffset = CbvSrvUavHeap->Allocate(1);
	RTexture->SetHandleOffset(HandleAndOffset.Offset);

	//SRV¶ŃÖĐ×ÓSRVµÄľä±ú
	auto Handle = HandleAndOffset.Handle;
	//SRVĂčĘö˝áąąĚĺ
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;//˛ÉŃůşó·ÖÁżËłĐň˛»¸Ä±ä
	srvDesc.Format = RTexture->TResource->GetDesc().Format;//ĘÓÍĽµÄÄ¬ČĎ¸ńĘ˝
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2DĚůÍĽ
	srvDesc.Texture2D.MostDetailedMip = 0;//Ď¸˝Ú×îĎęľˇµÄmipmap˛ăĽ¶ÎŞ0
	srvDesc.Texture2D.MipLevels = RTexture->TResource->GetDesc().MipLevels;//mipmap ˛ăĽ¶ĘýÁż
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;//żÉ·ĂÎĘµÄmipmap×îĐˇ˛ăĽ¶ĘýÎŞ0
	//´´˝¨SRV
	D3dDevice->CreateShaderResourceView(RTexture->TResource.Get(), &srvDesc, Handle);

	ExecuteCommandList();
	FlushCommandQueue();
	return RTexture;
}

GPUTexture* DX12RHI::CreateTexture(std::string TextureName, GPURenderTargetBuffer* RTBuffer)
{
	DX12GPUTexture* RTexture = new DX12GPUTexture();
	RTexture->Name = TextureName;

	auto DxRtBuffer = static_cast<DX12GPURenderTargetBuffer*>(RTBuffer);

	auto HandleAndOffset = CbvSrvUavHeap->Allocate(1);
	RTexture->SetHandleOffset(HandleAndOffset.Offset);

	//´´˝¨SRV żÉŇÔÔÚshaderÖĐ˛ÉŃůÉî¶ČÍĽ
	//SRV¶ŃÖĐ×ÓSRVµÄľä±ú
	auto Handle = HandleAndOffset.Handle;
	//SRVĂčĘö˝áąąĚĺ
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;//˛ÉŃůşó·ÖÁżËłĐň˛»¸Ä±ä
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;//ĘÓÍĽµÄÄ¬ČĎ¸ńĘ˝
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2DĚůÍĽ
	srvDesc.Texture2D.MostDetailedMip = 0;//Ď¸˝Ú×îĎęľˇµÄmipmap˛ăĽ¶ÎŞ0
	srvDesc.Texture2D.MipLevels = 1;//RTexture->TResource->GetDesc().MipLevels;//mipmap ˛ăĽ¶ĘýÁż
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;//żÉ·ĂÎĘµÄmipmap×îĐˇ˛ăĽ¶ĘýÎŞ0

	//´´˝¨SRV
	//D3dDevice->CreateShaderResourceView(RTexture->TResource.Get(), &srvDesc, Handle);

	//´´˝¨SRV
	//ĘąÓĂRenderTargetBufferµÄResource´´˝¨SRV
	//RenderTargetBufferŔďµÄTexture´ćOffset
	D3dDevice->CreateShaderResourceView(DxRtBuffer->GetResource().Get(), &srvDesc, Handle);

	return RTexture;
}

GPURenderTarget* DX12RHI::CreateRenderTarget(std::string RTName, UINT W, UINT H)
{	
	return new DX12GPURenderTarget(RTName, W, H);
}

GPURenderTargetBuffer* DX12RHI::CreateRenderTargetBuffer(RTBufferType Type, UINT W, UINT H)
{
	//´´˝¨Resource ˛˘´´˝¨View
	auto Buffer = new DX12GPURenderTargetBuffer(Type, W, H);
	Buffer->CreateResource(D3dDevice.Get());

	FAllocation Allocation;
	if (Type == RTBufferType::Color)
	{
		Allocation = RtvHeap->Allocate(1);
	}
	else
	{
		Allocation = DsvHeap->Allocate(1);
	}

	Buffer->SetHandleOffset(Allocation.Offset);
	Buffer->CreateView(D3dDevice.Get(), Allocation);


	return Buffer;
}

void DX12RHI::RootSignatureAndPSO()
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
	DsvHeap = std::make_unique<DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3dDevice);
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

		//ĐčŇŞÖŘĐÂ´´˝¨Resource
		//ĐčŇŞ´Ó¶ŃŔďČˇĎű·ÖĹä
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

		//´íÎó
		/*	SwapChain->GetBuffer(i, IID_PPV_ARGS(&RTBuffer->GetResource()));
			cout<<"GetBuffer\n";

			auto Allocation = RtvHeap->Allocate(1);
			RTBuffer->SetHandleOffset(Allocation.Offset);
			RTBuffer->CreateView(D3dDevice.Get(), Allocation);*/
		
		//ŐýČ·
		ComPtr<ID3D12Resource> Res;
		SwapChain->GetBuffer(i, IID_PPV_ARGS(&Res));
		RTBuffer->SetResource(Res);
		auto Allocation = RtvHeap->Allocate(1);
		RTBuffer->SetHandleOffset(Allocation.Offset);
		RTBuffer->CreateView(D3dDevice.Get(), Allocation);

		//Ô­ÓĐ
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

	//2¸öBufferˇŁAddBuffĘ±´´˝¨ÁËResource´´˝¨ÁËView
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
	//Default
	MvsByteCode[0] = D3DUtil::CompileShader(L"Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
	MpsByteCode[0] = D3DUtil::CompileShader(L"Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");

	//Dynamic
	MvsByteCode[1] = D3DUtil::CompileShader(L"Shaders\\colorOffset.hlsl", nullptr, "VS", "vs_5_0");
	MpsByteCode[1] = D3DUtil::CompileShader(L"Shaders\\colorOffset.hlsl", nullptr, "PS", "ps_5_0");

	//ShadowMap
	MvsByteCode[2] = D3DUtil::CompileShader(L"Shaders\\ShadowMap.hlsl", nullptr, "VS", "vs_5_0");
	MpsByteCode[2] = D3DUtil::CompileShader(L"Shaders\\ShadowMap.hlsl", nullptr, "PS", "ps_5_0");

	//ShadowMap
	//MvsByteCode[3] = D3DUtil::CompileShader(L"Shaders\\ShadowMap.hlsl", nullptr, "VS", "vs_5_0");
	//MpsByteCode[3] = D3DUtil::CompileShader(L"Shaders\\ShadowMap.hlsl", nullptr, "PS", "ps_5_0");

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
	//PsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;  //ĎßżňÄŁĘ˝

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


	//PsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;  //ĎßżňÄŁĘ˝
	PsoDesc.VS = { reinterpret_cast<BYTE*>(MvsByteCode[1]->GetBufferPointer()),MvsByteCode[1]->GetBufferSize() };
	PsoDesc.PS = { reinterpret_cast<BYTE*>(MpsByteCode[1]->GetBufferPointer()),MpsByteCode[1]->GetBufferSize() };

	PSOs.push_back(Pso);
	D3dDevice->CreateGraphicsPipelineState(&PsoDesc, IID_PPV_ARGS(&PSOs[1]));



	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc = {};
	ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaquePsoDesc.InputLayout = { InputLayout.data(), (UINT)InputLayout.size() };
	opaquePsoDesc.pRootSignature = RootSignature.Get();
	//opaquePsoDesc.VS = { reinterpret_cast<BYTE*>(MvsByteCode[2]->GetBufferPointer()),MvsByteCode[2]->GetBufferSize() };
	//opaquePsoDesc.PS = { reinterpret_cast<BYTE*>(MpsByteCode[2]->GetBufferPointer()),MpsByteCode[2]->GetBufferSize() };
	// 
	//opaquePsoDesc.VS =
	//{
	//	reinterpret_cast<BYTE*>(shaders["standardVS"]->GetBufferPointer()),
	//	shaders["standardVS"]->GetBufferSize()
	//};
	//opaquePsoDesc.PS =
	//{
	//	reinterpret_cast<BYTE*>(shaders["opaquePS"]->GetBufferPointer()),
	//	shaders["opaquePS"]->GetBufferSize()
	//};
	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//opaquePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;	//0xffffffff,Č«˛ż˛ÉŃůŁ¬Ă»ÓĐŐÚŐÖ
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;	//ąéŇ»»ŻµÄÎŢ·űşĹŐűĐÍ
	opaquePsoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	opaquePsoDesc.SampleDesc.Count = 1;	//˛»ĘąÓĂ4XMSAA
	opaquePsoDesc.SampleDesc.Quality = 0;	////˛»ĘąÓĂ4XMSAA
	//PSOs.push_back(Pso);
	//D3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&PSOs[2]));
	//ThrowIfFailed(D3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&PSOs[2])));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC shadowMapPsoDesc = opaquePsoDesc;
	//shadowMapPsoDesc.RasterizerState.DepthBias = 100000;//ąĚ¶¨µÄĆ«ŇĆÁż
	//shadowMapPsoDesc.RasterizerState.DepthBiasClamp = 0.0f;// ÔĘĐíµÄ×î´óÉî¶ČĆ«ŇĆÁż
	//shadowMapPsoDesc.RasterizerState.SlopeScaledDepthBias = 1.0f;//¸ůľÝ¶ŕ±ßĐÎĐ±ÂĘŔ´żŘÖĆĆ«ŇĆłÉ¶ĽµÄËő·ĹŇň×Ó
	shadowMapPsoDesc.pRootSignature = RootSignature.Get();
	shadowMapPsoDesc.VS = { reinterpret_cast<BYTE*>(MvsByteCode[2]->GetBufferPointer()),MvsByteCode[2]->GetBufferSize() };
	shadowMapPsoDesc.PS = { reinterpret_cast<BYTE*>(MpsByteCode[2]->GetBufferPointer()),MpsByteCode[2]->GetBufferSize() };
	// ŇőÓ°ÍĽµÄäÖČľąýłĚÎŢĐčÉćĽ°äÖČľÄż±ę
	shadowMapPsoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
	shadowMapPsoDesc.NumRenderTargets = 0;	//Ă»ÓĐäÖČľÄż±ęŁ¬˝űÖąŃŐÉ«Ęäłö
	PSOs.push_back(Pso);
	ThrowIfFailed(D3dDevice->CreateGraphicsPipelineState(&shadowMapPsoDesc, IID_PPV_ARGS(&PSOs[2])));
}

void DX12RHI::BuildRootSignature()
{
	CD3DX12_STATIC_SAMPLER_DESC pointClamp(1,	//×ĹÉ«Ć÷ĽÄ´ćĆ÷
		D3D12_FILTER_MIN_MAG_MIP_POINT,		//ąýÂËĆ÷ŔŕĐÍÎŞPOINT(łŁÁż˛ĺÖµ)
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	//U·˝ĎňÉĎµÄŃ°Ö·ÄŁĘ˝ÎŞCLAMPŁ¨ÇŻÎ»Ń°Ö·ÄŁĘ˝Ł©
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	//V·˝ĎňÉĎµÄŃ°Ö·ÄŁĘ˝ÎŞCLAMPŁ¨ÇŻÎ»Ń°Ö·ÄŁĘ˝Ł©
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP);	//W·˝ĎňÉĎµÄŃ°Ö·ÄŁĘ˝ÎŞCLAMPŁ¨ÇŻÎ»Ń°Ö·ÄŁĘ˝Ł©


	//RootSignature
	CD3DX12_ROOT_PARAMETER SlotRootParameter[ParameterNum];//¸ů˛ÎĘý

	//´´˝¨ĂčĘö·ű±í
	CD3DX12_DESCRIPTOR_RANGE CbvTable0;
	CbvTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);// ĂčĘö·űŔŕĐÍ ĘýÁż ĽÄ´ćĆ÷˛ŰşĹ
	CD3DX12_DESCRIPTOR_RANGE CbvTable1;
	CbvTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
	CD3DX12_DESCRIPTOR_RANGE CbvTable2;
	CbvTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);

	CD3DX12_DESCRIPTOR_RANGE TexTable0;
	TexTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	CD3DX12_DESCRIPTOR_RANGE TexTable1;
	TexTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
	CD3DX12_DESCRIPTOR_RANGE TexTable2;
	TexTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);

	SlotRootParameter[0].InitAsDescriptorTable(1, &CbvTable0);
	SlotRootParameter[1].InitAsDescriptorTable(1, &CbvTable1);
	SlotRootParameter[2].InitAsDescriptorTable(1, &CbvTable2);
	SlotRootParameter[3].InitAsDescriptorTable(1, &TexTable0, D3D12_SHADER_VISIBILITY_PIXEL);
	SlotRootParameter[4].InitAsDescriptorTable(1, &TexTable1, D3D12_SHADER_VISIBILITY_PIXEL);
	SlotRootParameter[5].InitAsDescriptorTable(1, &TexTable2, D3D12_SHADER_VISIBILITY_PIXEL);

	//SlotRootParameter[2].InitAsConstantBufferView(2);//ĐÂĚíĽÓ Ö®şó ÉľµôµÄ

	//SlotRootParameter[1].InitAsConstantBufferView(1);

	//
	//CD3DX12_ROOT_SIGNATURE_DESC RootSigDesc(
	//	3,//3¸ö¸ů˛ÎĘý
	//	SlotRootParameter,//¸ů˛ÎĘýÖ¸Őë
	//	0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	auto StaticSamplers = GetStaticSamplers();
	CD3DX12_ROOT_SIGNATURE_DESC RootSigDesc(
		ParameterNum,//¸ö¸ů˛ÎĘý
		SlotRootParameter,//¸ů˛ÎĘýÖ¸Őë
		(UINT)StaticSamplers.size(),
		StaticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);



	ComPtr<ID3DBlob> SerializedRootSig = nullptr;
	ComPtr<ID3DBlob> ErrorBlob = nullptr;
	D3D12SerializeRootSignature(&RootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, SerializedRootSig.GetAddressOf(), ErrorBlob.GetAddressOf());

	D3dDevice->CreateRootSignature(0, SerializedRootSig->GetBufferPointer(), SerializedRootSig->GetBufferSize(), IID_PPV_ARGS(&RootSignature));

}




std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> DX12RHI::GetStaticSamplers()
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

	const CD3DX12_STATIC_SAMPLER_DESC shadow(6, // ×ĹÉ«Ć÷ĽÄ´ćĆ÷
		D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // U·˝ĎňÉĎµÄŃ°Ö·ÄŁĘ˝ÎŞBORDER
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // V·˝ĎňÉĎµÄŃ°Ö·ÄŁĘ˝ÎŞBORDER
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // W·˝ĎňÉĎµÄŃ°Ö·ÄŁĘ˝ÎŞBORDER
		0.0f,                               // mipLODBias
		16,                                 // maxAnisotropy
		D3D12_COMPARISON_FUNC_LESS_EQUAL,	//Ö´ĐĐŇőÓ°ÍĽµÄ±Č˝Ď˛âĘÔ
		D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK);


	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp, shadow};
}
