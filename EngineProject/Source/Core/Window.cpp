#include "stdafx.h"
#include "Window.h"
#include "WindowWin32.h"

Window::Window()
{
	MInput =Input::CreateInput();
}

Window::~Window()
{
	if (MInput != nullptr)
	{
		delete MInput;
		MInput = nullptr;
	}
}

Window* Window::CreateAWindow(UINT32 Width, UINT32 Height)
{
	Window* NApp = nullptr;
#if PLATFORM_WINDOWS
	NApp = new WindowWin32(Width, Height);
#elif PLATFORM_IOS

#elif PLATFORM_ANDROID

#endif
	return NApp;
}

void Window::DestroyWindow(Window* DWindow)
{
	if (DWindow != nullptr)
	{
		delete DWindow;
		DWindow = nullptr;
	}
}

Input* Window::GetInput()
{
	return MInput;
}


