#include "stdafx.h"
#include "Engine.h"
#include "Scene.h"
#include "Task.h"
#include "RenderThread.h"
#include "RenderScene.h"


Scene::Scene()
{

}

Scene::~Scene()
{

}

void Scene::Tick()
{
	MainCamera.Update();

	UpdateMainPassBuffer();
	//if (Input::GetKeyState(Key::LM) == KeyState::BtnDown)
	//{
		//MeshActor* ma;
		//AddMeshActor(ma);
		//Engine::Get()->GetRender()->InitDraw();
	//}
}

void Scene::Init()
{
	glm::vec3 Position = { 3000.0f, 0.0f, 1000.0f };
	glm::vec3 Target = { 0.0f, 0.0f, 0.0f };
	glm::vec3 Up = { 0.0f, 0.0f, 1.0f };

	MainCamera.LookAt(Position, Target, Up);

	MainCamera.UpdateViewMatrix();
}

void Scene::LoadMapActors()
{	
	int HeadNum=0;
	for (auto MapMeshItem : Engine::Get()->GetAssetsManager()->GetMeshReadData())
	{
		MeshActor* AMeshActor = new MeshActor(MapMeshItem);
		//if (AMeshActor->MeshName == "Shape_Sphere")
		//{
			AMeshActor->MaterialName = "Head";
			HeadNum++;
		//}
		AMeshActor->UpdateMatrix();
		SceneMeshActors.push_back(AMeshActor);
	}
	std::stringstream ss;
	ss << "SS HeadActorNum: " << HeadNum << "\n";
	OutputDebugStringA(ss.str().c_str());

}

void Scene::PresentCurrentMap()
{
	if (SceneMeshActors.size() == 0)
	{
		return;
	}

	RenderThread* RenderT= RenderThread::Get();
	Task* task = new Task([=]() {RenderT->GetRenderScene()->AddMapData(SceneMeshActors); });
	RenderT->AddTask(task);
}

void Scene::AddMeshActor(MeshActor* NewActor)
{
	static int of = 300;
	if (of<6300)
	{
		of += 300;
		auto newa = SceneMeshActors[2];
		std::string name = newa->MeshName;
		newa->Location = Float3(-3002, -3000, 0);
		newa->Scale3D = Float3(2, 2, 2);
		newa->Location.Y += of;
		newa->UpdateMatrix();
		SceneMeshActors.push_back(newa);
	}

	NewActor->UpdateMatrix();

	SceneMeshActors.push_back(NewActor);

}

int Scene::GetMeshActorNum()
{
	return SceneMeshActors.size();
}

std::vector<MeshActor*> Scene::GetSceneMeshActors()
{
	return SceneMeshActors;
}

Camera& Scene::GetMainCamera()
{
	return MainCamera;
}

void Scene::UpdateMainPassBuffer()
{
	glm::mat4 VP_Matrix = MainCamera.GetProj() * MainCamera.GetView();
	glm::mat4 Po_Matrix = MainCamera.GetPosM();

	PassConstants NewPasConstants;
	//Time
	NewPasConstants.Time = Engine::Get()->GetTimer()->TotalTime();
	//Camera
	NewPasConstants.ViewProj_M = glm::transpose(VP_Matrix);
	NewPasConstants.CameraPos_M = Po_Matrix;
	NewPasConstants.CameraPos = MainCamera.GetPos();
	//Light
	NewPasConstants.ambientLight = { 0.25f,0.25f,0.35f,1.0f };
	NewPasConstants.lights[0].strength = { 1.0f,1.0f,1.0f };
	NewPasConstants.lights[0].direction = { 1.0f, 0.0f, 0.0f };//计算方向
	NewPasConstants.lights[0].direction = glm::normalize(NewPasConstants.lights[0].direction);

	static float sunPhi = 0;
	static float sunThe = MathHelper::Pi / 4;
	const float dt = Engine::Get()->GetTimer()->DeltaTime();

	//写Input里去
	if (GetAsyncKeyState(VK_LEFT) & 0x8000)
		sunPhi -= 1.0f * dt;
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
		sunPhi += 1.0f * dt;
	if (GetAsyncKeyState(VK_UP) & 0x8000)
		sunThe -= 1.0f * dt;
	if (GetAsyncKeyState(VK_DOWN) & 0x8000)
		sunThe += 1.0f * dt;

	sunThe = MathHelper::Clamp(sunThe, 0.01f, MathHelper::Pi * 0.55f);
	glm::vec3 sunDir = -MathHelper::SphericalToCartesian(1.0f, sunThe, sunPhi);
	NewPasConstants.lights[0].direction = sunDir;

	RenderThread* RenderT = RenderThread::Get();
	Task* task = new Task([=]() {RenderT->GetRenderScene()->UpdateMainPassBuffer(NewPasConstants); });
	RenderT->AddTask(task);
}

