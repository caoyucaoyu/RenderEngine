#pragma once

class StaticMesh
{
public:
	StaticMesh() {};
	StaticMesh(MeshRead MeshData)//由源数据构建Mesh资源
	{
		MeshName = MeshData.MeshName;
		Vertices = MeshData.Vertices;
		Indices = MeshData.Indices;
	}
	std::string MeshName;
	std::vector<Vertex> Vertices;
	std::vector<uint16_t> Indices;
};