#include "stdafx.h"
#include "DX12Shader.h"

ComPtr<ID3D12RootSignature> DX12Shader::GetRootSignatureDx()
{
	return RootSignature;
}
