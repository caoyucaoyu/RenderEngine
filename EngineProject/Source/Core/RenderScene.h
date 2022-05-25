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
	GPUCommonBuffer* GetMainPassBuffer(int Idx);
	GPUTexture* FindGPUTexture(std::string Name);

private:
	std::vector<Primitive*> Primitives;
	//不是ID Map 删除场景物体功能没有

	std::unordered_map<std::string, GPUMeshBuffer*> GPUMeshBuffers;

	//我RenderScene有MainPassCommonBuffer了
	//现在就是换了个Shader，俩Shader接不同的值，就一个P矩阵不一样,我他妈需要再创建一个CommonBuffer，感觉有点怪啊。
	std::vector<GPUCommonBuffer*> MainPassCommonBuffers;
	std::vector<GPUCommonBuffer*> MainPassCommonBuffersLight;


	std::unordered_map<std::string, GPUTexture*> GPUTextures;

	std::unordered_map<std::string, Material*> GPUMaterials;
};


