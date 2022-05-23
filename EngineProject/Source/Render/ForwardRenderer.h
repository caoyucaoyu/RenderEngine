#pragma once

#include "Renderer.h"

class GPURenderTarget;
class ForwardRenderer :public Renderer
{
public:
	ForwardRenderer();
	~ForwardRenderer();

	virtual void Init() override;
	virtual void DestroyRenderer() override;
	virtual void Update() override;
	virtual void RenderFrameBegin() override;
	virtual void Render() override;
	virtual void RenderFrameEnd() override;

private:
	void HDRPass();
	void ShadowMapPass();

private:
	int Width = 1280;
	int Hight = 720;
	int CurrentRenderIndex = 0;

	GPURenderTarget* RTShadowMap;
};