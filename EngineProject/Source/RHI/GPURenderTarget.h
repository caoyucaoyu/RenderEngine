#pragma once

#include "GPURenderTargetBuffer.h"

class GPURenderTarget
{
public:
	GPURenderTarget();
	GPURenderTarget(std::string Name,UINT W,UINT H);
	virtual ~GPURenderTarget();

	virtual void AddColorBuffer(UINT32 BufferNum);
	virtual void AddDepthStencilBuffer();

	virtual GPURenderTargetBuffer* GetColorBuffer(int Index);
	virtual GPURenderTargetBuffer* GetDepthStencilBuffer();
	virtual std::vector<GPURenderTargetBuffer*> GetColorBuffers() {return RTColorBuffers;}

	UINT Width = 1280;
	UINT Hight = 720;

protected:
	std::string RenderTargetName ="";
	std::vector<GPURenderTargetBuffer*> RTColorBuffers;
	GPURenderTargetBuffer* RTDepthStencilBuffer;

};