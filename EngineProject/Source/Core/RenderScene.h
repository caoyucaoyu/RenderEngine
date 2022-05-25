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
	//����ID Map ɾ���������幦��û��

	std::unordered_map<std::string, GPUMeshBuffer*> GPUMeshBuffers;

	//��RenderScene��MainPassCommonBuffer��
	//���ھ��ǻ��˸�Shader����Shader�Ӳ�ͬ��ֵ����һ��P����һ��,��������Ҫ�ٴ���һ��CommonBuffer���о��е�ְ���
	std::vector<GPUCommonBuffer*> MainPassCommonBuffers;
	std::vector<GPUCommonBuffer*> MainPassCommonBuffersLight;


	std::unordered_map<std::string, GPUTexture*> GPUTextures;

	std::unordered_map<std::string, Material*> GPUMaterials;
};


