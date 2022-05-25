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
	if (RTShadowMap != nullptr)
	{
		delete RTShadowMap;
		RTShadowMap = nullptr;
	}
}

void ForwardRenderer::Init()
{
	RHI::Get()->Init();
	RHI::Get()->ResizeWindow(1280, 720);
	MRenderScene->Init();//����CameraAndTime ConstantBuffer
	RHI::Get()->RootSignatureAndPSO();
}

void ForwardRenderer::RenderFrameBegin()
{
	RHI::Get()->BeginFrame();
}

void ForwardRenderer::Render()
{
	ShadowMapPass();
	HDRPass();
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
		OutputDebugStringA("SS Create ShadowMapRenderTarget\n");
		RTShadowMap = RHI::Get()->CreateRenderTarget("ShadowMap", Width, Hight);
		RTShadowMap->AddDepthStencilBuffer();//Resource Create DSV
		auto RTTexture = RHI::Get()->CreateTexture("ShadowMapTexture", RTShadowMap->GetDepthStencilBuffer());//Resource Create SRV
		RTShadowMap->GetDepthStencilBuffer()->RenderTargetTexture = RTTexture;
	}

	RHI::Get()->SetRenderTargetBufferBegin(RTShadowMap);//��ʼʱ״̬�л�
	RHI::Get()->SetRenderTargetBegin(RTShadowMap); //�����ӿںͲü����� ָ��RTV��DSV �����̨����������Ȼ�����������ֵ

	RHI::Get()->SetGraphicsPipeline(nullptr, 2);//2 ����PSO��2��ShadowMap��Pipeline

	//RHI::Get()->SetRenderResourceTable(1, MRenderScene->GetMainPassBuffer(CurrentRenderIndex)->GetHandleOffset());
	RHI::Get()->SetRenderResourceTable(1, MRenderScene->GetMainPassBuffer(CurrentRenderIndex)->GetHandleOffset());

	auto Primitives = MRenderScene->GetPrimitives();
	for (auto Primitive : Primitives)
	{
		RHI::Get()->IASetMeshBuffer(Primitive->GetMeshBuffer());

		RHI::Get()->SetRenderResourceTable(0, Primitive->GetObjectCommonBuffer(CurrentRenderIndex)->GetHandleOffset());//������Index
		RHI::Get()->SetRenderResourceTable(2, Primitive->GetMaterialCommonBuffer(CurrentRenderIndex)->GetHandleOffset());//������Index
		RHI::Get()->SetRenderResourceTable(3, MRenderScene->FindGPUTexture("Head_diff")->GetHandleOffset());
		RHI::Get()->SetRenderResourceTable(4, MRenderScene->FindGPUTexture("Head_norm")->GetHandleOffset());

		RHI::Get()->DrawIndexedInstanced(Primitive->GetMeshBuffer()->GetIndices().size());
	}

	RHI::Get()->SetRenderTargetBufferEnd(RTShadowMap);//������״̬�л�
}

void ForwardRenderer::HDRPass()
{

	RHI::Get()->SetBackBufferBegin();

	RHI::Get()->SetGraphicsPipeline(nullptr, 0);//1 ����PSO��1��Dynamic��Pipeline

	RHI::Get()->SetRenderResourceTable(1, MRenderScene->GetMainPassBuffer(CurrentRenderIndex)->GetHandleOffset());

	auto Primitives = MRenderScene->GetPrimitives();
	for (auto Primitive : Primitives)
	{
		//Pipeline* PrimitivePipeline = Primitive->GetPipeline(0);

		//RHI::Get()->SetGraphicsPipeline(PrimitivePipeline);
		
		RHI::Get()->IASetMeshBuffer(Primitive->GetMeshBuffer());

		RHI::Get()->SetRenderResourceTable(0, Primitive->GetObjectCommonBuffer(CurrentRenderIndex)->GetHandleOffset());//������Index
		RHI::Get()->SetRenderResourceTable(2, Primitive->GetMaterialCommonBuffer(CurrentRenderIndex)->GetHandleOffset());//������Index

		RHI::Get()->SetRenderResourceTable(3, MRenderScene->FindGPUTexture("Head_diff")->GetHandleOffset());
		RHI::Get()->SetRenderResourceTable(4, MRenderScene->FindGPUTexture("Head_norm")->GetHandleOffset());
		RHI::Get()->SetRenderResourceTable(5, MRenderScene->FindGPUTexture("Head_norm")->GetHandleOffset());//���ͼ��������

		RHI::Get()->DrawIndexedInstanced(Primitive->GetMeshBuffer()->GetIndices().size());
	}

	RHI::Get()->SetBackBufferEnd();
}



