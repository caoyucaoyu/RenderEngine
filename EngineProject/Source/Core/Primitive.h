#pragma once

#include "GPUMeshBuffer.h"
#include "GPUCommonBuffer.h"
//��Ⱦ��

//

class Primitive
{
public:
	Primitive();
	~Primitive();

	GPUMeshBuffer* GetMeshBuffer() { return MeshBuffer; }
	void SetMeshBuffer(GPUMeshBuffer* AMeshBuffer) { MeshBuffer = AMeshBuffer; }

	std::vector<GPUCommonBuffer*> GetObjectCommonBuffers() { return ObjectCommonBuffers; }
	GPUCommonBuffer* GetObjectCommonBuffer(int Idx) { return ObjectCommonBuffers[Idx]; }
	void SetObjectCommonBuffer(GPUCommonBuffer* AObjectCommonBuffer) { ObjectCommonBuffers.push_back(AObjectCommonBuffer); }

private:
	GPUMeshBuffer* MeshBuffer;
	std::vector<GPUCommonBuffer*> ObjectCommonBuffers; //�Ժ� ���ĳ�ID ��Ӧ��ǰ֡�� CommonBuffer

};