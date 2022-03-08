#pragma once
class Scene
{
public:
	Scene();
	~Scene();
	
	void LoadMap(const std::string& filePath);

public:
	Camera MainCamera;
	std::vector<MeshInfo> MeshsData;
};