#pragma once
#include "StaticMesh.h"
#include "Material.h"
#include "Texture.h"


class AssetsManager
{
public:
	AssetsManager();
	~AssetsManager();

	void LoadMap(const std::string& filePath);
	void LoadTexture(std::string Name,const std::string& filePath);


	const std::vector<MeshRead>& GetMeshReadData();//源数据

	//获得资产
	bool FindMesh(std::string Name,StaticMesh& NMesh);
	bool FindMaterial(std::string Name, Material& NMaterial);
	bool FindTexture(std::string Name, Texture& NTexture);

	void AddMaterial(std::string Name, Material NMaterial);

private:
	std::vector<MeshRead> MeshReadData;

	std::unordered_map<std::string, StaticMesh> StaticMeshAssets;
	std::unordered_map<std::string, Material> MaterialAssets;
	std::unordered_map<std::string, Texture> TextureAssets;

private:
	void ReadMapFile(const std::string& filePath, std::vector<MeshRead>& MeshData);

};

