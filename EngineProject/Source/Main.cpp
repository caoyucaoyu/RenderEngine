#include "stdafx.h"




int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,PSTR cmdLine, int showCmd)
{
	Engine::InitEngine();

	Engine::Get()->GetScene().LoadMap("StaticMesh\\Map1.Usmh");

	Engine::Get()->Start();

	Engine::Destroy();
	return 0;
}