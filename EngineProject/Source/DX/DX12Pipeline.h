#pragma once

#include "Pipeline.h"

using Microsoft::WRL::ComPtr;

class DX12Pipeline : public Pipeline
{
public:
	DX12Pipeline();
	~DX12Pipeline(); 

	ComPtr<ID3D12PipelineState> GetPipelineState();
	void SetPipelineState(ComPtr<ID3D12PipelineState> PipelineState);
private:
	ComPtr<ID3D12PipelineState> PipelineState; 
};

