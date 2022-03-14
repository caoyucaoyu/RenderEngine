#pragma once
#include "stdafx.h"
#include "D3DUtil.h"
#include "UploadBuffer.h"

struct ObjectConstants
{
	DirectX::XMFLOAT4X4 Location_M = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 Rotation_M = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 Scale3D_M = MathHelper::Identity4x4();
	//glm::mat4 World_M = MathHelper::GIdentity4x4();
};

struct PassConstants
{
	DirectX::XMFLOAT4X4 ViewProj_M = MathHelper::Identity4x4();
	float Time = 0;
	//glm::mat4 ViewProj_M = MathHelper::GIdentity4x4();
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
	FrameResource(ID3D12Device* Device, UINT PassCount, UINT ObjectCount, UINT materialCount);
	FrameResource(const FrameResource& rhs) = delete;
	FrameResource& operator=(const FrameResource& rhs) = delete;
	~FrameResource();

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc;

	std::unique_ptr<UploadBuffer<PassConstants>> PassCB = nullptr;
	std::unique_ptr<UploadBuffer<ObjectConstants>> ObjectCB = nullptr;
	std::unique_ptr<UploadBuffer<MaterialConstants>> MaterialCB = nullptr;

	UINT64 Fence = 0;
};


