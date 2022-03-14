#pragma once
#include <string>
#include <wrl/module.h>
#include <d3d12.h>

struct Texture
{
	std::string Name;
	std::wstring Filename;
};

struct DXTexture
{
	DXTexture() {};
	DXTexture(Texture tex)
	{
		Name = tex.Name;
		Filename = tex.Filename;
	};
	std::string Name;
	std::wstring Filename;
	Microsoft::WRL::ComPtr<ID3D12Resource> Resource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap = nullptr;
};