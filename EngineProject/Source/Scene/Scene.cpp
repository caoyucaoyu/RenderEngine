#include "stdafx.h"
#include "Engine.h"
#include "Scene.h"

Scene::Scene()
{

}

Scene::~Scene()
{

}

void Scene::Tick()
{
	MainCamera.Update();

	if (Input::GetKeyState(Key::LM) == KeyState::BtnDown)
	{
		MeshActor ma;
		AddMeshActor(ma);
		//Engine::Get()->GetRender()->InitDraw();
	}
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
		MeshActor AMeshActor(MapMeshItem);
		SceneMeshActors.push_back(AMeshActor);
		MeshActorNum++;
	}
}

void Scene::AddMeshActor(MeshActor NewActor)
{
	static int of = 300;
	if (of<6300)
	{
		of += 300;
		auto newa = SceneMeshActors[2];
		std::string name = newa.MeshName;
		newa.Location = Float3(-3002, -3000, 0);
		newa.Scale3D = Float3(2, 2, 2);
		newa.Location.Y += of;
		newa.UpdateMatrix();
		SceneMeshActors.push_back(newa);
	}

	NewActor.UpdateMatrix();

	SceneMeshActors.push_back(NewActor);

	UpdateActorNum();
}

void Scene::UpdateActorNum()
{
	MeshActorNum = SceneMeshActors.size();
}

std::vector<MeshActor> Scene::GetSceneMeshActors()
{
	return SceneMeshActors;
}

Camera& Scene::GetMainCamera()
{
	return MainCamera;
}


