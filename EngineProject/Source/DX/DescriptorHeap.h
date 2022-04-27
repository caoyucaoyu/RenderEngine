#pragma once

using Microsoft::WRL::ComPtr;

struct FAllocation
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE Handle;
	UINT32 Offset;
};

class DescriptorHeap
{
public:
	DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE HeapType, ComPtr<ID3D12Device> Device, UINT NumDescriptorPerHeap = 128);
	~DescriptorHeap();

	//��Heap����Count���ռ䡣����Handle��λ����š�
	FAllocation Allocate(UINT32 Size);
	void Deallocate(UINT32 Offset, UINT32 Size);
	void ClearHeap();

private:
	ID3D12DescriptorHeap* NeedANewHeap();

private:
	UINT32 NumDescriptorPerHeap;
	D3D12_DESCRIPTOR_HEAP_TYPE HeapType;
	UINT32 DescriptorSize;

	std::vector<ComPtr<ID3D12DescriptorHeap>> DescriptorHeapPool;

	ID3D12DescriptorHeap* CurrentHeap;
	CD3DX12_CPU_DESCRIPTOR_HANDLE CurrentHandle;
	UINT32 RemainFreeCount;//��ǰ��ʣ��ռ�����
	UINT32 CurrentOffset;//��ǰ��ƫ��

	//PositionCount -- Size
	std::unordered_map<UINT32, UINT32> FreeSizeMap;
	ComPtr<ID3D12Device> Device;
};