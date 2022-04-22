#include "stdafx.h"
#include "Window.h"
#include "WindowWin32.h"

Window::Window()
{
	MInput =Input::CreateInput();
}

Window::~Window()
{
	delete MInput;
}

Window* Window::CreateApp()
{
	Window* NApp = nullptr;
#if PLATFORM_WINDOWS
	NApp = new WindowWin32;
#elif PLATFORM_IOS

#elif PLATFORM_ANDROID

#endif
	return NApp;
}

void Window::DestroyApp(Window* DApp)
{
	delete DApp;
}

Input* Window::GetInput()
{
	return MInput;
}


