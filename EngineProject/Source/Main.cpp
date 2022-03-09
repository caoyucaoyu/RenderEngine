#include "stdafx.h"




int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,PSTR cmdLine, int showCmd)
{
	Engine::InitEngine();

	Engine::Get()->GetResourceManager()->LoadMap("StaticMesh\\Map1.Usmh");

	Engine::Get()->Run();

	Engine::Destroy();

	return 0;
}