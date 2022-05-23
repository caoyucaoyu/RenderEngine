#pragma once
#include "StaticMesh.h"
#include "Material.h"
#include "Texture.h"

/*
	读取的数据类型
	静态网格体
	材质
	贴图
*/

class AssetsManager
{
public:
	AssetsManager();
	~AssetsManager();

	void LoadMap(const std::string& filePath);
	void LoadTexture(std::string Name,const std::wstring& filePath);
	void CreateMaterial();

public:
	const std::vector<MeshRead>& GetMeshReadData();//源数据

	bool FindMesh(std::string Name,StaticMesh& NMesh);
	Texture* GetTexture(std::string Name);
	Material* GetMaterial(std::string Name);
	Material* GetDefaultMaterial();

	void AddTexture(std::string Name, Texture* NTexture);
	void AddMaterial(std::string Name, Material* NMaterial);

private:
	std::vector<MeshRead> MeshReadData;

	std::unordered_map<std::string, StaticMesh> StaticMeshAssets;
	std::unordered_map<std::string, Material*> MaterialAssets;
	std::unordered_map<std::string, Texture*> TextureAssets;

private:
	void ReadMapFile(const std::string& filePath, std::vector<MeshRead>& MeshData);

};

