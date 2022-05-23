#include "stdafx.h"
#include "DX12Pipeline.h"

DX12Pipeline::DX12Pipeline()
{

}

DX12Pipeline::~DX12Pipeline()
{

}

ComPtr<ID3D12PipelineState> DX12Pipeline::GetPipelineState()
{
	return PipelineState;
}

void DX12Pipeline::SetPipelineState(ComPtr<ID3D12PipelineState> PipelineState)
{
	this->PipelineState = PipelineState;
}
