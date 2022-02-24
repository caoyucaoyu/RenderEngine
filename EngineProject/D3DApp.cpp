#include "D3DApp.h"

LRESULT CALLBACK MainWindowProc(HWND Window, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	return D3DApp::GetApp()->MsgProc(Window, Msg, wParam, lParam);
}



D3DApp::D3DApp()
{
	//assert(App == nullptr);
	App = this;
}

D3DApp::~D3DApp()
{

}

LRESULT D3DApp::MsgProc(HWND Window, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 400;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 400;
		return 0;
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_SIZE:
		ClientWidth = LOWORD(lParam);
		ClientHight = HIWORD(lParam);
		if (D3dDevice)
		{
			if (wParam == SIZE_MINIMIZED)
			{
				IsAppPaused=true;
				IsMinimized=true;
				IsMaximized=false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				IsAppPaused=false;
				IsMinimized=false;
				IsMaximized=true;
				OnResize();
			}
			else if (wParam == SIZE_RESTORED)
			{
				if (IsMinimized)
				{
					IsAppPaused = false;
					IsMinimized = false;
					OnResize();
				}
				else if (IsMaximized)
				{
					IsAppPaused = false;
					IsMaximized = false;
					OnResize();
				}
				else if (IsResizing)
				{

				}
				else
				{
					OnResize();
				}
			}
		}
	default:
		break;
	}	
	return DefWindowProc(Window, Msg, wParam, lParam);
}

D3DApp* D3DApp::App = nullptr;

D3DApp* D3DApp::GetApp()
{
	return App;
}

void D3DApp::CreateDevice()
{
	//Create IDXGIFactory
	CreateDXGIFactory1(IID_PPV_ARGS(&DxgiFactory));

	//Create Hardware Device.
	HRESULT hardwareResult = D3D12CreateDevice(
		nullptr,             // default adapter
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&D3dDevice));
}

void D3DApp::CreateFence()
{
	//Create Fence for GPU CPU
	D3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence));
}

void D3DApp::GetDescriptorSize()
{
	//Descriptor Size
	RtvDescriptorSize = D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	UINT DsvDescriptorSize = D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	UINT CbvSrvUavDescriptorSize = D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

}

void D3DApp::SetMSAA()
{
	MsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	MsaaQualityLevels.SampleCount = 1;
	MsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	MsaaQualityLevels.NumQualityLevels = 0;
	D3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &MsaaQualityLevels, sizeof(MsaaQualityLevels));
	assert(MsaaQualityLevels.NumQualityLevels > 0);
}

void D3DApp::CreateCommandObject()
{
	//Create Command Objects
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

void D3DApp::CreateSwapChain()
{
	//Create SwapChain
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
	DxgiFactory->CreateSwapChain(CommandQueue.Get(), &Scd, SwapChain.GetAddressOf());
}

void D3DApp::CreateDescriptorHeap()
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

void D3DApp::CreateRTV()
{
	//Create RenderTargetView
	CD3DX12_CPU_DESCRIPTOR_HANDLE RtvHeapHandle(RtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < SwapChainBufferCount; i++)
	{
		SwapChain->GetBuffer(i, IID_PPV_ARGS(&SwapChainBuffer[i]));
		D3dDevice->CreateRenderTargetView(SwapChainBuffer[i].Get(), nullptr, RtvHeapHandle);
		RtvHeapHandle.Offset(1, RtvDescriptorSize);
	}
}

void D3DApp::CreateDSV()
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

void D3DApp::FlushCommandQueue()
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

void D3DApp::CreateViewPortAndScissorRect()
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

bool D3DApp::InitWindow(HINSTANCE hInstance)
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

	RECT R = { 0, 0, ClientWidth, ClientHight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int Width = R.right - R.left;
	int Height = R.bottom - R.top;

	std::wstring MainWndCaption = L"d3d xxx";
	HMainWnd = CreateWindow(L"MainWnd", MainWndCaption.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, Width, Height, 0, 0, hInstance, 0);
	ShowWindow(HMainWnd, SW_SHOW);
	UpdateWindow(HMainWnd);

	return true;
}

bool D3DApp::InitDirect3D()
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

	///Move To OnResize()
	///CreateRTV();
	///CreateDSV();
	///CreateViewPortAndScissorRect();

	return true;
}

void D3DApp::OnResize()
{
	assert(D3dDevice);
	assert(SwapChain);
	assert(CommandListAlloc);

	FlushCommandQueue();

	CommandList->Reset(CommandListAlloc.Get(), nullptr);

	for (int i = 0; i < SwapChainBufferCount; ++i)
		SwapChainBuffer[i].Reset();
	DepthStencilBuffer.Reset();

	SwapChain->ResizeBuffers(SwapChainBufferCount,ClientWidth,ClientHight, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
	CurrBackBuffer=0;
	CreateRTV();
	CreateDSV();
	CreateViewPortAndScissorRect();

	ThrowIfFailed(CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { CommandList.Get() };
	CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	FlushCommandQueue();
}

bool D3DApp::Init(HINSTANCE hInstance)
{
	if(!InitWindow(hInstance))return false;
	if(!InitDirect3D())return false;
	OnResize();
	return true;
}

int D3DApp::Run()
{
	MSG msg = { 0 };

	Timer.Reset();

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Timer.Tick();
			Update();
			Draw();
		}
	}
	return (int)msg.wParam;//..
}

D3D12_CPU_DESCRIPTOR_HANDLE D3DApp::DepthStencilView()
{
	return DsvHeap->GetCPUDescriptorHandleForHeapStart();
}

D3D12_CPU_DESCRIPTOR_HANDLE D3DApp::CurrentBackBufferView()
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		RtvHeap->GetCPUDescriptorHandleForHeapStart(),
		CurrBackBuffer,
		RtvDescriptorSize);
}
