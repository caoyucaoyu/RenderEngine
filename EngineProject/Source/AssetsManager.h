#pragma once
#include "Material.h"
#include "Texture.h"

class AssetsManager
{
public:
	AssetsManager();
	~AssetsManager();

	void LoadMap(const std::string& filePath);
	void LoadTexture(std::string Name,const std::string& filePath);

	void AddMaterial(std::string Name, Material NMaterial);

	const std::vector<MapItem>& GetMapMeshsData();

	bool FindMesh(std::string Name,Mesh& NMesh);
	bool FindMaterial(std::string Name, Material& NMaterial);
	bool FindTexture(std::string Name, Texture& NTexture);

private:
	std::vector<MapItem> MapItemData;

	std::unordered_map<std::string, Mesh> MeshAssets;
	std::unordered_map<std::string, Material> MaterialAssets;
	std::unordered_map<std::string, Texture> TextureAssets;
};

