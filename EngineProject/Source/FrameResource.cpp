#include "stdafx.h"
#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device* Device, UINT PassCount, UINT ObjectCount, UINT materialCount)
{
	ThrowIfFailed(
		Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(CmdListAlloc.GetAddressOf())));

	PassCB = std::make_unique<UploadBuffer<PassConstants>>(Device, PassCount, true);
	ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(Device, ObjectCount, true);
	MaterialCB = std::make_unique<UploadBuffer<MaterialConstants>>(Device, materialCount, true);
}

FrameResource::~FrameResource()
{

}
