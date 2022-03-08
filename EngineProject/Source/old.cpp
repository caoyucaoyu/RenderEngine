#include "stdafx.h"
#include "MathHelper.h"
#include "FrameResource.h"

//using DirectX::XMConvertToRadians;
//
//
//
//TAppWin32::TAppWin32()
//{
//	//Init();
//}
//
//TAppWin32::~TAppWin32()
//{
//
//}
//
//bool TAppWin32::CreateAppWindow(HINSTANCE hInstance)
//{
//	if (Init(hInstance))
//	{
//		return true;
//	}
//	return false;
//}

//bool TAppWin32::Init(HINSTANCE hInstance)
//{
//	HINSTANCE hInstancexxx = GetModuleHandle(0);
//	if(!TAppWin::Init(hInstancexxx))
//		return false;
//
//	CommandList->Reset(CommandListAlloc.Get(),nullptr);
//
//	MainCamera.SetPosition(0.0f,-2000.0f,1500.0f);
//
//	BuildGeometry();
//	BuildFrameResource();
//	BuildDescriptorHeaps();
//	BuildConstantBuffers();
//	BuildRootSignature();
//	BuildShadersAndInputLayout();
//	BuildPSO();
//
//	ThrowIfFailed(CommandList->Close());
//	ID3D12CommandList* cmdLists[] = { CommandList.Get() };
//	CommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);//送入命令队列
//
//	FlushCommandQueue();
//
//	return true;
//}

//void TAppWin32::Draw()
//{
//	auto CurrentAllocator = CurrFrameResource->CmdListAlloc;
//
//	CurrentAllocator->Reset();
//
//	CommandList->Reset(CurrentAllocator.Get(), PSOs[0].Get());
//
//	CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(SwapChainBuffer[CurrBackBuffer].Get(),
//		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
//
//	CommandList->RSSetViewports(1, &ScreenViewport);
//	CommandList->RSSetScissorRects(1, &ScissorRect);
//
//	CommandList->ClearRenderTargetView(CurrentBackBufferView(), DirectX::Colors::LightSkyBlue, 0, nullptr);
//	CommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
//
//	CommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());
//
//	//---
//	ID3D12DescriptorHeap* DescriptorHeaps[] = { CbvHeap.Get() };
//	CommandList->SetDescriptorHeaps(_countof(DescriptorHeaps), DescriptorHeaps);
//
//	CommandList->SetGraphicsRootSignature(RootSignature.Get());
//
//
//	int PasCbvIndex = FrameResourcesCount * DrawCount + CurrFrameResourceIndex;//3*n+Curr
//	auto Handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(CbvHeap->GetGPUDescriptorHandleForHeapStart());
//	Handle.Offset(PasCbvIndex, CbvSrvUavDescriptorSize);
//	CommandList->SetGraphicsRootDescriptorTable(1, Handle);
//
//	//int TimCbvIndex = FrameResourcesCount * DrawCount + FrameResourcesCount + CurrFrameResourceIndex;//3*n+Curr
//	//auto TimHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(CbvHeap->GetGPUDescriptorHandleForHeapStart());
//	//TimHandle.Offset(TimCbvIndex, CbvSrvUavDescriptorSize);
//	//CommandList->SetGraphicsRootDescriptorTable(1, TimHandle);
//
//	for (int i=0;i<DrawList.size();i++)
//	{
//		if(i == DrawList.size()/2)
//			CommandList->SetPipelineState(PSOs[1].Get());
//		CommandList->IASetVertexBuffers(0, 1, &DrawList[i]->VertexBufferView());
//		CommandList->IASetIndexBuffer(&DrawList[i]->IndexBufferView());
//		CommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//		CD3DX12_GPU_DESCRIPTOR_HANDLE ObjCbvHandle;
//		int ObjCbvIndex = CurrFrameResourceIndex*DrawCount+i;//
//		ObjCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(CbvHeap->GetGPUDescriptorHandleForHeapStart());
//		ObjCbvHandle.Offset(ObjCbvIndex, CbvSrvUavDescriptorSize);
//		CommandList->SetGraphicsRootDescriptorTable(0, ObjCbvHandle);
//
//		CommandList->DrawIndexedInstanced((UINT)DrawList[i]->OutIndexBuffer.size(), 1, 0, 0, 0);
//	}
//	//---
//
//	CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(SwapChainBuffer[CurrBackBuffer].Get(),
//		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
//
//	CommandList->Close();
//	ID3D12CommandList* CmdsLists[] = { CommandList.Get() };
//	CommandQueue->ExecuteCommandLists(_countof(CmdsLists), CmdsLists);
//
//	SwapChain->Present(0, 0);
//	CurrBackBuffer = (CurrBackBuffer + 1) % SwapChainBufferCount;
//
//	CurrFrameResource->Fence = ++CurrentFence;
//	CommandQueue->Signal(Fence.Get(), CurrentFence);
//}

//void TAppWin32::Update(const GameTimer& Gt)
//{	
	////UpdateCamera(); //Store View_Matrix
	//OnKeyboardInput(Gt);

	//CurrFrameResourceIndex = (CurrFrameResourceIndex + 1) % FrameResourcesCount;
	//CurrFrameResource = FrameResources[CurrFrameResourceIndex].get();
	//if (CurrFrameResource->Fence != 0 && Fence->GetCompletedValue() < CurrFrameResource->Fence)
	//{
	//	HANDLE EventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
	//	ThrowIfFailed(Fence->SetEventOnCompletion(CurrFrameResource->Fence, EventHandle));
	//	WaitForSingleObject(EventHandle, INFINITE);
	//	CloseHandle(EventHandle);
	//}

	//////TimeConstants TimeConstant;
	//////TimeConstant.Time=Gt.TotalTime();
	//////CurrFrameResource->TimeCB->CopyData(0,TimeConstant);

	//DirectX::XMMATRIX v = MainCamera.GetView();
	//DirectX::XMMATRIX p = MainCamera.GetProj();
	//DirectX::XMMATRIX VP_Matrix = v * p;
	//PassConstants PasConstant;
	//PasConstant.Time = Gt.TotalTime();
	//XMStoreFloat4x4(&PasConstant.ViewProj_M, XMMatrixTranspose(VP_Matrix));//XMMATRIX赋值给XMFLOAT4X4
	//CurrFrameResource->PassCB->CopyData(0, PasConstant);//数据拷贝至GPU缓存


	//for (int i=0;i<DrawCount;i++)
	//{
	//	ObjectConstants ObjConstant;
	//	DirectX::XMMATRIX L = XMLoadFloat4x4(&DrawList[i]->Location_Matrix);
	//	DirectX::XMMATRIX R = XMLoadFloat4x4(&DrawList[i]->Rotation_Matrix);
	//	DirectX::XMMATRIX S = XMLoadFloat4x4(&DrawList[i]->Scale3D_Matrix);
	//	//DirectX::XMMATRIX W_Matrix = w ;//* DirectX::XMMatrixTranslation(i*300, i*300, 0.0f);
	//	//float trans=120;
	//	//L*= DirectX::XMMatrixTranslation(sin(Gt.TotalTime()) * trans, sin(Gt.TotalTime()) * trans, 0.0f);
	//	XMStoreFloat4x4(&ObjConstant.Location_M, XMMatrixTranspose(L));
	//	XMStoreFloat4x4(&ObjConstant.Rotation_M, XMMatrixTranspose(R));
	//	XMStoreFloat4x4(&ObjConstant.Scale3D_M, XMMatrixTranspose(S));
	//	CurrFrameResource->ObjectCB->CopyData(i, ObjConstant);
	//}
	//
//}

//void TAppWin32::OnResize()
//{
//	TAppWin::OnResize();

	//MainCamera.SetLens(0.25f * MathHelper::Pi, static_cast<float>(ClientWidth) / ClientHight, 1.0f, 10000.0f);//AspectRation()
//}

//{
	//EyePos.y = Radius * cosf(Phi);
	//EyePos.x = Radius * sinf(Phi) * cosf(Theta);
	//EyePos.z = Radius * sinf(Phi) * sinf(Theta);
	//
	//float r = 1.0f;
	////z = sqrt(r * r - x * x);
	//
	//XMVECTOR pos = XMVectorSet(EyePos.x, EyePos.y, EyePos.z, r);
	//XMVECTOR target = XMVectorZero();
	//XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	//XMMATRIX v = XMMatrixLookAtLH(pos, target, up);
	//XMStoreFloat4x4(&View_Matrix, v);
//}

//void TAppWin32::BuildFrameResource()
//{
//	for (int i = 0; i < FrameResourcesCount; i++)
//	{
//		FrameResources.push_back(std::make_unique<FrameResource>(D3dDevice.Get(),1, DrawCount));//PassCount ObjCount
//	}
//}
//
//void TAppWin32::BuildDescriptorHeaps()
//{
//	DescriptorsNum=(DrawCount+1)*FrameResourcesCount;//Meshs
//
//	D3D12_DESCRIPTOR_HEAP_DESC CbvHeapDesc;
//	CbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
//	CbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
//	//cbHeapDesc.NumDescriptors = 2* FrameResourcesCount;//包含CBV的数量,乘了3帧
//	CbvHeapDesc.NumDescriptors = DescriptorsNum;//Meshs
//	CbvHeapDesc.NodeMask = 0;
//	D3dDevice->CreateDescriptorHeap(&CbvHeapDesc, IID_PPV_ARGS(&CbvHeap));
//}
//
//void TAppWin32::BuildConstantBuffers()
//{	
//	UINT ObjCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
//	UINT PassCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(PassConstants));
//	UINT TimeCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(TimeConstants));
//
//	//前三组
//	//ObjCBV
//	for (int FrameIndex = 0; FrameIndex < FrameResourcesCount; FrameIndex++)
//	{
//		auto CurObjectCB= FrameResources[FrameIndex]->ObjectCB->Resource();
//		for (int i=0;i<DrawCount;i++)
//		{
//			D3D12_GPU_VIRTUAL_ADDRESS ObjCBAddress= CurObjectCB->GetGPUVirtualAddress();
//
//			ObjCBAddress += i* ObjCBByteSize;//缓冲区 第i个常量缓冲区偏移量 得到起始位置
//
//			int HeapIndex = FrameIndex*DrawCount+i;//CBV堆 当前帧当前物体对应CB的序号
//
//			auto Handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(CbvHeap->GetCPUDescriptorHandleForHeapStart());//获得CBV堆首地址
//			Handle.Offset(HeapIndex, CbvSrvUavDescriptorSize);
//
//			D3D12_CONSTANT_BUFFER_VIEW_DESC CbvDesc;
//			CbvDesc.BufferLocation = ObjCBAddress;
//			CbvDesc.SizeInBytes = ObjCBByteSize;
//			D3dDevice->CreateConstantBufferView(&CbvDesc, Handle);
//		}
//
//		//D3D12_GPU_VIRTUAL_ADDRESS ObjCBAddress = FrameResources[FrameIndex]->ObjectCB->Resource()->GetGPUVirtualAddress();
//		//int ObjCBElementIndex = 0;
//		//ObjCBAddress += ObjCBElementIndex * ObjCBByteSize;
//		//int HeapIndex = FrameIndex;
//		//
//		//auto Handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(CbvHeap->GetCPUDescriptorHandleForHeapStart());//获得CBV堆首地址
//		//Handle.Offset(HeapIndex, CbvSrvUavDescriptorSize);
//		//
//		//D3D12_CONSTANT_BUFFER_VIEW_DESC CbvDesc;
//		//CbvDesc.BufferLocation = ObjCBAddress;
//		//CbvDesc.SizeInBytes = ObjCBByteSize;  // D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
//		//D3dDevice->CreateConstantBufferView(&CbvDesc, Handle);
//	}
//	//int HeapIndex;
//	//
//	//ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(D3dDevice.Get(), 1, true);
//	//D3D12_GPU_VIRTUAL_ADDRESS ObjCBAddress = ObjectCB->Resource()->GetGPUVirtualAddress();//首地址
//	////UINT ObjCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
//	//int ObjCBElementIndex = 0;                        //-=-
//	//ObjCBAddress += ObjCBElementIndex * ObjCBByteSize;//-=-
//	//HeapIndex = 0;
//	// 
//	//auto Handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(CbvHeap->GetCPUDescriptorHandleForHeapStart());//获得CBV堆首地址
//	//Handle.Offset(HeapIndex, CbvSrvUavDescriptorSize);
//	//
//	//D3D12_CONSTANT_BUFFER_VIEW_DESC CbvDesc0;
//	//CbvDesc0.BufferLocation = ObjCBAddress;
//	//CbvDesc0.SizeInBytes = ObjCBByteSize;  // D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
//	//D3dDevice->CreateConstantBufferView(&CbvDesc0, Handle);
//	
//
//	//三个
//	//PassCBV
//	for (int FrameIndex = 0; FrameIndex < FrameResourcesCount; FrameIndex++)
//	{
//		//
//		//D3D12_GPU_VIRTUAL_ADDRESS PassCBAddress = FrameResources[FrameIndex]->PassCB->Resource()->GetGPUVirtualAddress();
//		//int PassCBElementIndex = 0;
//		//PassCBAddress += PassCBElementIndex * PassCBByteSize;
//		//int HeapIndex = FrameResourcesCount+ FrameIndex;
//		//
//		//auto Handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(CbvHeap->GetCPUDescriptorHandleForHeapStart());//获得CBV堆首地址
//		//Handle.Offset(HeapIndex, CbvSrvUavDescriptorSize);
//		//
//		//D3D12_CONSTANT_BUFFER_VIEW_DESC CbvDesc1;
//		//CbvDesc1.BufferLocation = PassCBAddress;
//		//CbvDesc1.SizeInBytes = PassCBByteSize;  // D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
//		//D3dDevice->CreateConstantBufferView(&CbvDesc1, Handle);
//
//		D3D12_GPU_VIRTUAL_ADDRESS PassCBAddress = FrameResources[FrameIndex]->PassCB->Resource()->GetGPUVirtualAddress();
//		int HeapIndex = DrawCount*FrameResourcesCount+ FrameIndex;
//		
//		auto Handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(CbvHeap->GetCPUDescriptorHandleForHeapStart());//获得CBV堆首地址
//		Handle.Offset(HeapIndex, CbvSrvUavDescriptorSize);
//		
//		D3D12_CONSTANT_BUFFER_VIEW_DESC CbvDesc;
//		CbvDesc.BufferLocation = PassCBAddress;
//		CbvDesc.SizeInBytes = PassCBByteSize;  // D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
//		D3dDevice->CreateConstantBufferView(&CbvDesc, Handle);	
//	}	
//	//PassCB = std::make_unique<UploadBuffer<PassConstants>>(D3dDevice.Get(), 1, true);
//	//D3D12_GPU_VIRTUAL_ADDRESS PassCBAddress = PassCB->Resource()->GetGPUVirtualAddress();
//	////UINT PassCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(PassConstants));
//	//int PassCBElementIndex = 0;
//	//PassCBAddress += PassCBElementIndex* PassCBByteSize;
//	//HeapIndex=1;
//	// 
//	////Handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(CbvHeap->GetCPUDescriptorHandleForHeapStart());//获得CBV堆首地址
//	//Handle.Offset(HeapIndex, CbvSrvUavDescriptorSize);
//	// 
//	//D3D12_CONSTANT_BUFFER_VIEW_DESC CbvDesc1;
//	//CbvDesc1.BufferLocation = PassCBAddress;
//	//CbvDesc1.SizeInBytes = PassCBByteSize;  // D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
//	//D3dDevice->CreateConstantBufferView(&CbvDesc1, Handle);
//
//
//	//for (int FrameIndex = 0; FrameIndex < FrameResourcesCount; FrameIndex++)
//	//{
//	//	D3D12_GPU_VIRTUAL_ADDRESS TimeCBAddress = FrameResources[FrameIndex]->TimeCB->Resource()->GetGPUVirtualAddress();
//	//	int HeapIndex = DrawCount * FrameResourcesCount + FrameResourcesCount + FrameIndex;
//	//	auto Handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(CbvHeap->GetCPUDescriptorHandleForHeapStart());//获得CBV堆首地址
//	//	Handle.Offset(HeapIndex, CbvSrvUavDescriptorSize);
//	//
//	//	D3D12_CONSTANT_BUFFER_VIEW_DESC CbvDesc;
//	//	CbvDesc.BufferLocation = TimeCBAddress;
//	//	CbvDesc.SizeInBytes = TimeCBByteSize;  // D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
//	//	D3dDevice->CreateConstantBufferView(&CbvDesc, Handle);
//	//}
//}
//
//void TAppWin32::BuildRootSignature()
//{
//	//RootSignature
//	CD3DX12_ROOT_PARAMETER SlotRootParameter[2];//根参数
//
//	CD3DX12_DESCRIPTOR_RANGE CbvTable0;
//	CbvTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,1,0);
//	CD3DX12_DESCRIPTOR_RANGE CbvTable1;
//	CbvTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
//	//CD3DX12_DESCRIPTOR_RANGE CbvTable2;
//	//CbvTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);
//
//	SlotRootParameter[0].InitAsDescriptorTable(1, &CbvTable0);
//	SlotRootParameter[1].InitAsDescriptorTable(1, &CbvTable1);
//	//SlotRootParameter[2].InitAsDescriptorTable(1, &CbvTable2);
//
//	CD3DX12_ROOT_SIGNATURE_DESC RootSigDesc(
//		2,//3个根参数
//		SlotRootParameter,//根参数指针
//		0,nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
//
//	ComPtr<ID3DBlob> SerializedRootSig = nullptr;
//	ComPtr<ID3DBlob> ErrorBlob = nullptr;
//	D3D12SerializeRootSignature(&RootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,SerializedRootSig.GetAddressOf(), ErrorBlob.GetAddressOf());
//	D3dDevice->CreateRootSignature(0,SerializedRootSig->GetBufferPointer(),SerializedRootSig->GetBufferSize(),IID_PPV_ARGS(&RootSignature));
//
//}
//
//void TAppWin32::BuildShadersAndInputLayout()
//{
//	MvsByteCode[0] = CompileShader(L"Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
//	MpsByteCode[0] = CompileShader(L"Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");
//
//	MvsByteCode[1] = CompileShader(L"Shaders\\colorOffset.hlsl", nullptr, "VS", "vs_5_0");
//	MpsByteCode[1] = CompileShader(L"Shaders\\colorOffset.hlsl", nullptr, "PS", "ps_5_0");
//
//	InputLayout =
//	{
//		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
//		,{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
//	};
//}
//
//void TAppWin32::BuildGeometry()
//{	
//	std::vector<MeshInfo> MeshsData;
//	D3DUtil::ReadMapFile("StaticMesh\\Map1.Usmh",MeshsData);
//
//	//D3DUtil::ReadMeshFile("StaticMesh\\Shape_QuadPyramid.Usmh", OutVertexBuffer, OutIndexBuffer);
//	//std::vector<MeshInfo> MeshVector;
//	//D3DUtil::ReadMapFile("StaticMesh\\Map.Usmh", MeshVector);
//	//std::stringstream ss;
//	//for (const auto& Mesh : MeshVector)
//	//{
//	//	ss << Mesh.MeshName << "\n";
//	//}
//	//OutputDebugStringA(ss.str().c_str());
//
//	for (auto MeshData : MeshsData)
//	{
//		auto Mesh = std::make_shared<MeshGeometry>(MeshData);
//
//		D3DCreateBlob(Mesh->VbByteSize, &Mesh->VertexBufferCPU);
//		D3DCreateBlob(Mesh->IbByteSize, &Mesh->IndexBufferCPU);
//
//		CopyMemory(Mesh->IndexBufferCPU->GetBufferPointer(), MeshData.Indices.data(), Mesh->IbByteSize);
//		CopyMemory(Mesh->VertexBufferCPU->GetBufferPointer(), MeshData.Vertices.data(), Mesh->VbByteSize);
//
//		Mesh->VertexBufferGPU = CreateDefaultBuffer(D3dDevice.Get(), CommandList.Get(), MeshData.Vertices.data(), Mesh->VbByteSize, Mesh->VertexBufferUploader);
//		Mesh->IndexBufferGPU = CreateDefaultBuffer(D3dDevice.Get(), CommandList.Get(), MeshData.Indices.data(), Mesh->IbByteSize, Mesh->IndexBufferUploader);
//
//		DrawList.push_back(Mesh);
//		DrawCount++;
//	}
//}
//
//void TAppWin32::BuildPSO()
//{
//	D3D12_GRAPHICS_PIPELINE_STATE_DESC PsoDesc;
//	ZeroMemory(&PsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
//	PsoDesc.InputLayout = { InputLayout.data(), (UINT)InputLayout.size() };
//	PsoDesc.pRootSignature = RootSignature.Get();
//	PsoDesc.VS ={reinterpret_cast<BYTE*>(MvsByteCode[0]->GetBufferPointer()),MvsByteCode[0]->GetBufferSize()};
//	PsoDesc.PS ={reinterpret_cast<BYTE*>(MpsByteCode[0]->GetBufferPointer()),MpsByteCode[0]->GetBufferSize()};
//
//	PsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
//	//PsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;  //线框模式
//
//	PsoDesc.RasterizerState.FrontCounterClockwise = TRUE;
//	PsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
//	PsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
//	PsoDesc.SampleMask = UINT_MAX;
//	PsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
//	PsoDesc.NumRenderTargets = 1;
//	PsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;//DXGI_FORMAT mBackBufferFormat
//	PsoDesc.SampleDesc.Count = 1;//m4xMsaaState ? 4 : 1;
//	PsoDesc.SampleDesc.Quality = 0;//m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
//	PsoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;//DXGI_FORMAT mDepthStencilFormat
//
//	auto Pso= ComPtr<ID3D12PipelineState>();
//	PSOs.push_back(Pso);
//	D3dDevice->CreateGraphicsPipelineState(&PsoDesc, IID_PPV_ARGS(&PSOs[0]));
//
//	PsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;  //线框模式
//	PsoDesc.VS = { reinterpret_cast<BYTE*>(MvsByteCode[1]->GetBufferPointer()),MvsByteCode[1]->GetBufferSize() };
//	PsoDesc.PS = { reinterpret_cast<BYTE*>(MpsByteCode[1]->GetBufferPointer()),MpsByteCode[1]->GetBufferSize() };
//
//	PSOs.push_back(Pso);
//	D3dDevice->CreateGraphicsPipelineState(&PsoDesc, IID_PPV_ARGS(&PSOs[1]));
//}

//ComPtr<ID3DBlob> TAppWin32::CompileShader(const std::wstring& Filename, const D3D_SHADER_MACRO* Defines, const std::string& EntryPoint, const std::string& Target)
//{
//	ComPtr<ID3DBlob> ByteCode;
//	ComPtr<ID3DBlob> Errors;
//	D3DCompileFromFile(Filename.c_str(), Defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,EntryPoint.c_str(), Target.c_str(), 0, 0, &ByteCode, &Errors);
//	return ByteCode;
//}

//void TAppWin32::OnMouseDown(WPARAM btnState, int x, int y)
//{
//	LastMousePos.x = x;
//	LastMousePos.y = y;
//
//	SetCapture(HMainWnd);
//}
//
//void TAppWin32::OnMouseUp(WPARAM btnState, int x, int y)
//{
//	ReleaseCapture();
//}
//
//void TAppWin32::OnMouseMove(WPARAM btnState, int x, int y)
//{
//	if ((btnState & MK_RBUTTON) != 0)//MK_LBUTTON
//	{
//		float dx = DirectX::XMConvertToRadians(0.25f * static_cast<float>(x - LastMousePos.x));
//		float dy = DirectX::XMConvertToRadians(0.25f * static_cast<float>(y - LastMousePos.y));
//
//		MainCamera.Pitch(dy);
//		MainCamera.RotateZ(dx);
//		//
//		//Theta += dx;
//		//Phi += dy;
//		//Phi = MathHelper::Clamp(Phi, 0.1f, MathHelper::Pi - 0.1f);
//
//	}
//	//else if ((btnState & MK_RBUTTON) != 0)
//	//{
//	//	float dx = 0.005f * static_cast<float>(x - LastMousePos.x);
//	//	float dy = 0.005f * static_cast<float>(y - LastMousePos.y);
//
//	//	Radius += (dx - dy) * 25;
//	//	Radius = MathHelper::Clamp(Radius, 300.0f, 1000.0f);
//	//}
//
//
//	LastMousePos.x = x;
//	LastMousePos.y = y;
//}

//void TAppWin32::OnKeyboardInput(const GameTimer& Gt)
//{
	//float Speed=1500;
	//const float Dt=Gt.DeltaTime();

	//if (GetAsyncKeyState('W') & 0x8000)
	//	MainCamera.Walk(Speed * Dt);

	//if (GetAsyncKeyState('S') & 0x8000)
	//	MainCamera.Walk(-Speed * Dt);

	//if (GetAsyncKeyState('A') & 0x8000)
	//	MainCamera.Strafe(-Speed * Dt);

	//if (GetAsyncKeyState('D') & 0x8000)
	//	MainCamera.Strafe(Speed * Dt);

	//MainCamera.UpdateViewMatrix();
//}


