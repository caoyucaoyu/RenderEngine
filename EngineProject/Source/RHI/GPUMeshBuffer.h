#pragma once
#include "D3DUtil.h"
#include "AssetsManager.h"

class GPUMeshBuffer
{
public:
	GPUMeshBuffer() {};
	virtual ~GPUMeshBuffer() {};

	GPUMeshBuffer(MeshRead MeshData)//由源数据构建Mesh资源
	{
		Vertices = MeshData.Vertices;
		Indices = MeshData.Indices;
		VbByteSize = (UINT)MeshData.Vertices.size() * sizeof(Vertex);
		IbByteSize = (UINT)MeshData.Indices.size() * sizeof(std::uint16_t);
	}

	virtual void SetData(StaticMesh StaticMeshData)
	{
		Vertices = StaticMeshData.Vertices;
		Indices = StaticMeshData.Indices;
		VbByteSize = (UINT)StaticMeshData.Vertices.size() * sizeof(Vertex);
		IbByteSize = (UINT)StaticMeshData.Indices.size() * sizeof(std::uint16_t);
	};

	std::vector<Vertex> GetVertices() { return Vertices; }
	std::vector<uint16_t> GetIndices() { return Indices; }

protected:
	std::vector<Vertex> Vertices;
	std::vector<uint16_t> Indices;

	UINT VbByteSize = 0;
	UINT IbByteSize = 0;

};
