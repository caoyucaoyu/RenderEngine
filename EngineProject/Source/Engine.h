#pragma once
#include "App.h"
#include "DxRenderer.h"
#include "Scene.h"

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
	void Start();
	void Shutdown();

	App* GetApp();
	Scene GetScene();
	GameTimer GetTimer();

private:
	void Init();
	void Tick();
	
private:
	App* MApp;
	DxRenderer Render;
	Scene MScene;
	
	GameTimer Timer;
	bool IsRunning;
};

