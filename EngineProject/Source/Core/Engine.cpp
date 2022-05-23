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
	MWindow = Window::CreateAWindow(1280,720);
	RenderThread::CreateRenderThread();

	MScene = new Scene();
	MTimer = new GameTimer();
	MAssetsManager = new AssetsManager();

	MTimer->Reset();
	MTimer->Start();
	MScene->Init();

	//OldRun
	//MOldRender = new OldRenderer();
	//MOldRender->Init();
}

void Engine::Destroy()
{
	if (MEngine)
	{
		RenderThread::DestroyRenderThread();

		//OldRun
		//delete MEngine->MOldRender;

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
	MScene->PresentCurrentMap();

	//OldRun
	//MOldRender->InitDraw();
	//RenderThread::Get()->Start();

#if PLATFORM_WINDOWS

	while (IsRunning && MWindow->Run())
	{
		Tick();
	}

#elif PLATFORM_IOS
	
#elif PLATFORM_ANDROID

#else
	#error("Not supported platform")
#endif

}


void Engine::Tick()
{
	//Input Update Moved to :Window Run 
	MTimer->Tick();
	MScene->Tick();


	RenderThread* RenderThread = RenderThread::Get();
	RenderThread->TriggerRender();//Task give end
	while (RenderThread->GetRenderNum() > 0)//wait render do task
	{
		std::this_thread::sleep_for(std::chrono::nanoseconds(1));
	}
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

