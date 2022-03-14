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
		if (MeshAssets.count(MeshItem.MeshName))
			continue;
	
		Mesh AMeshData(MeshItem);
		MeshAssets.insert(std::make_pair(MeshItem.MeshName,AMeshData));
	}

}



void ResourceManager::LoadTexture(std::string Name, const std::string& filePath)
{
	Texture Tex;
	Tex.Name = Name;
	Tex.Filename = L"../../Textures/WoodCrate01.dds";//Use filePath
	//ThrowIfFailed(
	//DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),mCommandList.Get(), woodCrateTex->Filename.c_str(),woodCrateTex->Resource, woodCrateTex->UploadHeap));
	TextureAssets[Tex.Name] = Tex;
}

void ResourceManager::AddMaterial(std::string Name, Material NMaterial)
{
	MaterialAssets[Name] = NMaterial;
}


const std::vector<MapItem>& ResourceManager::GetMapMeshsData()
{
	return MapItemData;
}









bool ResourceManager::FindMesh(std::string Name, Mesh& NMesh)
{
	if (!MeshAssets.count(Name))
	{
		return false;
	}
	else
	{
		NMesh = MeshAssets.at(Name);
		return true;
	}
}

bool ResourceManager::FindMaterial(std::string Name, Material& NMaterial)
{
	if (!MaterialAssets.count(Name))
	{
		return false;
	}
	else
	{
		NMaterial = MaterialAssets.at(Name);
		return true;
	}
}

bool ResourceManager::FindTexture(std::string Name, Texture& NTexture)
{
	if (!TextureAssets.count(Name))
	{
		return false;
	}
	else
	{
		NTexture = TextureAssets.at(Name);
		return true;
	}
}
