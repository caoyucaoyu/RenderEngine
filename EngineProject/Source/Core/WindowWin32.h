#pragma once
#include "stdafx.h"
#include "Window.h"

using Microsoft::WRL::ComPtr;

class WindowWin32 : public Window
{
public:	
	WindowWin32(UINT32 Width, UINT32 Height);
	virtual ~WindowWin32();

	virtual void OnResize();
	virtual bool Init(HINSTANCE hInstance=nullptr);
	virtual int Run()override;
	virtual HWND GetWnd() override;

	virtual int GetHeight() { return Hight; }
	virtual int GetWidth() { return Width; }

	bool InitWindow();

protected:

	int Width = 1280;
	int Hight = 720;

	HWND HMainWnd;

	///
	bool IsAppPaused = false; 
	bool IsMinimized = false; 
	bool IsMaximized = false;
	bool IsResizing = false;
	bool IsFullscreenState = false;

};

