#include "stdafx.h"
#include "Engine.h"
#include <thread>

//SM_TableRound

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,PSTR cmdLine, int showCmd)
{
	std::thread t1([](){OutputDebugStringA("thread t1\n");});
	t1.join();

	Engine::InitEngine();

	Engine::Get()->GetResourceManager()->LoadMap("StaticMesh\\Map1.Usmh");

	Engine::Get()->Run();

	Engine::Destroy();

	return 0;
}

