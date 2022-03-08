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
		delete MEngine;
		MEngine = nullptr;
	}

}

Engine* Engine::Get()
{
	return MEngine;
}

void Engine::Start()
{
	IsRunning=true;

	while (IsRunning&&MApp->Run())
	{
		//Timer.Tick();
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
	Timer.Reset();
	//Timer.Start();
	MScene.MainCamera.UpdateViewMatrix();
	MScene.MainCamera.SetPosition(0.f, -4000.0f, 1000.0f);
	Render.Init();
}


void Engine::Tick()
{
	Timer.Tick();

	MScene.MainCamera.Update();
	Render.Run();
}

void Engine::Shutdown()
{

}



App* Engine::GetApp()
{
	return MApp;
}

Scene Engine::GetScene()
{
	return MScene;
}

GameTimer Engine::GetTimer()
{
	return Timer;
}

