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
	Scene* GetScene();
	AssetsManager* GetAssetsManager();
	GameTimer* GetTimer();

private:
	void Init();
	void Tick();
	
private:
	Window* MWindow;
	Scene* MScene;
	AssetsManager* MAssetsManager;
	GameTimer* MTimer;
	bool IsRunning;


	//OldRun
	OldRenderer* MOldRender;
public:
	OldRenderer* GetRender();////////////////
};

