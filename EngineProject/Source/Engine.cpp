#include "stdafx.h"
#include "Engine.h"
#include "DxRenderer.h"

Engine::Engine() : IsRunning(false)
{

}

Engine::~Engine()
{
	App::DestroyApp(MApp);
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

void Engine::Destroy()
{
	if (MEngine)
	{
		delete MEngine->MRender;
		delete MEngine->MScene;
		delete MEngine->MResourceManager;
		delete MEngine;
		MEngine = nullptr;
	}
}

Engine* Engine::Get()
{
	return MEngine;
}

void Engine::Run()
{
	IsRunning=true;

	MScene->LoadMapActors();
	MRender->InitDraw();

	while (IsRunning&&MApp->Run())
	{
		Tick();
	}
	//Engine::Destroy();

#if PLATFORM_WINDOWS
#elif PLATFORM_IOS
#else
#error("Not supported platform")
#endif
}

void Engine::Init()
{
	assert(MApp==nullptr);
	MApp = App::CreateApp();
	MScene = new Scene();
	MRender = new DxRenderer();
	MResourceManager = new ResourceManager();

	Timer.Reset();
	Timer.Start();

	MScene->Init();
	MRender->Init();
}


void Engine::Tick()
{
	Timer.Tick();
	MScene->Update();
	MRender->Run();
}



App* Engine::GetApp()
{
	return MApp;
}

Scene* Engine::GetScene()
{
	return MScene;
}

ResourceManager* Engine::GetResourceManager()
{
	return MResourceManager;
}

GameTimer Engine::GetTimer()
{
	return Timer;
}

