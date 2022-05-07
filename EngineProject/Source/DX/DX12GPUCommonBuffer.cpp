#include "stdafx.h"
#include "DX12GPUCommonBuffer.h"



DX12GPUCommonBuffer::DX12GPUCommonBuffer(ID3D12Device* Device, UINT ElementCount, bool IsConstantBuffer, UINT ElementByteSize)
{
	MIsConstantBuffer = IsConstantBuffer;

	//ElementByteSize = sizeof(T);
	MElementByteSize = ElementByteSize;

	if (IsConstantBuffer)
		MElementByteSize = CalcConstantBufferByteSize(ElementByteSize);//最后一个原sizeof(T)

	Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(MElementByteSize * ElementCount),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&AUploadBuffer));

	AUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&MappedData));
}

DX12GPUCommonBuffer::~DX12GPUCommonBuffer()
{
	if (AUploadBuffer != nullptr)
		AUploadBuffer->Unmap(0, nullptr);

	MappedData = nullptr;
}

ID3D12Resource* DX12GPUCommonBuffer::Resource() const
{
	return AUploadBuffer.Get();
}


void DX12GPUCommonBuffer::SetBufferHandle(CD3DX12_CPU_DESCRIPTOR_HANDLE Bufferhandle)
{
	BufferHandle = Bufferhandle;
}

void DX12GPUCommonBuffer::CopyData(int elementIndex, std::shared_ptr<void> data)
{
	memcpy(&MappedData[elementIndex * MElementByteSize], data.get(), MElementByteSize);//最后一个原sizeof(T)
}

UINT DX12GPUCommonBuffer::CalcConstantBufferByteSize(UINT byteSize)
{
	return (byteSize + 255) & ~255;//..
}
