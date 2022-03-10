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
}

void Scene::Init()
{
	DirectX::FXMVECTOR Position = { 3000.f, 0.0f, 1000.0f };
	DirectX::FXMVECTOR Target = { 0.0f, 0.0f, 0.0f };
	DirectX::FXMVECTOR Up = { 0.0f, 0.0f, 1.0f };
	MainCamera.LookAt(Position, Target, Up);

	MainCamera.UpdateViewMatrix();
}

void Scene::LoadMapActors()
{
	for (auto MapMeshItem : Engine::Get()->GetResourceManager()->GetMapMeshsData())
	{
		MeshActor AMeshActor(MapMeshItem);
		SceneMeshActors.push_back(AMeshActor);
		ActorNum++;
	}

}

std::vector<MeshActor> Scene::GetSceneMeshActors()
{
	return SceneMeshActors;
}

Camera& Scene::GetMainCamera()
{
	return MainCamera;
}

void Scene::AddActor()
{
	ActorNum++;
}
