#include "stdafx.h"
#include "Engine.h"
#include "OldRenderer.h"
#include "RenderThread.h"
#include "RHI/RHI.h"

Engine::Engine() : IsRunning(false)
{

}

Engine::~Engine()
{
	Window::DestroyWindow(MWindow);
}

Engine* Engine::MEngine = nullptr;

void Engine::InitEngine()
{
	if (MEngine == nullptr)
	{
		MEngine =new Engine;
		MEngine->Init();		
	}
}
void Engine::Init()
{
	assert(MWindow == nullptr);
	MWindow = Window::CreateAWindow();
	RenderThread::CreateRenderThread();//目前不是创建即运行

	MScene = new Scene();
	MTimer = new GameTimer();
	MAssetsManager = new AssetsManager();

	MTimer->Reset();
	MTimer->Start();

	MScene->Init();

	//Old ForRun
	MOldRender = new OldRenderer();
	MOldRender->Init();
}

void Engine::Destroy()
{
	if (MEngine)
	{
		RHI::DestroyRHI();
		RenderThread::DestroyRenderThread();
		delete MEngine->MOldRender;
		delete MEngine->MScene;
		delete MEngine->MAssetsManager;
		delete MEngine->MTimer;
		delete MEngine;
		MEngine = nullptr;
	}
}


void Engine::Run()
{
	IsRunning=true;

	MScene->LoadMapActors();
	MOldRender->InitDraw();

	RenderThread::Get()->Start();

#if PLATFORM_WINDOWS

	while (IsRunning&&MWindow->Run())
	{
		Tick();
	}
	//Engine::Destroy();

#elif PLATFORM_IOS
	
#elif PLATFORM_ANDROID

#else
	#error("Not supported platform")
#endif


}



void Engine::Tick()
{
	OutputDebugStringA("Engine Tick\n");
	MTimer->Tick();
	MScene->Tick();
	MWindow->GetInput()->Update();
	//MOldRender->Render();
}





Engine* Engine::Get()
{
	return MEngine;
}

Window* Engine::GetWindow()
{
	return MWindow;
}

OldRenderer* Engine::GetRender()
{
	return MOldRender;
}

Scene* Engine::GetScene()
{
	return MScene;
}

AssetsManager* Engine::GetAssetsManager()
{
	return MAssetsManager;
}

GameTimer* Engine::GetTimer()
{
	return MTimer;
}

