#pragma once

#include "Primitive.h"
#include "GPUMeshBuffer.h"
#include "GPUTexture.h"
#include "Material.h"

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
	//void AddTexture(Texture* InTexture);

	void UpdateMainPassBuffer(PassConstants NewPasConstants);
	
	GPUMeshBuffer* GetMeshBuffer(std::string MeshBufferName);
	GPUCommonBuffer* GetCameraBuffer(int Idx);

	GPUTexture* FindGPUTexture(std::string Name);

private:
	std::vector<Primitive*> Primitives;
	//不是ID Map 删除场景物体功能没有

	std::unordered_map<std::string, GPUMeshBuffer*> GPUMeshBuffers;

	std::vector<GPUCommonBuffer*> CameraCommonBuffers;

	//选用哪一个？
	//std::vector<GPUTexture*>Textures;
	std::unordered_map<std::string, GPUTexture*> GPUTextures;

	std::unordered_map<std::string, Material*> GPUMaterials;
};


