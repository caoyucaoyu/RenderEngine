#pragma once
#include "Material.h"
#include "Texture.h"

//读取文件，得到的数据项
struct MeshRead
{
	std::string MeshName;
	Float3 Location;
	Float4 Rotation;
	Float3 Scale3D;
	std::vector<Vertex> Vertices;
	std::vector<uint16_t> Indices;
};

class Mesh;

class AssetsManager
{
public:
	AssetsManager();
	~AssetsManager();

	void LoadMap(const std::string& filePath);
	void LoadTexture(std::string Name,const std::string& filePath);

	const std::vector<MeshRead>& GetMeshReadData();//源数据

	//获得资产
	bool FindMesh(std::string Name,Mesh& NMesh);
	bool FindMaterial(std::string Name, Material& NMaterial);
	bool FindTexture(std::string Name, Texture& NTexture);


	void AddMaterial(std::string Name, Material NMaterial);

private:
	void ReadMapFile(const std::string& filePath, std::vector<MeshRead>& MeshData);

	std::vector<MeshRead> MeshReadData;

	std::unordered_map<std::string, Mesh> MeshAssets;
	std::unordered_map<std::string, Material> MaterialAssets;
	std::unordered_map<std::string, Texture> TextureAssets;
};

