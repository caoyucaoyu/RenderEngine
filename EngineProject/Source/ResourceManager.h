#pragma once



class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	void LoadMap(const std::string& filePath);
	const std::vector<MapItem>& GetMapMeshsData();
	bool FindMesh(std::string MeshName,Mesh& NMesh);

private:
	std::vector<MapItem> MapItemData;

	std::unordered_map<std::string,Mesh> MeshAsssets; 
	
};

