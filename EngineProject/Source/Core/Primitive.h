#pragma once

#include "GPUMeshBuffer.h"
#include "GPUCommonBuffer.h"
//��Ⱦ��

//
class Pipeline;
class Primitive
{
public:
	Primitive();
	~Primitive();

	GPUMeshBuffer* GetMeshBuffer() { return MeshBuffer; }
	void SetMeshBuffer(GPUMeshBuffer* AMeshBuffer) { MeshBuffer = AMeshBuffer; }

	std::vector<GPUCommonBuffer*> GetObjectCommonBuffers() { return ObjectCommonBuffers; }
	GPUCommonBuffer* GetObjectCommonBuffer(int Idx) { return ObjectCommonBuffers[Idx]; }
	void SetObjectCommonBuffer(GPUCommonBuffer* AObjectCommonBuffer)
	{
		ObjectCommonBuffers.push_back(AObjectCommonBuffer);
	}

	std::vector<GPUCommonBuffer*> GetMaterialCommonBuffers() { return MaterialCommonBuffers; }
	GPUCommonBuffer* GetMaterialCommonBuffer(int Idx) { return MaterialCommonBuffers[Idx]; }
	void SetMaterialCommonBuffer(GPUCommonBuffer* AMaterialCommonBuffer)
	{
		MaterialCommonBuffers.push_back(AMaterialCommonBuffer);
	}

	void SetMaterial(Material* InMaterial) { MaterialUsed = InMaterial; }
	Material* GetMaterial() { return MaterialUsed; }

	Pipeline* GetPipeline(int Idx) { return Pipelines[Idx]; }
	void SetPipeline(Pipeline* InPipeline) {Pipelines.push_back(InPipeline);}

private:
	GPUMeshBuffer* MeshBuffer;
	std::vector<GPUCommonBuffer*> ObjectCommonBuffers; //�Ժ� ���ĳ�ID ��Ӧ��ǰ֡�� CommonBuffer
	std::vector<GPUCommonBuffer*> MaterialCommonBuffers;

	std::vector<Pipeline*> Pipelines;

	Material* MaterialUsed;

};


