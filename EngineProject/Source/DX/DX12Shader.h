#pragma once
#include "Shader.h"

using Microsoft::WRL::ComPtr;

class DX12Shader : public Shader
{
public:
	DX12Shader();
	~DX12Shader();
	ComPtr<ID3D12RootSignature> GetRootSignatureDx();

private:
	ComPtr<ID3D12RootSignature> RootSignature;
};