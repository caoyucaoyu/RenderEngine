#include "stdafx.h"
#include "RenderScene.h"
#include "Scene.h"

#include "Engine.h"
#include "RHI.h"


RenderScene::RenderScene()
{

}

RenderScene::~RenderScene()
{
	DestroyRenderScene();
}

void RenderScene::Init()
{	
	int FRCount = RHI::Get()->GetFrameResourceCount();
	CameraCommonBuffers.resize(FRCount);
	for (int i = 0; i < FRCount; i++)
	{
		CameraCommonBuffers[i] = RHI::Get()->CreateCommonBuffer(1, true, sizeof(PassConstants));
		RHI::Get()->AddCommonBuffer(i, "Tmd", CameraCommonBuffers[i]);
	}
}

void RenderScene::DestroyRenderScene()
{
	for (auto it : MeshBuffers)
	{
		if (it.second != nullptr)
		{
			delete it.second;
			it.second = nullptr;
		}
	}
	//for (auto it : Primitives)
}

void RenderScene::AddMapData(std::vector<MeshActor*> MeshActors)
{
	//std::cout << "Wei!" << std::endl;
	for (auto AMeshActor : MeshActors)
	{
		AddMeshBuffer(AMeshActor);
		AddPrimitive(AMeshActor);
	}
}

void RenderScene::AddMeshBuffer(MeshActor* AMeshActor)
{
	if (MeshBuffers.count(AMeshActor->MeshName))
		return;

	StaticMesh StaticMeshData;
	if (Engine::Get()->GetAssetsManager()->FindMesh(AMeshActor->MeshName, StaticMeshData))
	{
		GPUMeshBuffer* InMeshBuffer = RHI::Get()->CreateMeshBuffer();
		InMeshBuffer->SetData(StaticMeshData);
		RHI::Get()->UpdateMeshBuffer(InMeshBuffer);
		MeshBuffers.insert({ AMeshActor->MeshName , InMeshBuffer });
	}
}

void RenderScene::AddPrimitive(MeshActor* AMeshActor)
{
	auto MeshBuffer = GetMeshBuffer(AMeshActor->MeshName);
	if (!MeshBuffer)return;
	Primitive* NewPrimitive = new Primitive();

	NewPrimitive->SetMeshBuffer(MeshBuffer);

	for (int i = 0; i < RHI::Get()->GetFrameResourceCount(); i++)
	{
		GPUCommonBuffer* GpuCommonBuffer = RHI::Get()->CreateCommonBuffer(1, true, sizeof(ObjectConstants));
	
		auto Trans = std::make_shared<ObjectConstants>();

		glm::mat4 L = AMeshActor->Location_Matrix;
		glm::mat4 R = AMeshActor->Rotation_Matrix;
		glm::mat4 S = AMeshActor->Scale3D_Matrix;

		Trans->Location_M = glm::transpose(L);
		Trans->Rotation_M = glm::transpose(R);
		Trans->Scale3D_M = glm::transpose(S);

		//Trans->Location_M = glm::transpose(AMeshActor->Location_Matrix);
		//Trans->Rotation_M = glm::transpose(AMeshActor->Rotation_Matrix);
		//Trans->Scale3D_M = glm::transpose(AMeshActor->Scale3D_Matrix);

		std::shared_ptr<void> TransData = Trans;
		RHI::Get()->UpdateCommonBuffer(GpuCommonBuffer, Trans, 0);
		RHI::Get()->AddCommonBuffer(i, AMeshActor->MeshName, GpuCommonBuffer);

		NewPrimitive->SetObjectCommonBuffer(GpuCommonBuffer);
	}

	Primitives.push_back(NewPrimitive);
}

void RenderScene::UpdateCameraBuffer(PassConstants NewPasConstants)
{
	std::shared_ptr<void> CameraData = std::make_shared<PassConstants>(NewPasConstants);
	int Idx = RHI::Get()->GetCurFrameResourceIdx();
	RHI::Get()->UpdateCommonBuffer(CameraCommonBuffers[Idx], CameraData, 0);

	//std::cout<<"Update CommonBuffer\n";
}

GPUMeshBuffer* RenderScene::GetMeshBuffer(std::string MeshBufferName)
{
	if (MeshBuffers.count(MeshBufferName))
	{
		return MeshBuffers.at(MeshBufferName);
	}
	return nullptr;
}

GPUCommonBuffer* RenderScene::GetCameraBuffer(int Idx)
{
	return CameraCommonBuffers[Idx];
}
