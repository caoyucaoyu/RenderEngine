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
}

void ForwardRenderer::RenderFrameBegin()
{
	RHI::Get()->BeginFrame();
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
}

void ForwardRenderer::HDRPass()
{
	RHI::Get()->SetRenderTargetBegin();

	auto Primitives = MRenderScene->GetPrimitives();
	for (auto Primitive : Primitives)
	{
		RHI::Get()->IASetMeshBuffer(Primitive.GetMeshBuffer());
		RHI::Get()->DrawInstanced(Primitive.GetMeshBuffer()->GetIndices().size());
	}


	RHI::Get()->SetRenderTargetEnd();
}


