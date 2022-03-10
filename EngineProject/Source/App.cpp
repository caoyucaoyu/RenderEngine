#include "stdafx.h"
#include "App.h"
#include "AppWin32.h"

App::App()
{
	MInput =Input::CreateInput();
}

App::~App()
{
	delete MInput;
}

App* App::CreateApp()
{
	App* NApp = nullptr;
#if PLATFORM_WINDOWS
	NApp = new AppWin32;
#elif PLATFORM_IOS

#elif PLATFORM_ANDROID

#endif
	return NApp;
}

void App::DestroyApp(App* DApp)
{
	delete DApp;
}

Input* App::GetInput()
{
	return MInput;
}


