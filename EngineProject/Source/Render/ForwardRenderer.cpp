#include "stdafx.h"
#include "ForwardRenderer.h"
#include "RHI/RHI.h"
#include "RenderScene.h"


ForwardRenderer::ForwardRenderer()
{
	RHI::CreateRHI();
	MRenderScene = new RenderScene();
}

ForwardRenderer::~ForwardRenderer()
{

}

void ForwardRenderer::DestroyRenderer()
{
	RHI::DestroyRHI();
	if (MRenderScene != nullptr)
	{
		delete MRenderScene;
		MRenderScene = nullptr;
	}
}

void ForwardRenderer::Init()
{
	RHI::Get()->Init();
	RHI::Get()->ResizeWindow(1280, 720);
	MRenderScene->Init();//创建CameraAndTime ConstantBuffer
	RHI::Get()->XXX();
}

void ForwardRenderer::RenderFrameBegin()
{
	RHI::Get()->BeginFrame();
}

void ForwardRenderer::Render()
{
	ShadowMapPass();
	HDRPass();

	//Engine::Get()->GetRender()->Render();
}

void ForwardRenderer::RenderFrameEnd()
{
	RHI::Get()->EndFrame();
}

void ForwardRenderer::Update()
{
	RHI::Get()->UpdateFrameResource();
	CurrentRenderIndex = RHI::Get()->GetCurFrameResourceIdx();
}

void ForwardRenderer::ShadowMapPass()
{
	if (!RTShadowMap)
	{
		RTShadowMap = RHI::Get()->CreateRenderTarget("ShadowMap", Width, Hight);
	}
}

void ForwardRenderer::HDRPass()
{
	RHI::Get()->SetRenderTargetBegin();

	RHI::Get()->SetGraphicsPipeline(nullptr);

	GPUCommonBuffer* CameraBuffer = MRenderScene->GetCameraBuffer(CurrentRenderIndex);
	RHI::Get()->SetRenderResourceTable(1, CameraBuffer->GetHandleOffset());

	auto Primitives = MRenderScene->GetPrimitives();
	for (auto Primitive : Primitives)
	{
		//Pipeline* PrimitivePipeline = Primitive->GetPipeline(0);

		//RHI::Get()->SetGraphicsPipeline(PrimitivePipeline);
		
		RHI::Get()->IASetMeshBuffer(Primitive->GetMeshBuffer());

		RHI::Get()->SetRenderResourceTable(0, Primitive->GetObjectCommonBuffer(CurrentRenderIndex)->GetHandleOffset());//根参数Index
		RHI::Get()->SetRenderResourceTable(2, Primitive->GetMaterialCommonBuffer(CurrentRenderIndex)->GetHandleOffset());//根参数Index

		RHI::Get()->SetRenderResourceTable(3, MRenderScene->FindGPUTexture("Head_diff")->GetHandleOffset());
		RHI::Get()->SetRenderResourceTable(4, MRenderScene->FindGPUTexture("Head_norm")->GetHandleOffset());

		RHI::Get()->DrawIndexedInstanced(Primitive->GetMeshBuffer()->GetIndices().size());
	}

	RHI::Get()->SetRenderTargetEnd();
}



