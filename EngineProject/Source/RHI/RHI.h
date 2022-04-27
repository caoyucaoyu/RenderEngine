#pragma once

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

	virtual void FlushCommandQueue() = 0;
	virtual void ResizeWindow(UINT32 Width, UINT32 Height) = 0;
	virtual void DrawInstanced(UINT DrawIndexCount) = 0;
};