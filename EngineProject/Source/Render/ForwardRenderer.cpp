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
	MRenderScene->Init();//´´½¨CameraAndTime ConstantBuffer
}

void ForwardRenderer::RenderFrameBegin()
{
	RHI::Get()->BeginFrame();
	RHI::Get()->XXX();
}

void ForwardRenderer::Render()
{
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

void ForwardRenderer::HDRPass()
{
	RHI::Get()->SetRenderTargetBegin();
	RHI::Get()->SetGraphicsPipeline();

	GPUCommonBuffer* CameraBuffer = MRenderScene->GetCameraBuffer(CurrentRenderIndex);
	RHI::Get()->SetRenderResourceTable(1, CameraBuffer->GetHandleOffset());

	auto Primitives = MRenderScene->GetPrimitives();
	for (auto Primitive : Primitives)
	{
		RHI::Get()->IASetMeshBuffer(Primitive->GetMeshBuffer());
		RHI::Get()->SetRenderResourceTable(0,Primitive->GetObjectCommonBuffer(CurrentRenderIndex)->GetHandleOffset());
		RHI::Get()->DrawIndexedInstanced(Primitive->GetMeshBuffer()->GetIndices().size());
	}

	RHI::Get()->SetRenderTargetEnd();
}


