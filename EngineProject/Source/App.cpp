#include "stdafx.h"
#include "App.h"
#include "AppWin32.h"

App::App()
{
	Inputv = new Input;
}

App::~App()
{
	delete Inputv;
}

App* App::CreateApp()
{
	App* NApp = nullptr;
#if PLATFORM_WINDOWS
	NApp = new AppWin32;
#elif PLATFORM_IOS

#else

#endif
	return NApp;
}

void App::DestroyApp(App* DApp)
{
	delete DApp;
}

Input* App::GetInput()
{
	return Inputv;
}


