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
void Engine::Init()
{
	assert(MApp == nullptr);
	MApp = App::CreateApp();
	MScene = new Scene();
	MRender = new DxRenderer();
	MResourceManager = new ResourceManager();

	Timer.Reset();
	Timer.Start();

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

	while (IsRunning&&MApp->Run())
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
	Timer.Tick();
	MApp->GetInput()->Update();
	MScene->Tick();
	MRender->Render();
}





Engine* Engine::Get()
{
	return MEngine;
}

App* Engine::GetApp()
{
	return MApp;
}

DxRenderer* Engine::GetRender()
{
	return MRender;
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

