#pragma once
#include "GPURenderTargetBuffer.h"
#include "DescriptorHeap.h"

class DX12GPURenderTargetBuffer :public GPURenderTargetBuffer
{
public:
	DX12GPURenderTargetBuffer();
	DX12GPURenderTargetBuffer(RTBufferType Type, UINT W, UINT H);
	~DX12GPURenderTargetBuffer();
	void InitDepthStencilType();
	void InitColorType();

	void CreateResource(ID3D12Device* Device);
	void CreateView(ID3D12Device* Device,FAllocation Allocatiuon);

	virtual void ResetResource();
	virtual void SetResourceName(std::string ResourceName);

	Microsoft::WRL::ComPtr<ID3D12Resource> GetResource() { return RTResource; }
	void SetResource(Microsoft::WRL::ComPtr<ID3D12Resource> Resource) { RTResource = Resource;}

	D3D12_RESOURCE_DESC Desc;
	D3D12_RESOURCE_DESC DepthStencilDesc;
	D3D12_RESOURCE_DESC ColorDesc;
	D3D12_RESOURCE_STATES ResourceState;

	D3D12_CLEAR_VALUE OptClear;

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> RTResource;

	DXGI_FORMAT Format;
};


