#pragma once
#include "GPUMeshBuffer.h"
#include "GPUCommonBuffer.h"
#include "GPUTexture.h"
#include "GPURenderTarget.h"
#include "Pipeline.h"

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
	virtual void UpdateFrameResource() = 0;
	virtual int GetCurFrameResourceIdx() = 0;
	virtual int GetFrameResourceCount() = 0;

	virtual void FlushCommandQueue() = 0;
	virtual void ResizeWindow(UINT32 Width, UINT32 Height) = 0;
	virtual void DrawIndexedInstanced(UINT DrawIndexCount) = 0;

	virtual void SetBackBufferBegin() = 0;
	virtual void SetBackBufferEnd() = 0;
	virtual void SetRenderTargetBegin(GPURenderTarget* RenderTarget) = 0;
	virtual void SetRenderTargetBufferBegin(GPURenderTarget* RenderTarget) = 0;
	virtual void SetRenderTargetBufferEnd(GPURenderTarget* RenderTarget) = 0;

	virtual void SetGraphicsPipeline(Pipeline* NPipeline,int TemporaryType) = 0;
	virtual void SetRenderResourceTable(int Nu, UINT32 HeapOffset) = 0;//.....................................................
	virtual void IASetMeshBuffer(GPUMeshBuffer* GPUMeshbuffer) = 0;

	virtual GPUMeshBuffer* CreateMeshBuffer() = 0;
	virtual void UpdateMeshBuffer(GPUMeshBuffer* GpuMeshBuffer) = 0;

	virtual GPUCommonBuffer* CreateCommonBuffer(UINT ElementCount, bool IsConstantBuffer, UINT ElementByteSize) = 0;
	virtual void UpdateCommonBuffer(GPUCommonBuffer* GpuCommonBuffer, std::shared_ptr<void> Data, int elementIndex) = 0;
	virtual void AddCommonBuffer(int FrameSourceIndex, std::string, GPUCommonBuffer* GpuCommonBuffer) = 0;

	virtual GPUTexture* CreateTexture(std::string TextureName, std::wstring FileName) = 0;
	virtual GPUTexture* CreateTexture(std::string TextureName, GPURenderTargetBuffer* RTBuffer) = 0;

	virtual GPURenderTarget* CreateRenderTarget(std::string RTName, UINT W, UINT H) = 0;
	virtual GPURenderTargetBuffer* CreateRenderTargetBuffer(RTBufferType Type, UINT W, UINT H) = 0;

	virtual void RootSignatureAndPSO() = 0;

protected:
	static GPURenderTarget* BackBufferRT;
};


