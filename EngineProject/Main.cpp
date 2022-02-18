#include "Head.h"
#include <WindowsX.h>

using Microsoft::WRL::ComPtr;
using namespace std;
using namespace DirectX;


const int SwapChainBufferCount = 2;
int CurrBackBuffer = 0;
UINT64 CurrentFence = 0;
ComPtr<ID3D12Device> D3dDevice;
ComPtr<ID3D12Resource> DepthStencilBuffer;
ComPtr<ID3D12DescriptorHeap> DsvHeap;
ComPtr<ID3D12DescriptorHeap> RtvHeap;
ComPtr<ID3D12DescriptorHeap> CbvHeap;
ComPtr<ID3D12CommandQueue> CommandQueue;
ComPtr<ID3D12GraphicsCommandList> CommandList;
ComPtr<ID3D12Fence> Fence;
ComPtr<ID3DBlob> MvsByteCode=nullptr;
ComPtr<ID3DBlob> MpsByteCode=nullptr;
ComPtr<ID3D12RootSignature> MRootSignature;

UINT RtvDescriptorSize;

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

array<Vertex,8>Vertices=
{
		Vertex({ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::White) }),
		Vertex({ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Red) }),
		Vertex({ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Green) }),
		Vertex({ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Blue) }),
		Vertex({ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Yellow) }),
		Vertex({ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Cyan) }),
		Vertex({ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Magenta) })
};

std::array<std::uint16_t, 36> Indices =
{
	// front face
	0, 1, 2,
	0, 2, 3,

	// back face
	4, 6, 5,
	4, 7, 6,

	// left face
	4, 5, 1,
	4, 1, 0,

	// right face
	3, 2, 6,
	3, 6, 7,

	// top face
	1, 5, 6,
	1, 6, 2,

	// bottom face
	4, 0, 3,
	4, 3, 7
};

struct ObjectConstants
{
	XMFLOAT4X4 WorldViewProj = XMFLOAT4X4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
};



D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView();
D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView();
void FlushCommandQueue();
ComPtr<ID3D12Resource> CreateDefaultBuffer(ID3D12Device* Device,ID3D12GraphicsCommandList* CmdList, const void* InitData, UINT64 ByteSize, ComPtr<ID3D12Resource>& UploadBuffer);
void ShadersAndInputLayout();

LRESULT CALLBACK MainWindowProc(HWND Window, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		break;
	}
	return DefWindowProc(Window, Msg, wParam, lParam);;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,PSTR cmdLine, int showCmd)
{
	//Window
	WNDCLASS WndClass;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = MainWindowProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIcon(0, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(0, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);//NULL_BRUSH
	WndClass.lpszMenuName = 0;
	WndClass.lpszClassName = L"MainWnd";
	if (!RegisterClass(&WndClass))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}
	int ClientWidth = 800;
	int ClientHeight = 600;
	RECT R = { 0, 0, ClientWidth, ClientHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int Width = R.right - R.left;
	int Height = R.bottom - R.top;

	std::wstring MainWndCaption = L"d3d xxx";
	HWND HMainWnd = CreateWindow(L"MainWnd", MainWndCaption.c_str(),WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, Width, Height, 0, 0, hInstance, 0);
	ShowWindow(HMainWnd, SW_SHOW);
	UpdateWindow(HMainWnd);

	//Enable Debug
	ComPtr<ID3D12Debug> DebugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController))))
	{
		DebugController->EnableDebugLayer();
	}
	else
	{
		MessageBox(HMainWnd, L"Debug Failed.", 0,0);
	}

	//Create IDXGIFactory
	ComPtr<IDXGIFactory> DxgiFactory;
	CreateDXGIFactory1(IID_PPV_ARGS(&DxgiFactory));

	//Create Hardware Device.
	HRESULT hardwareResult = D3D12CreateDevice(
		nullptr,             // default adapter
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&D3dDevice));

	//Create Fence for GPU CPU
	D3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,IID_PPV_ARGS(&Fence));
	
	//Descriptor Size
	RtvDescriptorSize = D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	UINT DsvDescriptorSize = D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	UINT CbvSrvUavDescriptorSize = D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//Create Command Objects
	ComPtr<ID3D12CommandAllocator> DirectCmdListAlloc;
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	D3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&CommandQueue));
	D3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,IID_PPV_ARGS(DirectCmdListAlloc.GetAddressOf()));
	D3dDevice->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		DirectCmdListAlloc.Get(),
		nullptr,
		IID_PPV_ARGS(CommandList.GetAddressOf()));
	CommandList->Close();

	//Create SwapChain
	ComPtr<IDXGISwapChain> SwapChain;
	SwapChain.Reset();
	DXGI_SWAP_CHAIN_DESC Scd;
	Scd.BufferDesc.Width = ClientWidth;
	Scd.BufferDesc.Height = ClientHeight;
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
	DxgiFactory->CreateSwapChain(CommandQueue.Get(), &Scd, SwapChain.GetAddressOf());

	//Create Rtv DescriptorHeaps
	D3D12_DESCRIPTOR_HEAP_DESC RtvHeapDesc;
	RtvHeapDesc.NumDescriptors = SwapChainBufferCount;
	RtvHeapDesc.Type= D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	RtvHeapDesc.Flags=D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	RtvHeapDesc.NodeMask=0;
	D3dDevice->CreateDescriptorHeap(&RtvHeapDesc,IID_PPV_ARGS(RtvHeap.GetAddressOf()));
	//Create Dsv DescriptorHeaps
	D3D12_DESCRIPTOR_HEAP_DESC DsvHeapDesc;
	DsvHeapDesc.NumDescriptors=1;
	DsvHeapDesc.Type=D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	DsvHeapDesc.Flags= D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DsvHeapDesc.NodeMask=0;
	D3dDevice->CreateDescriptorHeap(&DsvHeapDesc,IID_PPV_ARGS(DsvHeap.GetAddressOf()));


	//for (UINT i = 0; i < 2; i++)
	//{
	//	SwapChainBuffer[i].Reset();
	//}
	//DepthStencilBuffer.Reset();
	// Resize the swap chain.
	//SwapChain->ResizeBuffers(SwapChainBufferCount,ClientWidth,ClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);


	//Create RenderTargetView
	ComPtr<ID3D12Resource> SwapChainBuffer[SwapChainBufferCount];
	CD3DX12_CPU_DESCRIPTOR_HANDLE RtvHeapHandle(RtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i=0;i< SwapChainBufferCount;i++)
	{
		SwapChain->GetBuffer(i, IID_PPV_ARGS(&SwapChainBuffer[i]));
		D3dDevice->CreateRenderTargetView(SwapChainBuffer[i].Get(),nullptr,RtvHeapHandle);
		RtvHeapHandle.Offset(1,RtvDescriptorSize);
	}

	//Create Depth/Stencil View
	D3D12_RESOURCE_DESC DepthStencilDesc;
	DepthStencilDesc.Dimension=D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	DepthStencilDesc.Alignment=0;
	DepthStencilDesc.Width=ClientWidth;
	DepthStencilDesc.Height=ClientHeight;
	DepthStencilDesc.DepthOrArraySize=1;
	DepthStencilDesc.MipLevels=1;
	DepthStencilDesc.Format=DXGI_FORMAT_R24G8_TYPELESS;
	DepthStencilDesc.SampleDesc.Count=1;
	DepthStencilDesc.SampleDesc.Quality=0;
	DepthStencilDesc.Layout=D3D12_TEXTURE_LAYOUT_UNKNOWN;
	DepthStencilDesc.Flags=D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	D3D12_CLEAR_VALUE OptClear;
	OptClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//DXGI_FORMAT DepthStencilFormat
	OptClear.DepthStencil.Depth=1.0f;
	OptClear.DepthStencil.Stencil=0;
	D3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&DepthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&OptClear,
		IID_PPV_ARGS(DepthStencilBuffer.GetAddressOf()));
	D3D12_DEPTH_STENCIL_VIEW_DESC DsvDesc;
	DsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	DsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	DsvDesc.Format= DXGI_FORMAT_D24_UNORM_S8_UINT;//DXGI_FORMAT mDepthStencilFormat
	DsvDesc.Texture2D.MipSlice=0;
	D3dDevice->CreateDepthStencilView(DepthStencilBuffer.Get(),&DsvDesc,DepthStencilView());

	//Set ScreenV ScissorR
	D3D12_VIEWPORT ScreenViewport;
	ScreenViewport.TopLeftX = 0.0f;
	ScreenViewport.TopLeftY = 0.0f;
	ScreenViewport.Width = static_cast<float>(ClientWidth);
	ScreenViewport.Height = static_cast<float>(ClientHeight);
	ScreenViewport.MinDepth = 0.0f;
	ScreenViewport.MaxDepth = 1.0f;
	D3D12_RECT ScissorRect = { 0,0,ClientWidth,ClientHeight };

	//New Initialize--------
	CommandList->Reset(DirectCmdListAlloc.Get(),nullptr);

	//Create Cbv DescriptorHeaps
	D3D12_DESCRIPTOR_HEAP_DESC CbvHeapDesc;
	CbvHeapDesc.NumDescriptors = 1;
	CbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	CbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	CbvHeapDesc.NodeMask=0;
	D3dDevice->CreateDescriptorHeap(&CbvHeapDesc,IID_PPV_ARGS(&CbvHeap));

	//Build Cb
	ComPtr<ID3D12Resource> UploadBuffer;
	UINT ObjCBByteSize = (sizeof(ObjectConstants) + 255) & ~255;
	D3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(ObjCBByteSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&UploadBuffer));
	BYTE* MappedData=nullptr;
	UploadBuffer->Map(0,nullptr,reinterpret_cast<void**>(&MappedData));
	D3D12_GPU_VIRTUAL_ADDRESS CbAddress = UploadBuffer.Get()->GetGPUVirtualAddress();
	int BoxCBufIndex = 0;
	CbAddress += BoxCBufIndex * ObjCBByteSize;
	D3D12_CONSTANT_BUFFER_VIEW_DESC CbvDesc;
	CbvDesc.BufferLocation= CbAddress;
	CbvDesc.SizeInBytes= ObjCBByteSize;
	D3dDevice->CreateConstantBufferView(&CbvDesc,CbvHeap->GetCPUDescriptorHandleForHeapStart());

	//RootSignature
	CD3DX12_ROOT_PARAMETER SlotRootParameter[1];
	CD3DX12_DESCRIPTOR_RANGE CbvTable;
	CbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,1,0);
	SlotRootParameter[0].InitAsDescriptorTable(1,&CbvTable);
	CD3DX12_ROOT_SIGNATURE_DESC RootSigDesc(1,SlotRootParameter,0,nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	ComPtr<ID3DBlob> SerializedRootSig = nullptr;
	ComPtr<ID3DBlob> ErrorBlob = nullptr;
	D3D12SerializeRootSignature(&RootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,SerializedRootSig.GetAddressOf(), ErrorBlob.GetAddressOf());
	D3dDevice->CreateRootSignature(0,SerializedRootSig->GetBufferPointer(),SerializedRootSig->GetBufferSize(),IID_PPV_ARGS(&MRootSignature));
	
	ShadersAndInputLayout();
	
	UINT VbByteSize = (UINT)Vertices.size() * sizeof(Vertex);
	UINT IbByteSize = (UINT)Indices.size() * sizeof(std::uint16_t);
	ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
	ComPtr<ID3DBlob> IndexBufferCPU = nullptr;
	D3DCreateBlob(VbByteSize, &VertexBufferCPU);
	D3DCreateBlob(IbByteSize, &IndexBufferCPU);
	CopyMemory(IndexBufferCPU->GetBufferPointer(),Indices.data(),IbByteSize);
	CopyMemory(VertexBufferCPU->GetBufferPointer(),Vertices.data(),VbByteSize);


	//PSO
	//std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
	//ComPtr<ID3D12PipelineState> PSO = nullptr;

	//D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	//ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	//psoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	//psoDesc.pRootSignature = MRootSignature.Get();
	//psoDesc.VS =
	//{
	//	reinterpret_cast<BYTE*>(MvsByteCode->GetBufferPointer()),
	//	MvsByteCode->GetBufferSize()
	//};
	//psoDesc.PS =
	//{
	//	reinterpret_cast<BYTE*>(MpsByteCode->GetBufferPointer()),
	//	MpsByteCode->GetBufferSize()
	//};
	//psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	////psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;  //Ïß¿òÄ£Ê½

	//psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	//psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	//psoDesc.SampleMask = UINT_MAX;
	//psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//psoDesc.NumRenderTargets = 1;
	//psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;//DXGI_FORMAT mBackBufferFormat
	//psoDesc.SampleDesc.Count = 1;//m4xMsaaState ? 4 : 1;
	//psoDesc.SampleDesc.Quality = 0;//m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	//psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;//DXGI_FORMAT mDepthStencilFormat
	//D3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&PSO));

	//-----------------------
	

	MSG msg = {0};

	while (msg.message != WM_QUIT)
	{
		// Process any messages in the queue.
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{		
			DirectCmdListAlloc->Reset();
			CommandList->Reset(DirectCmdListAlloc.Get(), nullptr);

			CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(SwapChainBuffer[CurrBackBuffer].Get(),
				D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

			CommandList->RSSetViewports(1, &ScreenViewport);
			CommandList->RSSetScissorRects(1, &ScissorRect);

			CommandList->ClearRenderTargetView(CurrentBackBufferView(),Colors::GreenYellow, 0, nullptr);
			CommandList->ClearDepthStencilView(DepthStencilView(),D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,1.0f, 0, 0, nullptr);

			CommandList->OMSetRenderTargets(1, &CurrentBackBufferView(),true, &DepthStencilView());

			//New -------------

			ID3D12DescriptorHeap* DescriptorHeaps[]={ CbvHeap.Get()};
			CommandList->SetDescriptorHeaps(_countof(DescriptorHeaps),DescriptorHeaps);
			CommandList->SetGraphicsRootSignature(MRootSignature.Get());

			ComPtr<ID3D12Resource> VertexBufferGPU=nullptr;
			ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;
			ComPtr<ID3D12Resource> VertexBufferUploader=nullptr;
			ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;
			VertexBufferGPU=CreateDefaultBuffer(D3dDevice.Get(),CommandList.Get(),Vertices.data(),VbByteSize, VertexBufferUploader);
			IndexBufferGPU=CreateDefaultBuffer(D3dDevice.Get(), CommandList.Get(), Indices.data(), IbByteSize, IndexBufferUploader);

			D3D12_VERTEX_BUFFER_VIEW Vbv;
			Vbv.BufferLocation= VertexBufferGPU->GetGPUVirtualAddress();
			Vbv.StrideInBytes=sizeof(Vertex);
			Vbv.SizeInBytes= VbByteSize;
			CommandList->IASetVertexBuffers(0,1,&Vbv); //**

			D3D12_INDEX_BUFFER_VIEW Ibv;
			Ibv.BufferLocation=IndexBufferGPU->GetGPUVirtualAddress();
			Ibv.Format= DXGI_FORMAT_R16_UINT;//DXGI_FORMAT IndexFormat
			Ibv.SizeInBytes = IbByteSize;
			CommandList->IASetIndexBuffer(&Ibv);//**
			CommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			//CommandList->SetGraphicsRootDescriptorTable(0, CbvHeap->GetGPUDescriptorHandleForHeapStart());			

			CommandList->DrawIndexedInstanced((UINT)Indices.size(),1,0,0,0);
			//---------------------------


			CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(SwapChainBuffer[CurrBackBuffer].Get(),
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

			CommandList->Close();
			ID3D12CommandList* CmdsLists[] = { CommandList.Get() };
			CommandQueue->ExecuteCommandLists(_countof(CmdsLists), CmdsLists);

			SwapChain->Present(0, 0);
			CurrBackBuffer = (CurrBackBuffer + 1) % SwapChainBufferCount;

			FlushCommandQueue();	
		}
	}
}

void FlushCommandQueue()
{
	CurrentFence++;
	CommandQueue->Signal(Fence.Get(), CurrentFence);
	if (Fence->GetCompletedValue() < CurrentFence)
	{
		HANDLE EventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		Fence->SetEventOnCompletion(CurrentFence, EventHandle);
		WaitForSingleObject(EventHandle, INFINITE);
		CloseHandle(EventHandle);
	}
}


D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView()
{
	return DsvHeap->GetCPUDescriptorHandleForHeapStart();
}

D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		RtvHeap->GetCPUDescriptorHandleForHeapStart(),
		CurrBackBuffer,
		RtvDescriptorSize);
}

ComPtr<ID3D12Resource> CreateDefaultBuffer(ID3D12Device* Device,
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

ComPtr<ID3DBlob> CompileShader(const std::wstring& Filename, const D3D_SHADER_MACRO* Defines, const std::string& EntryPoint, const std::string& Target)
{
	ComPtr<ID3DBlob> ByteCode;
	ComPtr<ID3DBlob> Errors;
	D3DCompileFromFile(Filename.c_str(),Defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		EntryPoint.c_str(),Target.c_str(),0,0,&ByteCode,&Errors);
	return ByteCode;
}

void ShadersAndInputLayout()
{
	MvsByteCode= CompileShader(L"Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
	MpsByteCode= CompileShader(L"Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");

	//InputLayout
}

