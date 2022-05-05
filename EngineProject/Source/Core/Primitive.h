#pragma once

#include "GPUMeshBuffer.h"

//‰÷»æœÓ

//

class Primitive
{
public:
	Primitive();
	~Primitive();

	GPUMeshBuffer* GetMeshBuffer() { return MeshBuffer; }

private:
	GPUMeshBuffer* MeshBuffer;
};