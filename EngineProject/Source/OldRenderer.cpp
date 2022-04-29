#include "stdafx.h"
#include "Engine.h"
#include "OldRenderer.h"
#include "RHI/RHI.h"


OldRenderer::OldRenderer()
{

}

bool OldRenderer::Init()
{
	//RHI::Get()->Init();
	InitDirect3D();
	return true;
}

void OldRenderer::Render()
{	
	RendererUpdate(Engine::Get()->GetTimer());
	Draw();
}

bool OldRenderer::InitDirect3D()
{
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

	OutputDebugStringA("DxInit Success\n");
	return true;
}

bool OldRenderer::InitDraw()
{
	FlushCommandQueue();
	CommandList->Reset(CommandListAlloc.Get(), nullptr);

	//BuildGeometry();
	BuildRenderData();
	BuildTextures();
	BuildFrameResource();
	BuildDescriptorHeaps();
	BuildConstantBuffers();

	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildPSO();

	//ExecuteCommandLists
	ThrowIfFailed(CommandList->Close());
	ID3D12CommandList* cmdLists[] = { CommandList.Get() };
	CommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);//送入命令队列


	FlushCommandQueue();

	return true;
}


//ok
void OldRenderer::RendererUpdate(const GameTimer* Gt)
{
	//帧资源变下一帧
	CurrFrameResourceIndex = (CurrFrameResourceIndex + 1) % OldFrameResourcesCount;
	CurrFrameResource = FrameResources[CurrFrameResourceIndex].get();

	//
	if (CurrFrameResource->Fence != 0 && Fence->GetCompletedValue() < CurrFrameResource->Fence)//当前帧不是第一次 且 GPU未完成此帧 等待
	{
		HANDLE EventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(Fence->SetEventOnCompletion(CurrFrameResource->Fence, EventHandle));
		WaitForSingleObject(EventHandle, INFINITE);
		CloseHandle(EventHandle);
	}

	
	//以下部分转Task

	//相机位置 时间
	glm::mat4 v = Engine::Get()->GetScene()->GetMainCamera().GetView();
	glm::mat4 p = Engine::Get()->GetScene()->GetMainCamera().GetProj();
	glm::mat4 VP_Matrix = p * v;

	PassConstants PasConstant;
	PasConstant.Time = Gt->TotalTime();
	PasConstant.ViewProj_M = glm::transpose(VP_Matrix);

	CurrFrameResource->PassCB->CopyData(0, PasConstant);//数据拷贝至GPU缓存


	//物体位置
	for (int i = 0; i < DrawCount; i++)
	{
		ObjectConstants ObjConstant;
		glm::mat4 L = DrawList[i].Location_Matrix;
		glm::mat4 R = DrawList[i].Rotation_Matrix;
		glm::mat4 S = DrawList[i].Scale3D_Matrix;

		ObjConstant.Location_M= glm::transpose(L);
		ObjConstant.Rotation_M= glm::transpose(R);
		ObjConstant.Scale3D_M= glm::transpose(S);
		CurrFrameResource->ObjectCB->CopyData(i, ObjConstant);
	}

	//for (int i = 0; i < DrawCount; i++)
	//{
	//	MaterialConstants MatConstant;
	//	CurrFrameResource->MaterialCB->CopyData(i, MatConstant);
	//}

}

void OldRenderer::Draw()
{
	//Render Begin----------------------------------------------------
	auto CurrentAllocator = CurrFrameResource->CmdListAlloc;//
	CurrentAllocator->Reset();//
	CommandList->Reset(CurrentAllocator.Get(), PSOs[0].Get());//********* nullptr now


	//Set Render Target Begin----------------------------------------------------------------------------------------
	//后台缓冲资源从呈现状态转换到渲染目标状态
	CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(SwapChainBuffers[CurrBackBuffer].Get(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	//设置视口和裁剪矩形
	CommandList->RSSetViewports(1, &ScreenViewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);

	//清除后台缓冲区和深度缓冲区，并赋值
	CommandList->ClearRenderTargetView(CurrentBackBufferView(), DirectX::Colors::Black, 0, nullptr);
	CommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	//指定将要渲染的缓冲区，指定RTV和DSV
	CommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());



	//PrepareBufferHeap------------------------------------
	ID3D12DescriptorHeap* DescriptorHeaps[] = { CbvHeap.Get() };
	CommandList->SetDescriptorHeaps(_countof(DescriptorHeaps), DescriptorHeaps);





	CommandList->SetGraphicsRootSignature(RootSignature.Get());


	int PasCbvIndex = OldFrameResourcesCount * DrawCount + CurrFrameResourceIndex;//3*n+Curr
	auto Handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(CbvHeap->GetGPUDescriptorHandleForHeapStart());
	Handle.Offset(PasCbvIndex, CbvSrvUavDescriptorSize);

	CommandList->SetGraphicsRootDescriptorTable(1, Handle);

	//RootDescriptorTable(根描述符表) 1
	//RootConstant(根常量) 1
	//RootDescriptor(根描述符) 2
	// 
	//设置根描述符,将根描述符与资源绑定
	//auto PCbvAdress = FrameResources[CurrFrameResourceIndex]->PassCB->Resource()->GetGPUVirtualAddress();
	//PCbvAdress+= CbvSrvUavDescriptorSize* PasCbvIndex;
	//CommandList->SetGraphicsRootConstantBufferView(1, PCbvAdress);

	for (int i = 0; i < DrawCount; i++)
	{	
		if(!CanFindRMesh(DrawList[i].MeshName))
			continue;

		DXMesh DrawMesh= FindRMesh(DrawList[i].MeshName);

		CommandList->SetPipelineState(PSOs[0].Get());

		if (DrawMesh.Indices.size() == 2304)
			CommandList->SetPipelineState(PSOs[1].Get());

		CommandList->IASetVertexBuffers(0, 1, &DrawMesh.VertexBufferView());
		CommandList->IASetIndexBuffer(&DrawMesh.IndexBufferView());
		CommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		CD3DX12_GPU_DESCRIPTOR_HANDLE ObjCbvHandle;
		int ObjCbvIndex = CurrFrameResourceIndex * DrawCount + i;//
		ObjCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(CbvHeap->GetGPUDescriptorHandleForHeapStart());
		ObjCbvHandle.Offset(ObjCbvIndex, CbvSrvUavDescriptorSize);

		CommandList->SetGraphicsRootDescriptorTable(0, ObjCbvHandle);

		//CD3DX12_GPU_DESCRIPTOR_HANDLE MatCbvHandle;
		//int MatCbvIndex = CurrFrameResourceIndex * DrawCount + i;//
		//MatCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(CbvHeap->GetGPUDescriptorHandleForHeapStart());
		//MatCbvHandle.Offset(MatCbvIndex, CbvSrvUavDescriptorSize);

		//auto ObjCbvAdress = FrameResources[CurrFrameResourceIndex]->ObjectCB->Resource()->GetGPUVirtualAddress();
		////ObjCbvAdress += CbvSrvUavDescriptorSize* ObjCbvIndex;
		//CommandList->SetGraphicsRootConstantBufferView(2, );


		//auto PCbvAdress = FrameResources[CurrFrameResourceIndex]->PassCB->Resource()->GetGPUVirtualAddress();
		//PCbvAdress+= CbvSrvUavDescriptorSize* PasCbvIndex;
		//CommandList->SetGraphicsRootConstantBufferView(1, PCbvAdress);

		CommandList->DrawIndexedInstanced((UINT)DrawMesh.Indices.size(), 1, 0, 0, 0);//换为 同理
	}




	//Set Render Target End----------------------------------------
	//后台缓冲区的状态改成呈现状态
	CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(SwapChainBuffers[CurrBackBuffer].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));






	//End Frame --------------------End Frame ------------End Frame ---
	CommandList->Close();//
	ID3D12CommandList* CmdsLists[] = { CommandList.Get() };//
	CommandQueue->ExecuteCommandLists(_countof(CmdsLists), CmdsLists);//

	SwapChain->Present(0, 0);//
	CurrBackBuffer = (CurrBackBuffer + 1) % SwapChainBufferCount;//

	CurrFrameResource->Fence = ++CurrentFence;//
	CommandQueue->Signal(Fence.Get(), CurrentFence);//

}

//ok
void OldRenderer::RendererReset()
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




void OldRenderer::BuildGeometry()
{
	//std::vector<MapItem> MeshsData;
	//D3DUtil::ReadMapFile("StaticMesh\\Map1.Usmh", MeshsData);
	//
	//for (auto MeshData : MeshsData)
	//{
	//	auto OMesh = std::make_shared<RenderMesh>(MeshData);
	//
	//	OMesh->BuildDefaultBuffer(D3dDevice.Get(), CommandList.Get());
	//
	//	DrawList.push_back(OMesh);//绘制项 
	//	DrawCount++;
	//}
}

void OldRenderer::BuildRenderData()
{
	DrawList.clear();
	DXMeshs.clear();
	DrawList = Engine::Get()->GetScene()->GetSceneMeshActors();
	DrawCount = DrawList.size();

	for (auto DrawListActor : DrawList)
	{
		Mesh AMesh;

		bool meshFinded = Engine::Get()->GetAssetsManager()->FindMesh(DrawListActor.MeshName, AMesh);

		if (!meshFinded||DXMeshs.count(DrawListActor.MeshName))
			continue;		

		DXMesh ARenderMesh(AMesh);
	
		ARenderMesh.BuildDefaultBuffer(D3dDevice.Get(), CommandList.Get());
		DXMeshs.insert(std::make_pair<>(DrawListActor.MeshName,ARenderMesh));//std::string,RenderMesh
	}
}





void OldRenderer::BuildTextures()
{
	Texture ATexture;
	for (auto DrawListActor : DrawList)
	{
		Material AMaterial;
		bool MaterialFinded = Engine::Get()->GetAssetsManager()->FindMaterial(DrawListActor.MaterialName, AMaterial);
		if (!MaterialFinded || DXMaterials.count(DrawListActor.MaterialName))//没有此材质 或 已经加载到
			continue;
		DXMaterials[AMaterial.Name] = AMaterial;

		Texture Atexture;
		bool TextureFinded = Engine::Get()->GetAssetsManager()->FindTexture(AMaterial.TextureName, Atexture);
		if (!TextureFinded || DXTextures.count(Atexture.Name))
			continue;

		DXTexture ADxtexture(Atexture);
		ThrowIfFailed(
			DirectX::CreateDDSTextureFromFile12(D3dDevice.Get(), CommandList.Get(), ADxtexture.Filename.c_str(), ADxtexture.Resource, ADxtexture.UploadHeap));
		DXTextures[ADxtexture.Name] = ADxtexture;

	}
	//遍历绘制列表里Actor 对应材质 材质使用了贴图
	//RenderScene 里的 Texture
	//进行 CreateDDSTextureFromFile12
	MaterialCount= DXMaterials.size();
	TextureCount= DXTextures.size();
}


void OldRenderer::BuildFrameResource()
{
	FrameResources.resize(OldFrameResourcesCount);
	for (int i = 0; i < OldFrameResourcesCount; i++)
	{
		FrameResources[i] = std::make_unique<FrameResource>(D3dDevice.Get());
		FrameResources[i]->Init(1, DrawCount, MaterialCount);
	}
}


//这里是提前知道了 所需大小 创建的，所以比较晚。
void OldRenderer::BuildDescriptorHeaps()
{
	DescriptorsNum = (DrawCount + 1 + MaterialCount) * OldFrameResourcesCount;//Meshs

	D3D12_DESCRIPTOR_HEAP_DESC CbvHeapDesc;
	CbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	CbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	//cbHeapDesc.NumDescriptors = 2* FrameResourcesCount;//包含CBV的数量,乘了3帧
	CbvHeapDesc.NumDescriptors = DescriptorsNum;//Meshs
	CbvHeapDesc.NodeMask = 0;
	D3dDevice->CreateDescriptorHeap(&CbvHeapDesc, IID_PPV_ARGS(&CbvHeap));
}



void OldRenderer::BuildConstantBuffers()
{
	UINT ObjCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	UINT PassCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(PassConstants));
	UINT MatCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

	int HeapIndexN = 0;
	//前三组
	//ObjCBV
	for (int FrameIndex = 0; FrameIndex < OldFrameResourcesCount; FrameIndex++)
	{
		auto CurObjectCB = FrameResources[FrameIndex]->ObjectCB->Resource();
		for (int i = 0; i < DrawCount; i++)
		{
			D3D12_GPU_VIRTUAL_ADDRESS ObjCBAddress = CurObjectCB->GetGPUVirtualAddress();

			ObjCBAddress += i * ObjCBByteSize;//缓冲区 第i个常量缓冲区偏移量 得到起始位置

			int HeapIndex = FrameIndex * DrawCount + i;//CBV堆 当前帧当前物体对应CB的序号
			HeapIndexN++;

			auto Handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(CbvHeap->GetCPUDescriptorHandleForHeapStart());//获得CBV堆首地址
			Handle.Offset(HeapIndex, CbvSrvUavDescriptorSize);

			D3D12_CONSTANT_BUFFER_VIEW_DESC CbvDesc;
			CbvDesc.BufferLocation = ObjCBAddress;
			CbvDesc.SizeInBytes = ObjCBByteSize;
			D3dDevice->CreateConstantBufferView(&CbvDesc, Handle);
		}
	}

	//3帧3个
	for (int FrameIndex = 0; FrameIndex < OldFrameResourcesCount; FrameIndex++)
	{
		D3D12_GPU_VIRTUAL_ADDRESS PassCBAddress = FrameResources[FrameIndex]->PassCB->Resource()->GetGPUVirtualAddress();
		int HeapIndex = DrawCount * OldFrameResourcesCount + FrameIndex;
		HeapIndexN++;

		auto Handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(CbvHeap->GetCPUDescriptorHandleForHeapStart());//获得CBV堆首地址
		Handle.Offset(HeapIndex, CbvSrvUavDescriptorSize);

		D3D12_CONSTANT_BUFFER_VIEW_DESC CbvDesc;
		CbvDesc.BufferLocation = PassCBAddress;
		CbvDesc.SizeInBytes = PassCBByteSize;  // D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
		D3dDevice->CreateConstantBufferView(&CbvDesc, Handle);
	}

	//for (int FrameIndex = 0; FrameIndex < FrameResourcesCount; FrameIndex++)
	//{
	//	auto CurMaterialCB = FrameResources[FrameIndex]->MaterialCB->Resource();
	//	for (int i = 0; i < MaterialCount; i++)
	//	{
	//		D3D12_GPU_VIRTUAL_ADDRESS MatCBAddress = CurMaterialCB->GetGPUVirtualAddress();
	//		MatCBAddress += i * MatCBByteSize;
	//
	//		int HeapIndex = MaterialCount * FrameResourcesCount + 3 + FrameIndex * MaterialCount + i;//CBV堆 当前帧当前物体对应CB的序号
	//		HeapIndexN++;
	//
	//		auto Handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(CbvHeap->GetCPUDescriptorHandleForHeapStart());//获得CBV堆首地址
	//		Handle.Offset(HeapIndex, CbvSrvUavDescriptorSize);
	//
	//		D3D12_CONSTANT_BUFFER_VIEW_DESC CbvDesc;
	//		CbvDesc.BufferLocation = MatCBAddress;
	//		CbvDesc.SizeInBytes = MatCBByteSize;
	//		D3dDevice->CreateConstantBufferView(&CbvDesc, Handle);
	//	}
	//}
}

void OldRenderer::BuildRootSignature()
{
	//RootSignature
	CD3DX12_ROOT_PARAMETER SlotRootParameter[3];//根参数

	CD3DX12_DESCRIPTOR_RANGE CbvTable0;
	CbvTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	CD3DX12_DESCRIPTOR_RANGE CbvTable1;
	CbvTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

	SlotRootParameter[0].InitAsDescriptorTable(1, &CbvTable0);
	SlotRootParameter[1].InitAsDescriptorTable(1, &CbvTable1);

	SlotRootParameter[2].InitAsConstantBufferView(2);
	//SlotRootParameter[1].InitAsConstantBufferView(1);


	CD3DX12_ROOT_SIGNATURE_DESC RootSigDesc(
		3,//3个根参数
		SlotRootParameter,//根参数指针
		0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> SerializedRootSig = nullptr;
	ComPtr<ID3DBlob> ErrorBlob = nullptr;
	D3D12SerializeRootSignature(&RootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, SerializedRootSig.GetAddressOf(), ErrorBlob.GetAddressOf());
	D3dDevice->CreateRootSignature(0, SerializedRootSig->GetBufferPointer(), SerializedRootSig->GetBufferSize(), IID_PPV_ARGS(&RootSignature));

}

void OldRenderer::BuildShadersAndInputLayout()
{
	MvsByteCode[0] = D3DUtil::CompileShader(L"Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
	MpsByteCode[0] = D3DUtil::CompileShader(L"Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");

	MvsByteCode[1] = D3DUtil::CompileShader(L"Shaders\\colorOffset.hlsl", nullptr, "VS", "vs_5_0");
	MpsByteCode[1] = D3DUtil::CompileShader(L"Shaders\\colorOffset.hlsl", nullptr, "PS", "ps_5_0");

	InputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		,{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		,{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void OldRenderer::BuildPSO()
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




DXMesh OldRenderer::FindRMesh(std::string MeshName)
{
	return DXMeshs.at(MeshName);
	//for(auto it : DrawMeshList)
	//{
	//	if(it.first == MeshName)
	//	{
	//		return it.second;
	//	}
	//}
}

bool OldRenderer::CanFindRMesh(std::string MeshName)
{
	if (DXMeshs.count(MeshName))
	{
		return true;
	}
	//OutputDebugStringA(("Cant Find RenderMesh: "+MeshName+"  \n").c_str());
	return false;
}































void OldRenderer::CreateDevice()
{
	//Create IDXGIFactory
	CreateDXGIFactory1(IID_PPV_ARGS(&DxgiFactory));

	//Create Hardware Device.
	HRESULT hardwareResult = D3D12CreateDevice(
		nullptr,             // default adapter
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&D3dDevice));
}

void OldRenderer::CreateFence()
{
	D3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence));
}

void OldRenderer::GetDescriptorSize()
{
	RtvDescriptorSize = D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	DsvDescriptorSize = D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	CbvSrvUavDescriptorSize = D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void OldRenderer::SetMSAA()
{
	MsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	MsaaQualityLevels.SampleCount = 1;
	MsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	MsaaQualityLevels.NumQualityLevels = 0;
	D3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &MsaaQualityLevels, sizeof(MsaaQualityLevels));
	assert(MsaaQualityLevels.NumQualityLevels > 0);
}

void OldRenderer::CreateCommandObject()
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

void OldRenderer::CreateSwapChain()
{
	HMainWnd = Engine::Get()->GetWindow()->GetWnd();

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


void OldRenderer::FlushCommandQueue()
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

void OldRenderer::CreateViewPortAndScissorRect()
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

D3D12_CPU_DESCRIPTOR_HANDLE OldRenderer::DepthStencilView()
{
	return DsvHeap->GetCPUDescriptorHandleForHeapStart();
}



//DescriptorHeap chouxiang
void OldRenderer::CreateDescriptorHeap()
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
	DsvHeapDesc.NumDescriptors = 1;//1
	DsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	DsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DsvHeapDesc.NodeMask = 0;
	D3dDevice->CreateDescriptorHeap(&DsvHeapDesc, IID_PPV_ARGS(DsvHeap.GetAddressOf()));
}

void OldRenderer::CreateRTV()
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

void OldRenderer::CreateDSV()
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


D3D12_CPU_DESCRIPTOR_HANDLE OldRenderer::CurrentBackBufferView()
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		RtvHeap->GetCPUDescriptorHandleForHeapStart(),
		CurrBackBuffer,
		RtvDescriptorSize);
}
