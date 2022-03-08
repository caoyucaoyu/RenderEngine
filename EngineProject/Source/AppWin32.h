#pragma once
#include "stdafx.h"
#include "App.h"
#include "D3dUtil.h"
#include "GameTimer.h"
#include "Input.h"

using Microsoft::WRL::ComPtr;

class AppWin32 : public App
{
public:
	AppWin32();
	virtual ~AppWin32();

	/*static TAppWin* GetApp();*/

	virtual void OnResize();
	virtual bool Init(HINSTANCE hInstance=nullptr);
	virtual int Run()override;

	virtual HWND GetWnd() override;
	bool InitWindow();

protected:

	int ClientWidth = 1280;
	int ClientHight = 720;

	HWND HMainWnd;

	///
	bool IsAppPaused = false; 
	bool IsMinimized = false; 
	bool IsMaximized = false;
	bool IsResizing = false;
	bool IsFullscreenState = false;

};

