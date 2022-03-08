#include "stdafx.h"
#include "AppWin32.h"

static AppWin32* AAppWin32 =nullptr;

LRESULT CALLBACK MainWindowProc(HWND Window, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if(AAppWin32 == nullptr)
		return DefWindowProc(Window, Msg, wParam, lParam);
	Input* Inputv = AAppWin32->GetInput();

	switch (Msg) 
	{
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
		Inputv->OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		SetCapture(AAppWin32->GetWnd());
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		Inputv->OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		Inputv->OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_SIZE:
		/*ClientWidth = LOWORD(lParam);
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
		}*/
	default:
		break;
	}
	return DefWindowProc(Window, Msg, wParam, lParam);
}

AppWin32::AppWin32()
{
	AAppWin32=this;
	Init();
}

bool AppWin32::Init(HINSTANCE hInstance)
{
	InitWindow();
	//OnResize();
	return true;
}

bool AppWin32::InitWindow()
{
	HINSTANCE hInstance = GetModuleHandle(0);

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

	std::wstring MainWndCaption = L"Hello";
	HMainWnd = CreateWindow(L"MainWnd", MainWndCaption.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, Width, Height, 0, 0, hInstance, 0);
	
	ShowWindow(HMainWnd, SW_SHOW);
	UpdateWindow(HMainWnd);

	return true;
}


AppWin32::~AppWin32()
{
	AAppWin32=nullptr;
}


void AppWin32::OnResize()
{
	//assert(D3dDevice);
	//assert(SwapChain);
	//assert(CommandListAlloc);

	//FlushCommandQueue();

	//CommandList->Reset(CommandListAlloc.Get(), nullptr);

	//for (int i = 0; i < SwapChainBufferCount; ++i)
	//	SwapChainBuffer[i].Reset();
	//DepthStencilBuffer.Reset();

	//SwapChain->ResizeBuffers(SwapChainBufferCount,ClientWidth,ClientHight, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
	//CurrBackBuffer=0;
	// 
	//CreateRTV();
	//CreateDSV();
	//CreateViewPortAndScissorRect();

	//ThrowIfFailed(CommandList->Close());
	//ID3D12CommandList* cmdsLists[] = { CommandList.Get() };
	//CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	//FlushCommandQueue();
}


int AppWin32::Run()
{
	bool Quit=false;	
	MSG msg = {};
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_QUIT)
		{
			Quit=true;
		}

	}
	return !Quit;
}

HWND AppWin32::GetWnd()
{
	return HMainWnd;
}