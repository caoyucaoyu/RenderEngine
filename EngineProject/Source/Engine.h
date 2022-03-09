#pragma once
#include "App.h"
#include "DxRenderer.h"
#include "Scene.h"
#include "ResourceManager.h"

class Engine
{
private:
	Engine();
	~Engine();
	static Engine* MEngine;

public:
	static void InitEngine();
	static void Destroy();
	static Engine* Get();
	void Run();

	App* GetApp();
	Scene* GetScene();
	ResourceManager* GetResourceManager();
	GameTimer GetTimer();


private:
	void Init();
	void Tick();
	
private:
	App* MApp;
	DxRenderer* MRender;
	Scene* MScene;
	ResourceManager* MResourceManager;
	GameTimer Timer;
	bool IsRunning;
};

