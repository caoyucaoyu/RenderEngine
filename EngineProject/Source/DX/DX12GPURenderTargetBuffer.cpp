#include "stdafx.h"
#include "DX12GPURenderTargetBuffer.h"


DX12GPURenderTargetBuffer::DX12GPURenderTargetBuffer()
{

}

DX12GPURenderTargetBuffer::DX12GPURenderTargetBuffer(RTBufferType Type, UINT W, UINT H)
{
	this->Type = Type;
	Width = W;
	Hight = H;
	if (Type == RTBufferType::DepthStencil)
		InitDepthStencilType();
	else if (Type == RTBufferType::Color)
		InitColorType();
}

void DX12GPURenderTargetBuffer::InitDepthStencilType()
{
	ResourceState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ; 
	//D3D12_RESOURCE_STATE_DEPTH_WRITE
	//D3D12_RESOURCE_STATE_GENERIC_READ
	//D3D12_RESOURCE_STATE_DEPTH_WRITE

	Desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	Desc.Alignment = 0;
	Desc.Width = Width;
	Desc.Height = Hight;
	Desc.DepthOrArraySize = 1;
	Desc.MipLevels = 1;
	Desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//后边可能会使用不同的格式类型 这里先写死 DXGI_FORMAT_D24_UNORM_S8_UINT
	Desc.SampleDesc.Count = 1;
	Desc.SampleDesc.Quality = 0;
	Desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	Desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	OptClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//DXGI_FORMAT_R24G8_TYPELESS;//DXGI_FORMAT DepthStencilFormat  DXGI_FORMAT_D24_UNORM_S8_UINT
	OptClear.DepthStencil.Depth = 1.0f;
	OptClear.DepthStencil.Stencil = 0;
	/*D3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&DepthStencilDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&OptClear,
		IID_PPV_ARGS(DepthStencilBuffer.GetAddressOf()));*/

}

void DX12GPURenderTargetBuffer::InitColorType()
{
	ResourceState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;

	Desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	Desc.Alignment = 0;
	Desc.Width = Width;
	Desc.Height = Hight;
	Desc.DepthOrArraySize = 1;
	Desc.MipLevels = 1;
	Desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//DXGI_FORMAT_R8G8B8A8_UNORM
	Desc.SampleDesc.Count = 1;
	Desc.SampleDesc.Quality = 0;
	Desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	Desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	OptClear.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//DXGI_FORMAT DepthStencilFormat
	//DXGI_FORMAT_D24_UNORM_S8_UINT
	OptClear.Color[0] = 0;
	OptClear.Color[1] = 0;
	OptClear.Color[2] = 0;
	OptClear.Color[3] = 0;
}

DX12GPURenderTargetBuffer::~DX12GPURenderTargetBuffer()
{

}

void DX12GPURenderTargetBuffer::CreateResource(ID3D12Device* Device)
{
	Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&Desc,
		ResourceState,
		&OptClear,
		IID_PPV_ARGS(RTResource.GetAddressOf()));
	//cout<<"Create RT Resource"<<endl;
}

void DX12GPURenderTargetBuffer::CreateView(ID3D12Device* Device, FAllocation Allocation)
{	
	if (Type == RTBufferType::Color)
	{
		D3D12_RENDER_TARGET_VIEW_DESC RtvDesc = {};
		RtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		RtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		RtvDesc.Texture2D.PlaneSlice = 0;
		Device->CreateRenderTargetView(RTResource.Get(), &RtvDesc, Allocation.Handle);
	}
	else if (Type == RTBufferType::DepthStencil)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC DsvDesc;
		DsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		DsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		DsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//DXGI_FORMAT_D24_UNORM_S8_UINT
		DsvDesc.Texture2D.MipSlice = 0;
		Device->CreateDepthStencilView(RTResource.Get(), &DsvDesc, Allocation.Handle);
	}
}

void DX12GPURenderTargetBuffer::ResetResource()
{
	RTResource.Reset();
}

void DX12GPURenderTargetBuffer::SetResourceName(std::string ResourceName)
{
	RTResource->SetName((LPCWSTR)ResourceName.c_str());
}
