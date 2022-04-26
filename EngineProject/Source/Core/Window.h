#pragma once
#include "Input.h"

class Window 
{
public:
	static Window* CreateAWindow(UINT32 Width, UINT32 Height);
	static void DestroyWindow(Window* DWindow);
	
	virtual int Run() = 0;
	virtual HWND GetWnd() = 0;

	virtual int GetHeight() { return Hight; }
	virtual int GetWidth() { return Width; }

	Input* GetInput();

protected:
	Window();
	virtual ~Window();

	int Width;
	int Hight;

	Input* MInput;
};
