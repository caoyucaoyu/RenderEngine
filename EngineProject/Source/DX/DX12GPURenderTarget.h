#pragma once
#include "GPURenderTarget.h"
#include "DX12GPURenderTargetBuffer.h"

class DX12GPURenderTarget : public GPURenderTarget
{
public:
	DX12GPURenderTarget() {};
	DX12GPURenderTarget(std::string Name, UINT W, UINT H);
	~DX12GPURenderTarget() {};

	D3D12_VIEWPORT Viewport;
	D3D12_RECT ScissorRect;
};