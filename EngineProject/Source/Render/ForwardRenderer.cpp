#include "stdafx.h"
#include "ForwardRenderer.h"
#include "RHI/RHI.h"

ForwardRenderer::ForwardRenderer()
{
	RHI::CreateRHI();
}

ForwardRenderer::~ForwardRenderer()
{

}

void ForwardRenderer::DestroyRenderer()
{
	RHI::DestroyRHI();
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

	//OldRun
	//Engine::Get()->GetRender()->Render();
}

void ForwardRenderer::RenderFrameEnd()
{
	RHI::Get()->EndFrame();
	//OutputDebugStringA("RenderFrameEnd\n");
}

void ForwardRenderer::Update()
{
	RHI::Get()->UpdateFrameResource();
}

void ForwardRenderer::HDRPass()
{
	RHI::Get()->SetRenderTargetBegin();

	RHI::Get()->SetRenderTargetEnd();
}


