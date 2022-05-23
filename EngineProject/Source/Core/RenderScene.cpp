#include "stdafx.h"
#include "RenderScene.h"
#include "Scene.h"

#include "Engine.h"
#include "RHI.h"

using namespace std;


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
	for (auto it : GPUMeshBuffers)
	{
		if (it.second != nullptr)
		{
			delete it.second;
			it.second = nullptr;
		}
	}
	for (auto it : GPUTextures)
	{
		if (it.second != nullptr)
		{
			delete it.second;
			it.second = nullptr;
		}
	}
}

void RenderScene::AddMapData(std::vector<MeshActor*> MeshActors)
{
	for (auto AMeshActor : MeshActors)
	{
		AddMeshBuffer(AMeshActor);
		AddPrimitive(AMeshActor);
	}
}

void RenderScene::AddMeshBuffer(MeshActor* AMeshActor)
{
	if (GPUMeshBuffers.count(AMeshActor->MeshName))
		return;

	StaticMesh StaticMeshData;
	if (Engine::Get()->GetAssetsManager()->FindMesh(AMeshActor->MeshName, StaticMeshData))
	{
		GPUMeshBuffer* InMeshBuffer = RHI::Get()->CreateMeshBuffer();
		InMeshBuffer->SetData(StaticMeshData);
		RHI::Get()->UpdateMeshBuffer(InMeshBuffer);
		GPUMeshBuffers.insert({ AMeshActor->MeshName , InMeshBuffer });
	}
}

void RenderScene::AddPrimitive(MeshActor* AMeshActor)
{
	auto MeshBuffer = GetMeshBuffer(AMeshActor->MeshName);
	if (!MeshBuffer)return;

	Primitive* NewPrimitive = new Primitive();
	NewPrimitive->SetMeshBuffer(MeshBuffer);

	Material* AMaterial = Engine::Get()->GetAssetsManager()->GetMaterial(AMeshActor->MaterialName);

	if (AMaterial != nullptr && GPUMaterials.count(AMaterial->Name) == 0)
	{
		GPUMaterials.insert({AMaterial->Name,AMaterial});
		for (auto ATexture : AMaterial->Textures)
		{
			//Ð´³ÉAddTextureÒ²ÐÐ
			auto InTexture = ATexture.second;

			if (GPUTextures.count(InTexture->Name))break;
			GPUTexture* GTexture = RHI::Get()->CreateTexture(InTexture->Name, InTexture->Filename);
			GPUTextures.insert({ GTexture->Name,GTexture });
		}
	}
	else
	{
		AMaterial = Engine::Get()->GetAssetsManager()->GetDefaultMaterial();
	}
	NewPrimitive->SetMaterial(AMaterial);


	for (int i = 0; i < RHI::Get()->GetFrameResourceCount(); i++)
	{
		GPUCommonBuffer* ObjectCommonBuffer = RHI::Get()->CreateCommonBuffer(1, true, sizeof(ObjectConstants));

		auto Trans = std::make_shared<ObjectConstants>();
		Trans->Location_M = glm::transpose(AMeshActor->Location_Matrix);
		Trans->Rotation_M = glm::transpose(AMeshActor->Rotation_Matrix);
		Trans->Scale3D_M = glm::transpose(AMeshActor->Scale3D_Matrix);

		std::shared_ptr<void> TransData = Trans;
		RHI::Get()->UpdateCommonBuffer(ObjectCommonBuffer, Trans, 0);
		RHI::Get()->AddCommonBuffer(i, AMeshActor->MeshName, ObjectCommonBuffer);
		NewPrimitive->SetObjectCommonBuffer(ObjectCommonBuffer);
		

		GPUCommonBuffer* MaterialCommonBuffer = RHI::Get()->CreateCommonBuffer(1, true, sizeof(MaterialConstants));

		auto Mat = std::make_shared<MaterialConstants>();
		Mat->MatTransform = glm::mat4(1.0f);
		Mat->DiffuseAlbedo = AMaterial->DiffuseAlbedo;
		Mat->FresnelR0 = AMaterial->FresnelR0;
		Mat->Roughness = AMaterial->Roughness;

		std::shared_ptr<void> MatData = Mat;
		RHI::Get()->UpdateCommonBuffer(MaterialCommonBuffer, Mat, 0);
		RHI::Get()->AddCommonBuffer(i, "Material Name", MaterialCommonBuffer);
		NewPrimitive->SetMaterialCommonBuffer(MaterialCommonBuffer);

		//NewPrimitive->SetPipeline();
	}

	Primitives.push_back(NewPrimitive);
}


void RenderScene::UpdateMainPassBuffer(PassConstants NewPasConstants)
{
	std::shared_ptr<void> CameraData = std::make_shared<PassConstants>(NewPasConstants);
	int Idx = RHI::Get()->GetCurFrameResourceIdx();
	RHI::Get()->UpdateCommonBuffer(CameraCommonBuffers[Idx], CameraData, 0);

	//std::cout<<"Update CommonBuffer\n";
}

GPUMeshBuffer* RenderScene::GetMeshBuffer(std::string MeshBufferName)
{
	if (GPUMeshBuffers.count(MeshBufferName))
	{
		return GPUMeshBuffers.at(MeshBufferName);
	}
	return nullptr;
}

GPUCommonBuffer* RenderScene::GetCameraBuffer(int Idx)
{
	return CameraCommonBuffers[Idx];
}

GPUTexture* RenderScene::FindGPUTexture(std::string Name)
{
	if (GPUTextures.count(Name))
	{
		return GPUTextures.at(Name);
	}
	else
	{
		return nullptr;
	}
}
