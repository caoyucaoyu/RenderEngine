#include "D3DInit.h"
#include "MathHelper.h"
#include "FrameResource.h"

//array<Vertex, 8>Vertices =
//{
//		Vertex({ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::White) }),
//		Vertex({ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Black) }),
//		Vertex({ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Red) }),
//		Vertex({ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Green) }),
//		Vertex({ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Blue) }),
//		Vertex({ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Yellow) }),
//		Vertex({ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Cyan) }),
//		Vertex({ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Magenta) })
//};
//
//
//
//std::array<std::uint16_t, 36> Indices =
//{
//	// front face
//	0, 1, 2,
//	0, 2, 3,
//
//	// back face
//	4, 6, 5,
//	4, 7, 6,
//
//	// left face
//	4, 5, 1,
//	4, 1, 0,
//
//	// right face
//	3, 2, 6,
//	3, 6, 7,
//
//	// top face
//	1, 5, 6,
//	1, 6, 2,
//
//	// bottom face
//	4, 0, 3,
//	4, 3, 7
//};


D3DInit::D3DInit()
{

}

D3DInit::~D3DInit()
{

}

bool D3DInit::Init(HINSTANCE hInstance) 
{
	if(!D3DApp::Init(hInstance))
		return false;

	CommandList->Reset(CommandListAlloc.Get(),nullptr);

	//BuildFrameResource();
	BuildDescriptorHeaps();
	BuildConstantBuffers();
	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildGeometry();
	BuildPSO();

	ThrowIfFailed(CommandList->Close());
	ID3D12CommandList* cmdLists[] = { CommandList.Get() };
	CommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	FlushCommandQueue();

	return true;
}

void D3DInit::Draw()
{
	CommandListAlloc->Reset();
	CommandList->Reset(CommandListAlloc.Get(), PSO.Get());

	CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(SwapChainBuffer[CurrBackBuffer].Get(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CommandList->RSSetViewports(1, &ScreenViewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);

	CommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::PaleGreen, 0, nullptr);
	CommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	CommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	//---
	ID3D12DescriptorHeap* DescriptorHeaps[] = { CbvHeap.Get() };
	CommandList->SetDescriptorHeaps(_countof(DescriptorHeaps), DescriptorHeaps);

	CommandList->SetGraphicsRootSignature(RootSignature.Get());

	CommandList->IASetVertexBuffers(0, 1, &BoxGeo->VertexBufferView());

	CommandList->IASetIndexBuffer(&BoxGeo->IndexBufferView());

	CommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//描述符表与渲染管线绑定，
	//寄存器槽号,句柄（描述符表 第一个 符）所有符将设置到表中
	CommandList->SetGraphicsRootDescriptorTable(0, CbvHeap->GetGPUDescriptorHandleForHeapStart());

	CommandList->DrawIndexedInstanced((UINT)outIndexBuffer.size(), 1, 0, 0, 0);
	//---

	CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(SwapChainBuffer[CurrBackBuffer].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	CommandList->Close();
	ID3D12CommandList* CmdsLists[] = { CommandList.Get() };
	CommandQueue->ExecuteCommandLists(_countof(CmdsLists), CmdsLists);

	SwapChain->Present(0, 0);
	CurrBackBuffer = (CurrBackBuffer + 1) % SwapChainBufferCount;

	FlushCommandQueue();

	//CurrFrameResource->Fence = ++CurrentFence;
	//CommandQueue->Signal(Fence.Get(), CurrentFence);
}

void D3DInit::Update()
{	
	UpdateCamera();

	//CurrFrameResourceIndex = (CurrFrameResourceIndex + 1) % NumFrameResources;
	//CurrFrameResource = FrameResources[CurrFrameResourceIndex].get();
	//if (CurrFrameResource->Fence != 0 && Fence->GetCompletedValue() < CurrFrameResource->Fence)
	//{
	//	HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
	//	
	//	ThrowIfFailed(Fence->SetEventOnCompletion(CurrFrameResource->Fence, eventHandle));
	//	WaitForSingleObject(eventHandle, INFINITE);
	//	CloseHandle(eventHandle);
	//}

	XMMATRIX v = XMLoadFloat4x4(&View_Matrix);
	XMMATRIX p = XMLoadFloat4x4(&Proj_Matrix);

	//XMMATRIX p = XMMatrixPerspectiveFovLH(
	//	0.25f * 3.1416f, 
	//	static_cast<float>(ClientWidth) / ClientHight,
	//	//1280.0f / 720.0f, 
	//	1.0f, 
	//	1000.0f);
	//世界矩阵
	//XMMATRIX w = XMLoadFloat4x4(&World); 

	XMMATRIX WVP_Matrix = v * p;

	ObjectConstants ObjConstants;

	//XMMATRIX赋值给XMFLOAT4X4
	XMStoreFloat4x4(&ObjConstants.WorldViewProj, XMMatrixTranspose(WVP_Matrix));

	//数据拷贝至GPU缓存
	ObjectCB->CopyData(0, ObjConstants);
}

void D3DInit::OnResize()
{
	D3DApp::OnResize();
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * 3.1416f, static_cast<float>(ClientWidth) / ClientHight, 1.0f, 1000.0f);
	XMStoreFloat4x4(&Proj_Matrix, P);
}

void D3DInit::UpdateCamera()
{
	EyePos.y = Radius * cosf(Phi);
	EyePos.x = Radius * sinf(Phi) * cosf(Theta);
	EyePos.z = Radius * sinf(Phi) * sinf(Theta);
	float r = 1.0f;

	//x *= sinf(Timer.TotalTime());
	//z *= cosf(Timer.TotalTime());
	//z = sqrt(r * r - x * x);

	XMVECTOR pos = XMVectorSet(EyePos.x, EyePos.y, EyePos.z, r);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX v = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&View_Matrix, v);
}


void D3DInit::BuildFrameResource()
{
	for (int i = 0; i < NumFrameResources; ++i)
	{
		FrameResources.push_back(std::make_unique<FrameResource>(D3dDevice.Get(),0, 1));
	}
}

void D3DInit::BuildDescriptorHeaps()
{
	UINT ObjConstSize = D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	D3D12_DESCRIPTOR_HEAP_DESC cbHeapDesc;
	cbHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbHeapDesc.NumDescriptors = 1;
	cbHeapDesc.NodeMask = 0;
	D3dDevice->CreateDescriptorHeap(&cbHeapDesc, IID_PPV_ARGS(&CbvHeap));
}

void D3DInit::BuildConstantBuffers()
{
	ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(D3dDevice.Get(), 1, true);

	UINT ObjCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	D3D12_GPU_VIRTUAL_ADDRESS CbAddress = ObjectCB->Resource()->GetGPUVirtualAddress();

	int BoxCBufIndex = 0;
	CbAddress += BoxCBufIndex * ObjCBByteSize;

	D3D12_CONSTANT_BUFFER_VIEW_DESC CbvDesc;
	CbvDesc.BufferLocation = CbAddress;
	CbvDesc.SizeInBytes = D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	D3dDevice->CreateConstantBufferView(&CbvDesc,CbvHeap->GetCPUDescriptorHandleForHeapStart());
}

void D3DInit::BuildRootSignature()
{
	//RootSignature
	CD3DX12_ROOT_PARAMETER SlotRootParameter[1];
	CD3DX12_DESCRIPTOR_RANGE CbvTable;
	CbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,1,0);
	SlotRootParameter[0].InitAsDescriptorTable(1,&CbvTable);
	CD3DX12_ROOT_SIGNATURE_DESC RootSigDesc(1,SlotRootParameter,0,nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	ComPtr<ID3DBlob> SerializedRootSig = nullptr;
	ComPtr<ID3DBlob> ErrorBlob = nullptr;
	D3D12SerializeRootSignature(&RootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,SerializedRootSig.GetAddressOf(), ErrorBlob.GetAddressOf());
	D3dDevice->CreateRootSignature(0,SerializedRootSig->GetBufferPointer(),SerializedRootSig->GetBufferSize(),IID_PPV_ARGS(&RootSignature));
}

void D3DInit::BuildShadersAndInputLayout()
{
	MvsByteCode = CompileShader(L"Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
	MpsByteCode = CompileShader(L"Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");

	InputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void D3DInit::BuildGeometry()
{
	//Read Get Vb Ib

	D3DUtil::ReadMeshFile("StaticMesh\\Shape_QuadPyramid.Usmh", outVertexBuffer, outIndexBuffer);
	
	//std::stringstream ss;
	//for (const auto& ob : outVertexBuffer)
	//{
	//	ss << ob.Pos.x << "    " << ob.Pos.y << "    " << ob.Pos.z << "\n";	
	//}
	//for (const auto& ob : outIndexBuffer)
	//{	
	//	ss << ob <<"\n";
	//}
	//OutputDebugStringA(ss.str().c_str());

	//Get Size
	VbByteSize = (UINT)outVertexBuffer.size() * sizeof(Vertex);
	IbByteSize = (UINT)outIndexBuffer.size() * sizeof(std::uint16_t);

	BoxGeo = std::make_unique<MeshGeometry>();

	//BoxGeo
	D3DCreateBlob(VbByteSize, &BoxGeo->VertexBufferCPU);
	D3DCreateBlob(IbByteSize, &BoxGeo->IndexBufferCPU);
	CopyMemory(BoxGeo->IndexBufferCPU->GetBufferPointer(), outIndexBuffer.data(), IbByteSize);
	CopyMemory(BoxGeo->VertexBufferCPU->GetBufferPointer(), outVertexBuffer.data(), VbByteSize);
	BoxGeo->VertexBufferGPU = CreateDefaultBuffer(D3dDevice.Get(), CommandList.Get(), outVertexBuffer.data(), VbByteSize, BoxGeo->VertexBufferUploader);
	BoxGeo->IndexBufferGPU = CreateDefaultBuffer(D3dDevice.Get(), CommandList.Get(), outIndexBuffer.data(), IbByteSize, BoxGeo->IndexBufferUploader);
	BoxGeo->IndexBufferByteSize= IbByteSize;
	BoxGeo->VertexBufferByteSize = VbByteSize;
}

void D3DInit::BuildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC PsoDesc;
	ZeroMemory(&PsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	PsoDesc.InputLayout = { InputLayout.data(), (UINT)InputLayout.size() };
	PsoDesc.pRootSignature = RootSignature.Get();
	PsoDesc.VS ={reinterpret_cast<BYTE*>(MvsByteCode->GetBufferPointer()),MvsByteCode->GetBufferSize()};
	PsoDesc.PS ={reinterpret_cast<BYTE*>(MpsByteCode->GetBufferPointer()),MpsByteCode->GetBufferSize()};

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

	D3dDevice->CreateGraphicsPipelineState(&PsoDesc, IID_PPV_ARGS(&PSO));

}



ComPtr<ID3DBlob> D3DInit::CompileShader(const std::wstring& Filename, const D3D_SHADER_MACRO* Defines, const std::string& EntryPoint, const std::string& Target)
{
	ComPtr<ID3DBlob> ByteCode;
	ComPtr<ID3DBlob> Errors;
	D3DCompileFromFile(Filename.c_str(), Defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		EntryPoint.c_str(), Target.c_str(), 0, 0, &ByteCode, &Errors);
	return ByteCode;
}


void D3DInit::OnMouseDown(WPARAM btnState, int x, int y)
{
	LastMousePos.x = x;
	LastMousePos.y = y;

	SetCapture(HMainWnd);
}

void D3DInit::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void D3DInit::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - LastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - LastMousePos.y));

		Theta += dx;

		Phi += dy;
		Phi = MathHelper::Clamp(Phi, 0.1f, MathHelper::Pi - 0.1f);

	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		float dx = 0.005f * static_cast<float>(x - LastMousePos.x);
		float dy = 0.005f * static_cast<float>(y - LastMousePos.y);

		Radius += (dx - dy) * 25;

		Radius = MathHelper::Clamp(Radius, 300.0f, 600.0f);
	}
	LastMousePos.x = x;
	LastMousePos.y = y;
}


ComPtr<ID3D12Resource> D3DInit::CreateDefaultBuffer(ID3D12Device* Device,
	ID3D12GraphicsCommandList* CmdList, const void* InitData, UINT64 ByteSize, ComPtr<ID3D12Resource>& UploadBuffer)
{
	ComPtr<ID3D12Resource> DefaultBuffer;

	//Create Default Buffer
	Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(ByteSize),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(DefaultBuffer.GetAddressOf()));

	//Create Upload Heap,  for Copy CPU into Default Buffer
	Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(ByteSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(UploadBuffer.GetAddressOf()));

	D3D12_SUBRESOURCE_DATA SubResourceData = {};
	SubResourceData.pData = InitData;
	SubResourceData.RowPitch = ByteSize;
	SubResourceData.SlicePitch = SubResourceData.RowPitch;

	CmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(DefaultBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources<1>(CmdList, DefaultBuffer.Get(), UploadBuffer.Get(), 0, 0, 1, &SubResourceData);
	CmdList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(DefaultBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	return DefaultBuffer;
}


