#pragma once
#include "GPUMeshBuffer.h"
#include "GPUCommonBuffer.h"

class RenderScene;

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
	virtual void RebuildFrameResource(RenderScene* MRenderScene) = 0;
	virtual void UpdateFrameResource() = 0;

	virtual int GetCurFrameResourceIdx() = 0;
	virtual int GetFrameResourceCount() = 0;

	virtual void FlushCommandQueue() = 0;
	virtual void ResizeWindow(UINT32 Width, UINT32 Height) = 0;
	virtual void DrawIndexedInstanced(UINT DrawIndexCount) = 0;

	virtual void SetRenderTargetBegin() = 0;
	virtual void SetRenderTargetEnd() = 0;

	virtual void SetGraphicsPipeline() = 0;//..................................................................................
	virtual void SetRenderResourceTable(int Nu, UINT32 HeapOffset) = 0;//.....................................................
	virtual void IASetMeshBuffer(GPUMeshBuffer* GPUMeshbuffer) = 0;

	virtual GPUMeshBuffer* CreateMeshBuffer() = 0;
	virtual void UpdateMeshBuffer(GPUMeshBuffer* GpuMeshBuffer) = 0;

	virtual GPUCommonBuffer* CreateCommonBuffer(UINT ElementCount, bool IsConstantBuffer, UINT ElementByteSize) = 0;
	virtual void UpdateCommonBuffer(GPUCommonBuffer* GpuCommonBuffer, std::shared_ptr<void> Data, int elementIndex) = 0;
	virtual void AddCommonBuffer(int FrameSourceIndex,std::string, GPUCommonBuffer* GpuCommonBuffer) = 0;

	virtual void XXX() = 0;
};