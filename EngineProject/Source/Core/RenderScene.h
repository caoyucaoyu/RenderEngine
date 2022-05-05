#pragma once

#include "Primitive.h"
#include "GPUMeshBuffer.h"


class RenderScene
{
public:
	RenderScene();
	~RenderScene();

	int GetPrimitivesCount() { return Primitives.size(); }
	std::vector<Primitive> GetPrimitives() { return Primitives; }

private:
	std::vector<Primitive> Primitives;
	std::unordered_map<std::string, GPUMeshBuffer> MeshBuffers;

};


