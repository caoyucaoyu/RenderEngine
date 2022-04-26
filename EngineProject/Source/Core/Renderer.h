#pragma once


class RenderScene;
class Renderer
{
public:
	Renderer();
	~Renderer();

	virtual void Init() = 0;
	virtual void DestroyRenderer() = 0;

	virtual void RenderFrameBegin() = 0;
	virtual void Render() = 0;
	virtual void RenderFrameEnd() = 0;

	RenderScene* GetRenderScene();

protected:
	RenderScene* MRenderAssets;

};








