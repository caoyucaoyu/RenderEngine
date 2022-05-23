#include "stdafx.h"
#include "Engine.h"
using namespace std;

/*
RenderThread* RenderT = RenderThread::Get();
Task* task = new Task([=]() {RenderT->GetRenderScene()->AddMapData(SceneMeshActors); });
RenderT->AddTask(task);
*/

int main()
{
	//std::thread t1([]() {OutputDebugStringA("SS Thread t1\n"); });
	//t1.join();

	Engine::InitEngine();

	Engine::Get()->GetAssetsManager()->LoadMap("StaticMesh\\Map1.Usmh");
	Engine::Get()->GetAssetsManager()->LoadTexture("Head_diff", L"Textures/head_diff.dds");
	Engine::Get()->GetAssetsManager()->LoadTexture("Head_norm", L"Textures/head_norm.dds");
	Engine::Get()->GetAssetsManager()->CreateMaterial();
	Engine::Get()->Run();

	Engine::Destroy();

	return 0;
}

#if PLATFORM_WINDOWS

//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
//{
//	std::thread t1([]() {OutputDebugStringA("SS Thread t1\n"); });
//	t1.join();
//
//	Engine::InitEngine();
//
//	Engine::Get()->GetAssetsManager()->LoadMap("StaticMesh\\Map1.Usmh");
//	Engine::Get()->GetAssetsManager()->LoadTexture("Head_diff", L"Textures/head_diff.dds");
//	Engine::Get()->GetAssetsManager()->LoadTexture("Head_norm", L"Textures/head_norm.dds");
//	Engine::Get()->GetAssetsManager()->CreateMaterial();
//
//	Engine::Get()->Run();
//
//	Engine::Destroy();
//
//	return 0;
//}

#elif PLATFORM_IOS

#elif PLATFORM_ANDROID

#endif


