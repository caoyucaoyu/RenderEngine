#pragma once

#include "GPUMeshBuffer.h"

//��Ⱦ��

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