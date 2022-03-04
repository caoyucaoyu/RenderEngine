#include "stdafx.h"
#include <WindowsX.h>
#include "D3DInit.h"

using Microsoft::WRL::ComPtr;


FApp* GetApp()
{
#if PLATFORM_WINDOWS
	return new D3DInit();
#else
	return nullptr;
#endif
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,PSTR cmdLine, int showCmd)
{
	FApp* app = GetApp();
	app->CreateAppWindow(hInstance);
	app->Run();
	delete(app);
	return 0;
	
	//D3DInit D3dapp;
	//D3dapp.Init(hInstance);
	//return D3dapp.Run();
}

