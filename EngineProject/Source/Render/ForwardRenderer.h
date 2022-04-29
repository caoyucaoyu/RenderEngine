#pragma once

#include "Renderer.h"


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
;};