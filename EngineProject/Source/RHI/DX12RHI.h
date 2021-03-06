#pragma once
#include "RHI.h"

//using Microsoft::WRL::ComPtr;

class Window;
class DescriptorHeap;
class FrameResource;

const int FrameResourcesCount = 4;

const int ParameterNum = 6;

class DX12RHI : public RHI
{
public:
	DX12RHI();
	~DX12RHI();

public:
	virtual void Init()override;

	virtual void BeginFrame() override;
	virtual void EndFrame() override;

	virtual void CreateFrameResource() override;
	virtual void UpdateFrameResource() override;
	virtual int GetCurFrameResourceIdx() override;
	virtual int GetFrameResourceCount() override;

	virtual void FlushCommandQueue()override;
	virtual void DrawIndexedInstanced(UINT DrawIndexCount) override;
	virtual void ResizeWindow(UINT32 Width, UINT32 Height) override;

	virtual void SetBackBufferBegin() override;//呈现转渲染
	virtual void SetBackBufferEnd() override;
	virtual void SetRenderTargetBegin(GPURenderTarget* RenderTarget) override;//视口裁剪、清除与指定
	virtual void SetRenderTargetBufferBegin(GPURenderTarget* RenderTarget) override;
	virtual void SetRenderTargetBufferEnd(GPURenderTarget* RenderTarget) override;

	virtual void SetGraphicsPipeline(Pipeline* NPipeline, int TemporaryType) override;
	virtual void SetRenderResourceTable(int Nu, UINT32 HeapOffset) override;//............
	virtual void IASetMeshBuffer(GPUMeshBuffer* GPUMeshbuffer) override;


	virtual GPUMeshBuffer* CreateMeshBuffer() override;
	virtual void UpdateMeshBuffer(GPUMeshBuffer* GpuMeshBuffer) override;

	virtual GPUCommonBuffer* CreateCommonBuffer(UINT ElementCount, bool IsConstantBuffer, UINT ElementByteSize) override;
	virtual void UpdateCommonBuffer(GPUCommonBuffer* GpuCommonBuffer, std::shared_ptr<void> Data, int elementIndex) override;
	virtual void AddCommonBuffer(int FrameSourceIndex,std::string, GPUCommonBuffer* GpuCommonBuffer) override;

	virtual GPUTexture* CreateTexture(std::string TextureName, std::wstring FileName) override;
	virtual GPUTexture* CreateTexture(std::string TextureName, GPURenderTargetBuffer* RTBuffer) override;

	virtual GPURenderTarget* CreateRenderTarget(std::string RTName, UINT W, UINT H) override;
	virtual GPURenderTargetBuffer* CreateRenderTargetBuffer(RTBufferType Type, UINT W, UINT H) override;

	virtual void RootSignatureAndPSO() override;

public:
	void ExecuteCommandList();//将待执行的命令列表加入GPU的命令队列
	void ResetCommandList(ComPtr<ID3D12CommandAllocator> CommandListAllocator);

private:
	void PrepareBufferHeap();


private:
	void CreateDevice();
	void CreateFence();
	void GetDescriptorSize();
	void SetMSAA();
	void CreateCommandObject();
	void CreateSwapChain();

	void CreateRtvAndDsvDescriptorHeaps();
	void ResetBuffers();
	void CreateRTV();
	void CreateDSV();
	void CreateViewPortAndScissorRect();
	void CreateBackRenderTerget();

	void CreatCbvSrvUavHeap();
	
	//只读
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;
	ComPtr<ID3D12Resource> GetCurrentBackBuffer() const;

private:
	Window* Wd;
	ComPtr<IDXGIFactory> DxgiFactory;
	ComPtr<ID3D12Device> D3dDevice;
	ComPtr<ID3D12Fence> Fence;
	ComPtr<ID3D12CommandQueue> CommandQueue;
	ComPtr<ID3D12GraphicsCommandList> CommandList;
	ComPtr<ID3D12CommandAllocator> CommandListAllocator;
	ComPtr<IDXGISwapChain> SwapChain;
	UINT RtvDescriptorSize;
	UINT DsvDescriptorSize;
	UINT CbvSrvUavDescriptorSize;
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS MsaaQualityLevels;
	UINT MsaaQuality4X;
	HWND HMainWnd;

	static const int SwapChainBufferCount = 2;

	ComPtr<ID3D12Resource> SwapChainBuffers[2];
	ComPtr<ID3D12Resource> DepthStencilBuffer;

	std::unique_ptr<DescriptorHeap> RtvHeap;
	std::unique_ptr<DescriptorHeap> DsvHeap;
	std::unique_ptr<DescriptorHeap> CbvSrvUavHeap;

	UINT64 CurrentFence = 0;
	UINT64 CurrentBackBufferIndex = 0;

	D3D12_VIEWPORT ScreenViewport;
	D3D12_RECT ScissorRect;

	std::vector<std::unique_ptr<FrameResource>> FrameResources;
	FrameResource* CurrFrameResource = nullptr;
	int CurrFrameResourceIndex = 0;

private:
	ComPtr<ID3D12RootSignature> RootSignature;
	void BuildRootSignature();

	ComPtr<ID3DBlob> MvsByteCode[4];
	ComPtr<ID3DBlob> MpsByteCode[4];
	std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;
	void BuildShadersAndInputLayout();

	std::vector<ComPtr<ID3D12PipelineState>> PSOs;
	void BuildPSO();

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> GetStaticSamplers();
	
};
