#pragma once

#include "D3dUtil.h"

template<typename T>
class UploadBuffer
{
public:
	UploadBuffer(ID3D12Device* Device, UINT ElementCount, bool IsConstantBuffer) :mIsConstantBuffer(IsConstantBuffer)
	{
		ElementByteSize = sizeof(T);
		if (IsConstantBuffer)
			ElementByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(T));

		Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(ElementByteSize * ElementCount),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&AUploadBuffer));

		AUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&MappedData));
	}

	UploadBuffer(const UploadBuffer& rhs) = delete;
	UploadBuffer& operator=(const UploadBuffer& rhs) = delete;
	~UploadBuffer()
	{
		if (AUploadBuffer != nullptr)
			AUploadBuffer->Unmap(0, nullptr);

		MappedData = nullptr;
	}

	ID3D12Resource* Resource()const
	{
		return AUploadBuffer.Get();
	}

	void CopyData(int elementIndex, const T& data)
	{
		memcpy(&MappedData[elementIndex * ElementByteSize], &data, sizeof(T));
	}

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> AUploadBuffer;
	BYTE* MappedData = nullptr;

	UINT ElementByteSize = 0;
	bool mIsConstantBuffer = false;
};