#include "stdafx.h"
#include "D3DUtil.h"
#include "Mesh.h"
#include "ResourceManager.h"

ResourceManager::ResourceManager()
{

}

ResourceManager::~ResourceManager()
{

}

void ResourceManager::LoadMap(const std::string& filePath)
{
	D3DUtil::ReadMapFile(filePath, MapItemData);

	for (auto MeshItem : MapItemData)
	{
		if (MeshAsssets.count(MeshItem.MeshName))
			continue;
		Mesh AMeshData(MeshItem);
		MeshAsssets.insert(std::make_pair(MeshItem.MeshName,AMeshData));
	}

}



const std::vector<MapItem>& ResourceManager::GetMapMeshsData()
{
	return MapItemData;
}



bool ResourceManager::FindMesh(std::string MeshName, Mesh& NMesh)
{
	if (!MeshAsssets.count(MeshName))
	{
		return false;
	}
	else
	{
		NMesh = MeshAsssets.at(MeshName);
		return true;
	}
}
