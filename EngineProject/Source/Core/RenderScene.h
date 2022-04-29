#pragma once

#include "Primitive.h"

class RenderScene
{
public:
	RenderScene();
	~RenderScene();

	int GetPrimitivesCount(){};
	std::vector<Primitive> Primitives;
	//std::unordered_map<std::string, DXMesh> DXMeshs;
};


