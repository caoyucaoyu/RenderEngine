#include "stdafx.h"
#include "WindowWin32.h"
#include "Engine.h"

static WindowWin32* Win32Window =nullptr;

LRESULT CALLBACK MainWindowProc(HWND Window, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if(Win32Window == nullptr)
		return DefWindowProc(Window, Msg, wParam, lParam);
	Input* Inputv = Win32Window->GetInput();

	switch (Msg) 
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 500;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 300;
		return 0;

	case WM_LBUTTONDOWN:
		Engine::Get()->GetWindow()->GetInput()->OnMouseDown(Key::LM, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		SetCapture(Win32Window->GetWnd());
		return 0;
	case WM_RBUTTONDOWN:
		Engine::Get()->GetWindow()->GetInput()->OnMouseDown(Key::RM, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		SetCapture(Win32Window->GetWnd());
		return 0;

	case WM_LBUTTONUP:
		ReleaseCapture();
		Engine::Get()->GetWindow()->GetInput()->OnMouseUp(Key::LM, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_RBUTTONUP:
		ReleaseCapture();
		Engine::Get()->GetWindow()->GetInput()->OnMouseUp(Key::RM, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	case WM_MOUSEMOVE:
		Engine::Get()->GetWindow()->GetInput()->OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	//case WM_SIZE:
		//ClientWidth = LOWORD(lParam);
		//ClientHight = HIWORD(lParam);
		//if (D3dDevice)
		//{
			//if (wParam == SIZE_MINIMIZED)
			//{
				//IsAppPaused=true;
				//IsMinimized=true;
				//IsMaximized=false;
			//}
			//else if (wParam == SIZE_MAXIMIZED)
			//{
				//IsAppPaused=false;
				//IsMinimized=false;
				//IsMaximized=true;
				//AAppWin32->OnResize();
			//}
			//else if (wParam == SIZE_RESTORED)
			//{
				//AAppWin32->OnResize();
				//if (IsMinimized)
				//{
				//	IsAppPaused = false;
				//	IsMinimized = false;
				//	AAppWin32->OnResize();
				//}
				//else if (IsMaximized)
				//{
				//	IsAppPaused = false;
				//	IsMaximized = false;
				//	AAppWin32->OnResize();
				//}
				//else if (IsResizing)
				//{
				//
				//}
				//else
				//{
				//	AAppWin32->OnResize();
				//}
			//}
		//}
	default:
		break;
	}
	return DefWindowProc(Window, Msg, wParam, lParam);

	//Inputv->OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	//Inputv->OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	//Inputv->OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
}

WindowWin32::WindowWin32(UINT32 Width, UINT32 Height)
{
	Win32Window = this;
	Win32Window->Width = Width;
	Win32Window->Hight = Hight;
	InitWindow();
}

bool WindowWin32::Init(HINSTANCE hInstance)
{
	InitWindow();
	return true;
}

bool WindowWin32::InitWindow()
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

	RECT R = { 0, 0, Width, Hight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int Width = R.right - R.left;
	int Height = R.bottom - R.top;

	HMainWnd = CreateWindow(L"MainWnd", MainWndCaption.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, Width, Height, 0, 0, hInstance, 0);
	
	ShowWindow(HMainWnd, SW_SHOW);
	UpdateWindow(HMainWnd);

	return true;
}


void WindowWin32::CalculateFrameStats()
{
	// Code computes the average frames per second, and also the 
	// average time it takes to render one frame.  These stats 
	// are appended to the window caption bar.

	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	// Compute averages over one second period.
	if ((Engine::Get()->GetTimer()->TotalTime() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;

		std::wstring fpsStr = std::to_wstring(fps);
		std::wstring mspfStr = std::to_wstring(mspf);

		std::wstring windowText = MainWndCaption +
			L"     " + fpsStr;
			// + L"   mspf: " + mspfStr;

		SetWindowText(HMainWnd, windowText.c_str());

		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

WindowWin32::~WindowWin32()
{
	Win32Window=nullptr;
}


void WindowWin32::OnResize()
{
	RECT r;
	GetWindowRect(HMainWnd, &r);
	MoveWindow(HMainWnd, r.left, r.top, 1920, 1080, TRUE);
	Engine::Get()->GetRender()->RendererReset();
}


int WindowWin32::Run()
{
	MInput->Update();

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
	//CalculateFrameStats();
	return !Quit;
}

HWND WindowWin32::GetWnd()
{
	return HMainWnd;
}
