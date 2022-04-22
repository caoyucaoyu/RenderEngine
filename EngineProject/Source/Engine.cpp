#include "stdafx.h"
#include "Engine.h"
#include "Renderer.h"

Engine::Engine() : IsRunning(false)
{

}

Engine::~Engine()
{
	Window::DestroyApp(MWindow);
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
	MWindow = Window::CreateApp();
	MScene = new Scene();
	MResourceManager = new AssetsManager();
	MRender = new Renderer();
	MTimer = new GameTimer();

	MTimer->Reset();
	MTimer->Start();

	MScene->Init();
	MRender->Init();
}

void Engine::Destroy()
{
	if (MEngine)
	{
		delete MEngine->MRender;
		delete MEngine->MScene;
		delete MEngine->MResourceManager;
		delete MEngine->MTimer;
		delete MEngine;
		MEngine = nullptr;
	}
}


void Engine::Run()
{
	IsRunning=true;

	MScene->LoadMapActors();
	MRender->InitDraw();

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
	MTimer->Tick();
	MScene->Tick();
	MWindow->GetInput()->Update();
	MRender->Render();
}





Engine* Engine::Get()
{
	return MEngine;
}

Window* Engine::GetWindow()
{
	return MWindow;
}

Renderer* Engine::GetRender()
{
	return MRender;
}

Scene* Engine::GetScene()
{
	return MScene;
}

AssetsManager* Engine::GetAssetsManager()
{
	return MResourceManager;
}

GameTimer* Engine::GetTimer()
{
	return MTimer;
}

