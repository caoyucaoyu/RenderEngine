#pragma once
#include "stdafx.h"
#include "D3DUtil.h"
#include "DX12GPUCommonBuffer.h"


struct FrameResource
{
public:

	FrameResource(ID3D12Device* Device)
	{
		ThrowIfFailed(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(CmdListAlloc.GetAddressOf())));
		this->Device = Device;
	}
	FrameResource(const FrameResource& rhs) = delete;
	FrameResource& operator=(const FrameResource& rhs) = delete;
	~FrameResource()
	{
		for (auto it : CommonBuffers)
		{
			if (it != nullptr)
			{
				delete it;
				it = nullptr;
			}
		}
	};

	std::vector<GPUCommonBuffer*> CommonBuffers;// 删除功能不要了

	UINT64 Fence = 0;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc = nullptr;


private:
	ID3D12Device* Device = nullptr;


public:
	void Resize(INT PassCount, UINT ObjectCount)//, UINT MaterialCount
	{	
		//3个
		//PassCB = std::make_unique<DX12GPUCommonBuffer<PassConstants>>(Device, PassCount, true);
		//ObjectCB = std::make_unique<DX12GPUCommonBuffer<ObjectConstants>>(Device, ObjectCount, true);
		//MaterialCB = std::make_unique<UploadBuffer<MaterialConstants>>(Device, MaterialCount, true);
	}

	//std::unique_ptr<DX12GPUCommonBuffer<PassConstants>> PassCB = nullptr;
	//std::unique_ptr<DX12GPUCommonBuffer<ObjectConstants>> ObjectCB = nullptr;
	//std::unique_ptr<UploadBuffer<MaterialConstants>> MaterialCB = nullptr;
};


