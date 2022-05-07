#pragma once

#include "Primitive.h"
#include "GPUMeshBuffer.h"

struct MeshActor;

class RenderScene
{
public:
	RenderScene();
	~RenderScene();

	void Init();
	void DestroyRenderScene();

	int GetPrimitivesCount() { return Primitives.size(); }
	std::vector<Primitive*> GetPrimitives() { return Primitives; }

	void AddMapData(std::vector<MeshActor*> MeshActors);
	void AddMeshBuffer(MeshActor* AMeshActor);
	void AddPrimitive(MeshActor* AMeshActor);
	void UpdateCameraBuffer(PassConstants NewPasConstants);

	GPUMeshBuffer* GetMeshBuffer(std::string MeshBufferName);
	GPUCommonBuffer* GetCameraBuffer(int Idx);

private:
	std::vector<Primitive*> Primitives;//����ID Map ɾ���������幦��û��
	std::unordered_map<std::string, GPUMeshBuffer*> MeshBuffers;

	std::vector<GPUCommonBuffer*> CameraCommonBuffers;
};


