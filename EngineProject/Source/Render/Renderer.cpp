#include "stdafx.h"
#include "Renderer.h"

Renderer::Renderer()
{
	
}

Renderer::~Renderer()
{

}

RenderScene* Renderer::GetRenderScene()
{
	return MRenderAssets;
}
