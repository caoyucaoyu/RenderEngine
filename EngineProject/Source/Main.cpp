#include "stdafx.h"
#include "Engine.h"
using namespace std;

/*
RenderThread* RenderT = RenderThread::Get();
Task* task = new Task([=]() {RenderT->GetRenderScene()->AddMapData(SceneMeshActors); });
RenderT->AddTask(task);
*/
//
//int main()
//{
//	std::thread t1([]() {OutputDebugStringA("Thread t1\n"); });
//	t1.join();
//
//	Engine::InitEngine();
//
//	Engine::Get()->GetAssetsManager()->LoadMap("StaticMesh\\Map1.Usmh");
//
//	Engine::Get()->Run();
//
//	Engine::Destroy();
//
//	return 0;
//}

#if PLATFORM_WINDOWS

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
	std::thread t1([]() {OutputDebugStringA("Thread t1\n"); });
	t1.join();

	Engine::InitEngine();

	Engine::Get()->GetAssetsManager()->LoadMap("StaticMesh\\Map1.Usmh");

	Engine::Get()->Run();

	Engine::Destroy();

	return 0;
}

#elif PLATFORM_IOS

#elif PLATFORM_ANDROID

#endif


