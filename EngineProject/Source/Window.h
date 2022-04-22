#pragma once
#include "Input.h"

class Window 
{
public:
	static Window* CreateApp();
	static void DestroyApp(Window* DApp);
	
	virtual int Run() = 0;

	virtual HWND GetWnd() = 0;
	Input* GetInput();

protected:
	Window();
	virtual ~Window();

	int Width;
	int Height;

	Input* MInput;
};
