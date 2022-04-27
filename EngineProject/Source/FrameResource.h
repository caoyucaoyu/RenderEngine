#pragma once
#include "stdafx.h"
#include "D3DUtil.h"
#include "UploadBuffer.h"

struct ObjectConstants
{	
	//glm::mat4 World_M = MathHelper::GIdentity4x4();
	glm::mat4 Location_M = MathHelper::GIdentity4x4();
	glm::mat4 Rotation_M = MathHelper::GIdentity4x4();
	glm::mat4 Scale3D_M = MathHelper::GIdentity4x4();
};

struct PassConstants
{
	glm::mat4 ViewProj_M= MathHelper::GIdentity4x4();
	float Time = 0;
};

struct MaterialConstants
{
	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
	float Roughness = 0.25f;
	DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();
};


struct FrameResource
{
public:
	//VP Time only 1,
	FrameResource(ID3D12Device* Device, UINT PassCount, UINT ObjectCount, UINT MaterialCount)
	{
		ThrowIfFailed(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(CmdListAlloc.GetAddressOf())));

		PassCB = std::make_unique<UploadBuffer<PassConstants>>(Device, PassCount, true);
		ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(Device, ObjectCount, true);
		//MaterialCB = std::make_unique<UploadBuffer<MaterialConstants>>(Device, MaterialCount, true);
	}

	FrameResource(const FrameResource& rhs) = delete;
	FrameResource& operator=(const FrameResource& rhs) = delete;
	~FrameResource(){};

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc;

	std::unique_ptr<UploadBuffer<PassConstants>> PassCB = nullptr;
	std::unique_ptr<UploadBuffer<ObjectConstants>> ObjectCB = nullptr;
	//std::unique_ptr<UploadBuffer<MaterialConstants>> MaterialCB = nullptr;

	UINT64 Fence = 0;
};


