#pragma once
#include "Input.h"

class App 
{
public:
	static App* CreateApp();
	static void DestroyApp(App* DApp);
	
	virtual int Run() = 0;

	virtual HWND GetWnd() = 0;
	Input* GetInput();

protected:
	App();
	virtual ~App();

	int Width;
	int Height;

	Input* MInput;
};
