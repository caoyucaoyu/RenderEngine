#include "stdafx.h"
#include "Renderer.h"
#include "RHI/RHI.h"
#include "Engine.h"


Renderer::Renderer()
{

}

Renderer::~Renderer()
{

}

void Renderer::Init()
{
	//RHI::CreateRHI();
	//RHI::Get()->Init();
}

void Renderer::Render()
{
	Engine::Get()->GetRender()->Render();
}
