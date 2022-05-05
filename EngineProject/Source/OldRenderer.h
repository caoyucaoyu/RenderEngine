#pragma once
#include "DX\FrameResource.h"
#include "GPUMeshBuffer.h"
#include "Material.h"
#include "Texture.h"
#include "Camera.h"
#include "Scene.h"
#include <dxgi.h>
#include "DX12GPUMeshBuffer.h"

using Microsoft::WRL::ComPtr;
using DirectX::XMConvertToRadians;
const int OldFrameResourcesCount = 3;

class OldRenderer
{
public:
	OldRenderer();
	~OldRenderer(){};

	void Render();

	bool Init();//
	bool InitDirect3D();//...
	bool InitDraw();

	void RendererUpdate(const GameTimer* Gt);
	void Draw();
	void RendererReset();

	void CreateDevice();//
	void CreateFence();//
	void GetDescriptorSize();//
	void SetMSAA();//
	void CreateCommandObject();//
	void CreateSwapChain();//

	void CreateDescriptorHeap();//
	void CreateRTV();//
	void CreateDSV();//

	void FlushCommandQueue();//

	void CreateViewPortAndScissorRect();

	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView();
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView();

	void BuildGeometry();//Abandon

	void BuildRenderData();//New
	void BuildTextures();

	void BuildFrameResource();
	void BuildDescriptorHeaps();
	void BuildConstantBuffers();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildPSO();

protected:
	ComPtr<IDXGIFactory> DxgiFactory;//
	ComPtr<ID3D12Device> D3dDevice;//
	ComPtr<ID3D12Fence> Fence;//
	ComPtr<ID3D12CommandQueue> CommandQueue;//
	ComPtr<ID3D12GraphicsCommandList> CommandList;//
	ComPtr<ID3D12CommandAllocator> CommandListAlloc;//
	ComPtr<IDXGISwapChain> SwapChain;//
	static const int SwapChainBufferCount = 2;//
	ComPtr<ID3D12Resource> SwapChainBuffers[2];//SwapChainBufferCount
	UINT RtvDescriptorSize;//
	UINT DsvDescriptorSize;//
	UINT CbvSrvUavDescriptorSize;//
	int ClientWidth = 1280;//
	int ClientHight = 720;
	HWND HMainWnd;///
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS MsaaQualityLevels;//
	//
	ComPtr<ID3D12DescriptorHeap> DsvHeap;//
	ComPtr<ID3D12DescriptorHeap> RtvHeap;//


	ComPtr<ID3D12Resource> DepthStencilBuffer;

	D3D12_VIEWPORT ScreenViewport;///
	D3D12_RECT ScissorRect;///

	UINT64 CurrentFence = 0;///
	int CurrBackBuffer = 0;///

	std::vector<std::unique_ptr<FrameResource>> FrameResources;//////

	int DescriptorsNum = 0;

	ComPtr<ID3D12DescriptorHeap> CbvHeap;

	ComPtr<ID3D12RootSignature> RootSignature;
	ComPtr<ID3DBlob> MvsByteCode[2];
	ComPtr<ID3DBlob> MpsByteCode[2];
	std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;
	std::vector<ComPtr<ID3D12PipelineState>> PSOs;


	FrameResource* CurrFrameResource = nullptr;/////
	int CurrFrameResourceIndex = 0;////


private:
	int DrawCount = 0;
	int MaterialCount = 0;
	int TextureCount = 0;

	std::vector<MeshActor> DrawList;// RenderScene //////////////////////////////////////////////

	std::unordered_map<std::string, DX12GPUMeshBuffer> DXMeshs;//////////////////
	//‘›«“≤ª”√
	std::unordered_map<std::string, DXTexture> DXTextures;
	std::unordered_map<std::string, Material> DXMaterials;

public:
	DX12GPUMeshBuffer FindRMesh(std::string MeshName);

	bool CanFindRMesh(std::string MeshName);

};

