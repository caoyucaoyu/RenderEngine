#include "stdafx.h"
#include "Scene.h"

Scene::Scene()
{

}

Scene::~Scene()
{

}

void Scene::LoadMap(const std::string& filePath)
{
	D3DUtil::ReadMapFile(filePath, MeshsData);

}
