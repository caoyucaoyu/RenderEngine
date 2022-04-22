#pragma once
#include "Window.h"
#include "Renderer.h"
#include "Scene.h"
#include "AssetsManager.h"

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

	Window* GetWindow();
	Renderer* GetRender();
	Scene* GetScene();
	AssetsManager* GetAssetsManager();
	GameTimer* GetTimer();

private:
	void Init();
	void Tick();
	
private:
	Window* MWindow;
	Renderer* MRender;
	Scene* MScene;
	AssetsManager* MResourceManager;
	GameTimer* MTimer;
	bool IsRunning;
};

