#pragma once
#include "GPUMeshBuffer.h"

class RHI
{
public:
	RHI();
	virtual ~RHI();

	static RHI* Get();
	static void CreateRHI();
	static void DestroyRHI();

	static RHI* MRHI;

public:
	virtual void Init() = 0;

	virtual void BeginFrame() = 0;
	virtual void EndFrame() = 0;

	virtual void CreateFrameResource() = 0;
	virtual void UpdateFrameResource() = 0;

	virtual void FlushCommandQueue() = 0;
	virtual void ResizeWindow(UINT32 Width, UINT32 Height) = 0;
	virtual void DrawInstanced(UINT DrawIndexCount) = 0;

	virtual void SetRenderTargetBegin() = 0;
	virtual void SetRenderTargetEnd() = 0;

	virtual void SetGraphicsPipeline() = 0;
	virtual void IASetMeshBuffer(GPUMeshBuffer* GPUMeshbuffer) = 0;
};