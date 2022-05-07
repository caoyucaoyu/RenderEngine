#pragma once

#include "GPUCommonBuffer.h"

class DX12GPUCommonBuffer :public GPUCommonBuffer
{
public:
	DX12GPUCommonBuffer(ID3D12Device* Device, UINT ElementCount, bool IsConstantBuffer, UINT ElementByteSize);// :mIsConstantBuffer(IsConstantBuffer)
	DX12GPUCommonBuffer(const DX12GPUCommonBuffer& rhs) = delete;
	DX12GPUCommonBuffer& operator=(const DX12GPUCommonBuffer& rhs) = delete;
	~DX12GPUCommonBuffer();

	void CopyData(int elementIndex, std::shared_ptr<void> data) override;

public:
	ID3D12Resource* Resource()const;
	void SetBufferHandle(CD3DX12_CPU_DESCRIPTOR_HANDLE Bufferhandle);
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetHandle() {return BufferHandle;}
	UINT GetElementByteSize() {return MElementByteSize;}

private:
	UINT CalcConstantBufferByteSize(UINT byteSize);

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> AUploadBuffer;
	BYTE* MappedData = nullptr;
	UINT MElementByteSize = 0;
	bool MIsConstantBuffer = false;

	CD3DX12_CPU_DESCRIPTOR_HANDLE BufferHandle;
};
