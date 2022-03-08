#pragma once
#include "AppWin32.h"
#include "MathHelper.h"
#include "UploadBuffer.h"
#include "FrameResource.h"
#include "Camera.h"
#include "Mesh.h"

//using DirectX::XMConvertToRadians;
//const int FrameResourcesCount = 3;

//class TAppWin32 : public TAppWin
//{
//public:
	//TAppWin32();
	//~TAppWin32();
	//virtual bool CreateAppWindow(HINSTANCE hInstance);
	//virtual bool Init(HINSTANCE hInstance=nullptr) override;
	//virtual void Draw()override;
	//virtual void Update(const GameTimer& Gt)override;
	//virtual void OnResize()override;

//protected:
	//void BuildFrameResource();//////
	//void BuildDescriptorHeaps();///////
	//void BuildConstantBuffers();
	//void BuildRootSignature();
	//void BuildShadersAndInputLayout();
	//void BuildGeometry();
	//void BuildPSO();

//private:
	//virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	//virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	//virtual void OnMouseMove(WPARAM btnState, int x, int y) override;

	//virtual void OnKeyboardInput(const GameTimer& gt);

//private:
//	ComPtr<ID3D12DescriptorHeap> CbvHeap;
//	ComPtr<ID3D12RootSignature> RootSignature = nullptr;
//
//	ComPtr<ID3DBlob> MvsByteCode[2];
//	ComPtr<ID3DBlob> MpsByteCode[2];
//	std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;
//
//	std::vector<ComPtr<ID3D12PipelineState>> PSOs;
	//ComPtr<ID3D12PipelineState> PSO = nullptr;

	//Camera MainCamera;
	//POINT LastMousePos;

	//std::vector<std::unique_ptr<FrameResource>> FrameResources;
	//FrameResource* CurrFrameResource = nullptr;
	//int CurrFrameResourceIndex = 0;
	//int DescriptorsNum=0;

//private:
	////Util
	//ComPtr<ID3D12Resource> CreateDefaultBuffer(ID3D12Device* Device, ID3D12GraphicsCommandList* CmdList, const void* InitData, UINT64 ByteSize, ComPtr<ID3D12Resource>& UploadBuffer);
	////Util
	//ComPtr<ID3DBlob> CompileShader(const std::wstring& Filename, const D3D_SHADER_MACRO* Defines, const std::string& EntryPoint, const std::string& Target);

	//std::vector<std::shared_ptr<MeshGeometry>> DrawList;
	//int DrawCount=0;
//};
