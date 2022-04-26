#pragma once
#include "Window.h"
#include "OldRenderer.h"
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
	OldRenderer* GetRender();
	Scene* GetScene();
	AssetsManager* GetAssetsManager();
	GameTimer* GetTimer();

private:
	void Init();
	void Tick();
	
private:
	Window* MWindow;
	OldRenderer* MOldRender;
	Scene* MScene;
	AssetsManager* MAssetsManager;
	GameTimer* MTimer;
	bool IsRunning;
};

