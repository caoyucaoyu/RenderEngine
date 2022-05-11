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
	UpdateCameraBuffer();
	//if (Input::GetKeyState(Key::LM) == KeyState::BtnDown)
	//{
		//MeshActor* ma;
		//AddMeshActor(ma);
		//Engine::Get()->GetRender()->InitDraw();
	//}
}

void Scene::Init()
{
	glm::vec3 Position = { 3000.f, 0.0f, 1000.0f };
	glm::vec3 Target = { 0.0f, 0.0f, 0.0f };
	glm::vec3 Up = { 0.0f, 0.0f, 1.0f };

	MainCamera.LookAt(Position, Target, Up);

	MainCamera.UpdateViewMatrix();
}

void Scene::LoadMapActors()
{
	for (auto MapMeshItem : Engine::Get()->GetAssetsManager()->GetMeshReadData())
	{
		MeshActor* AMeshActor = new MeshActor(MapMeshItem);
		AMeshActor->UpdateMatrix();
		SceneMeshActors.push_back(AMeshActor);
	}

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


	//for (auto AMeshActor : SceneMeshActors)
	//{
	//	Task* task1 = new Task([=]() {RenderT->GetRenderScene()->AddMeshBuffer(AMeshActor); });
	//	RenderT->AddTask(task1);
	//	Task* task2 = new Task([=]() {RenderT->GetRenderScene()->AddPrimitive(AMeshActor); });
	//	RenderT->AddTask(task2);
	//}
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

void Scene::UpdateCameraBuffer()
{
	glm::mat4 VP_Matrix = MainCamera.GetProj() * MainCamera.GetView();

	PassConstants NewPasConstants;
	NewPasConstants.Time = Engine::Get()->GetTimer()->TotalTime();
	NewPasConstants.ViewProj_M = glm::transpose(VP_Matrix);

	RenderThread* RenderT = RenderThread::Get();
	Task* task = new Task([=]() {RenderT->GetRenderScene()->UpdateCameraBuffer(NewPasConstants); });
	RenderT->AddTask(task);
}

