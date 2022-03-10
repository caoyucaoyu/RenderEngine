#include "stdafx.h"
#include "Engine.h"
#include "DxRenderer.h"


DxRenderer::DxRenderer()
{

}

bool DxRenderer::Init()
{
	InitDirect3D();
	//InitDraw();
	return true;
}

void DxRenderer::Render()
{	
	Update(Engine::Get()->GetTimer());
	Draw();
}

bool DxRenderer::InitDirect3D()
{
	//Enable Debug
	ComPtr<ID3D12Debug> DebugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController))))
	{
		DebugController->EnableDebugLayer();
	}
	else
	{
		MessageBox(HMainWnd, L"Debug Failed.", 0, 0);
	}

	CreateDevice();
	CreateFence();
	GetDescriptorSize();
	SetMSAA();

	CreateCommandObject();
	CreateSwapChain();
	CreateDescriptorHeap();

	CreateRTV();
	CreateDSV();
	CreateViewPortAndScissorRect();

	//for (int i = 0; i < SwapChainBufferCount; ++i)
	//	SwapChainBuffer[i].Reset();
	//DepthStencilBuffer.Reset();

	//SwapChain->ResizeBuffers(SwapChainBufferCount, ClientWidth, ClientHight, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
	//CurrBackBuffer = 0;

	//ThrowIfFailed(CommandList->Close());
	//ID3D12CommandList* cmdsLists[] = { CommandList.Get() };
	//CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	//FlushCommandQueue();

	OutputDebugStringA("DxInit Success\n");
	return true;
}

bool DxRenderer::InitDraw()
{
	CommandList->Reset(CommandListAlloc.Get(), nullptr);

	BuildRenderData();
	//BuildGeometry();
	BuildFrameResource();
	BuildDescriptorHeaps();
	BuildConstantBuffers();
	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildPSO();

	ThrowIfFailed(CommandList->Close());
	ID3D12CommandList* cmdLists[] = { CommandList.Get() };
	CommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);//送入命令队列

	FlushCommandQueue();

	return true;
}


void DxRenderer::Update(const GameTimer& Gt)
{
	CurrFrameResourceIndex = (CurrFrameResourceIndex + 1) % FrameResourcesCount;
	CurrFrameResource = FrameResources[CurrFrameResourceIndex].get();
	if (CurrFrameResource->Fence != 0 && Fence->GetCompletedValue() < CurrFrameResource->Fence)
	{
		HANDLE EventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(Fence->SetEventOnCompletion(CurrFrameResource->Fence, EventHandle));
		WaitForSingleObject(EventHandle, INFINITE);
		CloseHandle(EventHandle);
	}

	DirectX::XMMATRIX v = Engine::Get()->GetScene()->GetMainCamera().GetView();
	DirectX::XMMATRIX p = Engine::Get()->GetScene()->GetMainCamera().GetProj();
	DirectX::XMMATRIX VP_Matrix = v * p;
	PassConstants PasConstant;
	PasConstant.Time = Gt.TotalTime();

	XMStoreFloat4x4(&PasConstant.ViewProj_M, XMMatrixTranspose(VP_Matrix));//XMMATRIX赋值给XMFLOAT4X4
	CurrFrameResource->PassCB->CopyData(0, PasConstant);//数据拷贝至GPU缓存


	for (int i = 0; i < DrawCount; i++)
	{
		ObjectConstants ObjConstant;
		DirectX::XMMATRIX L = XMLoadFloat4x4(&NDrawList[i].Location_Matrix);
		DirectX::XMMATRIX R = XMLoadFloat4x4(&NDrawList[i].Rotation_Matrix);
		DirectX::XMMATRIX S = XMLoadFloat4x4(&NDrawList[i].Scale3D_Matrix);

		//DirectX::XMMATRIX L = XMLoadFloat4x4(&DrawList[i]->Location_Matrix);
		//DirectX::XMMATRIX R = XMLoadFloat4x4(&DrawList[i]->Rotation_Matrix);
		//DirectX::XMMATRIX S = XMLoadFloat4x4(&DrawList[i]->Scale3D_Matrix);

		//DirectX::XMMATRIX W_Matrix = w ;//* DirectX::XMMatrixTranslation(i*300, i*300, 0.0f);
		//float trans=120;
		//L*= DirectX::XMMatrixTranslation(sin(Gt.TotalTime()) * trans, sin(Gt.TotalTime()) * trans, 0.0f);
		XMStoreFloat4x4(&ObjConstant.Location_M, XMMatrixTranspose(L));
		XMStoreFloat4x4(&ObjConstant.Rotation_M, XMMatrixTranspose(R));
		XMStoreFloat4x4(&ObjConstant.Scale3D_M, XMMatrixTranspose(S));
		CurrFrameResource->ObjectCB->CopyData(i, ObjConstant);
	}
}

void DxRenderer::Draw()
{
	auto CurrentAllocator = CurrFrameResource->CmdListAlloc;

	CurrentAllocator->Reset();

	CommandList->Reset(CurrentAllocator.Get(), PSOs[0].Get());

	CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(SwapChainBuffers[CurrBackBuffer].Get(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CommandList->RSSetViewports(1, &ScreenViewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);

	CommandList->ClearRenderTargetView(CurrentBackBufferView(), DirectX::Colors::Black, 0, nullptr);
	CommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	CommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	//---
	ID3D12DescriptorHeap* DescriptorHeaps[] = { CbvHeap.Get() };
	CommandList->SetDescriptorHeaps(_countof(DescriptorHeaps), DescriptorHeaps);

	CommandList->SetGraphicsRootSignature(RootSignature.Get());

	int PasCbvIndex = FrameResourcesCount * DrawCount + CurrFrameResourceIndex;//3*n+Curr
	auto Handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(CbvHeap->GetGPUDescriptorHandleForHeapStart());
	Handle.Offset(PasCbvIndex, CbvSrvUavDescriptorSize);
	CommandList->SetGraphicsRootDescriptorTable(1, Handle);

	//for (int i = 0; i < DrawList.size(); i++)
	for (int i = 0; i < DrawCount; i++)
	{	
		RenderMesh DrawMesh= FindRMesh(NDrawList[i].MeshName);//new

		CommandList->SetPipelineState(PSOs[0].Get());

		//if(DrawList[i]->Indices.size() ==2304)	
		if (DrawMesh.Indices.size() == 2304)//new
			CommandList->SetPipelineState(PSOs[1].Get());

		//CommandList->IASetVertexBuffers(0, 1, &DrawList[i]->VertexBufferView());	//换为 绘制项 对应 Mesh 的VBV
		//CommandList->IASetIndexBuffer(&DrawList[i]->IndexBufferView());				//换为 同理

		CommandList->IASetVertexBuffers(0, 1, &DrawMesh.VertexBufferView());
		CommandList->IASetIndexBuffer(&DrawMesh.IndexBufferView());

		CommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		CD3DX12_GPU_DESCRIPTOR_HANDLE ObjCbvHandle;
		int ObjCbvIndex = CurrFrameResourceIndex * DrawCount + i;//
		ObjCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(CbvHeap->GetGPUDescriptorHandleForHeapStart());
		ObjCbvHandle.Offset(ObjCbvIndex, CbvSrvUavDescriptorSize);
		CommandList->SetGraphicsRootDescriptorTable(0, ObjCbvHandle);

		CommandList->DrawIndexedInstanced((UINT)DrawMesh.Indices.size(), 1, 0, 0, 0);//换为 同理
	}
	//---

	CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(SwapChainBuffers[CurrBackBuffer].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	CommandList->Close();
	ID3D12CommandList* CmdsLists[] = { CommandList.Get() };
	CommandQueue->ExecuteCommandLists(_countof(CmdsLists), CmdsLists);

	SwapChain->Present(0, 0);
	CurrBackBuffer = (CurrBackBuffer + 1) % SwapChainBufferCount;

	CurrFrameResource->Fence = ++CurrentFence;
	CommandQueue->Signal(Fence.Get(), CurrentFence);

}

void DxRenderer::Reset()
{
	OutputDebugStringA("Render Reset \n");
	assert(D3dDevice);
	assert(SwapChain);
	assert(CommandListAlloc);

	FlushCommandQueue();

	CommandList->Reset(CommandListAlloc.Get(), nullptr);

	for (int i = 0; i < SwapChainBufferCount; ++i)
		SwapChainBuffers[i].Reset();
	DepthStencilBuffer.Reset();

	SwapChain->ResizeBuffers(SwapChainBufferCount, ClientWidth, ClientHight, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
	CurrBackBuffer = 0;

	CreateRTV();
	CreateDSV();
	CreateViewPortAndScissorRect();

	ThrowIfFailed(CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { CommandList.Get() };
	CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	FlushCommandQueue();
}

void DxRenderer::CreateDevice()
{
	//Create IDXGIFactory
	CreateDXGIFactory1(IID_PPV_ARGS(&DxgiFactory));

	//Create Hardware Device.
	HRESULT hardwareResult = D3D12CreateDevice(
		nullptr,             // default adapter
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&D3dDevice));
}

void DxRenderer::CreateFence()
{
	D3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence));
}

void DxRenderer::GetDescriptorSize()
{
	RtvDescriptorSize = D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	DsvDescriptorSize = D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	CbvSrvUavDescriptorSize = D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void DxRenderer::SetMSAA()
{
	MsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	MsaaQualityLevels.SampleCount = 1;
	MsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	MsaaQualityLevels.NumQualityLevels = 0;
	D3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &MsaaQualityLevels, sizeof(MsaaQualityLevels));
	assert(MsaaQualityLevels.NumQualityLevels > 0);
}

void DxRenderer::CreateCommandObject()
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

void DxRenderer::CreateSwapChain()
{
	HMainWnd=Engine::Get()->GetApp()->GetWnd();
	
	SwapChain.Reset();
	DXGI_SWAP_CHAIN_DESC Scd;
	Scd.BufferDesc.Width = ClientWidth;
	Scd.BufferDesc.Height = ClientHight;
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

void DxRenderer::CreateDescriptorHeap()
{
	//Create Rtv DescriptorHeaps
	D3D12_DESCRIPTOR_HEAP_DESC RtvHeapDesc;
	RtvHeapDesc.NumDescriptors = SwapChainBufferCount;
	RtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	RtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	RtvHeapDesc.NodeMask = 0;
	D3dDevice->CreateDescriptorHeap(&RtvHeapDesc, IID_PPV_ARGS(RtvHeap.GetAddressOf()));
	//Create Dsv DescriptorHeaps
	D3D12_DESCRIPTOR_HEAP_DESC DsvHeapDesc;
	DsvHeapDesc.NumDescriptors = 1;
	DsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	DsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DsvHeapDesc.NodeMask = 0;
	D3dDevice->CreateDescriptorHeap(&DsvHeapDesc, IID_PPV_ARGS(DsvHeap.GetAddressOf()));
}

void DxRenderer::CreateRTV()
{
	//Create RenderTargetView
	CD3DX12_CPU_DESCRIPTOR_HANDLE RtvHeapHandle(RtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < SwapChainBufferCount; i++)
	{
		SwapChain->GetBuffer(i, IID_PPV_ARGS(&SwapChainBuffers[i]));
		D3dDevice->CreateRenderTargetView(SwapChainBuffers[i].Get(), nullptr, RtvHeapHandle);
		RtvHeapHandle.Offset(1, RtvDescriptorSize);
	}
}

void DxRenderer::CreateDSV()
{
	//Create Depth/Stencil View
	D3D12_RESOURCE_DESC DepthStencilDesc;
	DepthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	DepthStencilDesc.Alignment = 0;
	DepthStencilDesc.Width = ClientWidth;
	DepthStencilDesc.Height = ClientHight;
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
	D3dDevice->CreateDepthStencilView(DepthStencilBuffer.Get(), &DsvDesc, DepthStencilView());
}

void DxRenderer::FlushCommandQueue()
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

void DxRenderer::CreateViewPortAndScissorRect()
{
	//Set ScreenV ScissorR
	ScreenViewport.TopLeftX = 0.0f;
	ScreenViewport.TopLeftY = 0.0f;
	ScreenViewport.Width = static_cast<float>(ClientWidth);
	ScreenViewport.Height = static_cast<float>(ClientHight);
	ScreenViewport.MinDepth = 0.0f;
	ScreenViewport.MaxDepth = 1.0f;
	ScissorRect = { 0,0,ClientWidth,ClientHight };
}

D3D12_CPU_DESCRIPTOR_HANDLE DxRenderer::DepthStencilView()
{
	return DsvHeap->GetCPUDescriptorHandleForHeapStart();
}

D3D12_CPU_DESCRIPTOR_HANDLE DxRenderer::CurrentBackBufferView()
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		RtvHeap->GetCPUDescriptorHandleForHeapStart(),
		CurrBackBuffer,
		RtvDescriptorSize);
}



void DxRenderer::BuildGeometry()
{
	//std::vector<MapItem> MeshsData;
	//D3DUtil::ReadMapFile("StaticMesh\\Map1.Usmh", MeshsData);

	//for (auto MeshData : MeshsData)
	//{
	//	auto OMesh = std::make_shared<RenderMesh>(MeshData);

	//	OMesh->BuildDefaultBuffer(D3dDevice.Get(), CommandList.Get());

	//	DrawList.push_back(OMesh);//绘制项 
	//	DrawCount++;
	//}
}

void DxRenderer::BuildRenderData()
{
	NDrawList = Engine::Get()->GetScene()->GetSceneMeshActors();
	DrawCount = NDrawList.size();

	for (auto DrawListActor : NDrawList)
	{
		Mesh AMesh;
		Engine::Get()->GetResourceManager()->FindMesh(DrawListActor.MeshName,AMesh);

		if (DrawMeshList.count(DrawListActor.MeshName))
			continue;		

		RenderMesh ARenderMesh(AMesh);
	
		ARenderMesh.BuildDefaultBuffer(D3dDevice.Get(), CommandList.Get());
		DrawMeshList.insert(std::make_pair<>(DrawListActor.MeshName,ARenderMesh));//std::string,RenderMesh
	}
}

void DxRenderer::BuildFrameResource()
{
	for (int i = 0; i < FrameResourcesCount; i++)
	{
		FrameResources.push_back(std::make_unique<FrameResource>(D3dDevice.Get(), 1, DrawCount));//PassCount ObjCount
	}
}

void DxRenderer::BuildDescriptorHeaps()
{
	DescriptorsNum = (DrawCount + 1) * FrameResourcesCount;//Meshs

	D3D12_DESCRIPTOR_HEAP_DESC CbvHeapDesc;
	CbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	CbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	//cbHeapDesc.NumDescriptors = 2* FrameResourcesCount;//包含CBV的数量,乘了3帧
	CbvHeapDesc.NumDescriptors = DescriptorsNum;//Meshs
	CbvHeapDesc.NodeMask = 0;
	D3dDevice->CreateDescriptorHeap(&CbvHeapDesc, IID_PPV_ARGS(&CbvHeap));
}

void DxRenderer::BuildConstantBuffers()
{
	UINT ObjCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	UINT PassCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(PassConstants));
	UINT TimeCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(TimeConstants));

	//前三组
	//ObjCBV
	for (int FrameIndex = 0; FrameIndex < FrameResourcesCount; FrameIndex++)
	{
		auto CurObjectCB = FrameResources[FrameIndex]->ObjectCB->Resource();
		for (int i = 0; i < DrawCount; i++)
		{
			D3D12_GPU_VIRTUAL_ADDRESS ObjCBAddress = CurObjectCB->GetGPUVirtualAddress();

			ObjCBAddress += i * ObjCBByteSize;//缓冲区 第i个常量缓冲区偏移量 得到起始位置

			int HeapIndex = FrameIndex * DrawCount + i;//CBV堆 当前帧当前物体对应CB的序号

			auto Handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(CbvHeap->GetCPUDescriptorHandleForHeapStart());//获得CBV堆首地址
			Handle.Offset(HeapIndex, CbvSrvUavDescriptorSize);

			D3D12_CONSTANT_BUFFER_VIEW_DESC CbvDesc;
			CbvDesc.BufferLocation = ObjCBAddress;
			CbvDesc.SizeInBytes = ObjCBByteSize;
			D3dDevice->CreateConstantBufferView(&CbvDesc, Handle);
		}
	}

	for (int FrameIndex = 0; FrameIndex < FrameResourcesCount; FrameIndex++)
	{
		D3D12_GPU_VIRTUAL_ADDRESS PassCBAddress = FrameResources[FrameIndex]->PassCB->Resource()->GetGPUVirtualAddress();
		int HeapIndex = DrawCount * FrameResourcesCount + FrameIndex;

		auto Handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(CbvHeap->GetCPUDescriptorHandleForHeapStart());//获得CBV堆首地址
		Handle.Offset(HeapIndex, CbvSrvUavDescriptorSize);

		D3D12_CONSTANT_BUFFER_VIEW_DESC CbvDesc;
		CbvDesc.BufferLocation = PassCBAddress;
		CbvDesc.SizeInBytes = PassCBByteSize;  // D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
		D3dDevice->CreateConstantBufferView(&CbvDesc, Handle);
	}

}

void DxRenderer::BuildRootSignature()
{
	//RootSignature
	CD3DX12_ROOT_PARAMETER SlotRootParameter[2];//根参数

	CD3DX12_DESCRIPTOR_RANGE CbvTable0;
	CbvTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	CD3DX12_DESCRIPTOR_RANGE CbvTable1;
	CbvTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

	SlotRootParameter[0].InitAsDescriptorTable(1, &CbvTable0);
	SlotRootParameter[1].InitAsDescriptorTable(1, &CbvTable1);

	CD3DX12_ROOT_SIGNATURE_DESC RootSigDesc(
		2,//3个根参数
		SlotRootParameter,//根参数指针
		0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> SerializedRootSig = nullptr;
	ComPtr<ID3DBlob> ErrorBlob = nullptr;
	D3D12SerializeRootSignature(&RootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, SerializedRootSig.GetAddressOf(), ErrorBlob.GetAddressOf());
	D3dDevice->CreateRootSignature(0, SerializedRootSig->GetBufferPointer(), SerializedRootSig->GetBufferSize(), IID_PPV_ARGS(&RootSignature));

}

void DxRenderer::BuildShadersAndInputLayout()
{
	MvsByteCode[0] = D3DUtil::CompileShader(L"Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
	MpsByteCode[0] = D3DUtil::CompileShader(L"Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");

	MvsByteCode[1] = D3DUtil::CompileShader(L"Shaders\\colorOffset.hlsl", nullptr, "VS", "vs_5_0");
	MpsByteCode[1] = D3DUtil::CompileShader(L"Shaders\\colorOffset.hlsl", nullptr, "PS", "ps_5_0");

	InputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		,{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void DxRenderer::BuildPSO()
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
