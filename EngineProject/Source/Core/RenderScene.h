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
	std::vector<Primitive*> Primitives;//不是ID Map 删除场景物体功能没有
	std::unordered_map<std::string, GPUMeshBuffer*> MeshBuffers;

	std::vector<GPUCommonBuffer*> CameraCommonBuffers;
};


