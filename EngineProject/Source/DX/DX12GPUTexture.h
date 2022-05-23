#pragma once
#include "GPUTexture.h"
#include "D3DUtil.h"

class DX12GPUTexture : public GPUTexture
{
public:
	DX12GPUTexture();
	~DX12GPUTexture();


	Microsoft::WRL::ComPtr<ID3D12Resource> TResource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap = nullptr;

};