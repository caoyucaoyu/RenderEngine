#pragma once

class StaticMesh
{
public:
	StaticMesh() {};
	StaticMesh(MeshRead MeshData)//��Դ���ݹ���Mesh��Դ
	{
		MeshName = MeshData.MeshName;
		Vertices = MeshData.Vertices;
		Indices = MeshData.Indices;
	}
	std::string MeshName;
	std::vector<Vertex> Vertices;
	std::vector<uint16_t> Indices;
};