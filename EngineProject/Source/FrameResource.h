#pragma once
#include "stdafx.h"
#include "D3DUtil.h"
#include "UploadBuffer.h"

struct FrameResource
{
public:

	FrameResource(ID3D12Device* Device, UINT PassCount, UINT ObjectCount);
	FrameResource(const FrameResource& rhs) = delete;
	FrameResource& operator=(const FrameResource& rhs) = delete;
	~FrameResource();

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc;

	std::unique_ptr<UploadBuffer<PassConstants>> PassCB = nullptr;
	std::unique_ptr<UploadBuffer<ObjectConstants>> ObjectCB = nullptr;
	//std::unique_ptr<UploadBuffer<TimeConstants>> TimeCB = nullptr;

	UINT64 Fence = 0;
};


