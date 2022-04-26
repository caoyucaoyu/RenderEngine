#include "stdafx.h"
#include "ForwardRenderer.h"
#include "RHI/RHI.h"

ForwardRenderer::ForwardRenderer()
{
	RHI::CreateRHI();
	RHI::Get()->Init();
}

ForwardRenderer::~ForwardRenderer()
{

}

void ForwardRenderer::Init()
{

}

void ForwardRenderer::Render()
{
	//OldRun
	//Engine::Get()->GetRender()->Render();
}

void ForwardRenderer::RenderFrameEnd()
{

}

void ForwardRenderer::DestroyRenderer()
{
	RHI::DestroyRHI();
}

void ForwardRenderer::RenderFrameBegin()
{

}
