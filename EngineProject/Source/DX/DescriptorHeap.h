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

	ID3D12DescriptorHeap* GetCurrentHeap();

private:
	ID3D12DescriptorHeap* NeedANewHeap();

private:
	UINT32 NumDescriptorPerHeap;
	D3D12_DESCRIPTOR_HEAP_TYPE HeapType;
	UINT32 DescriptorSize;//��ǰ����Descriptor��Size

	//D3D12_DESCRIPTOR_HEAP_TYPE_RTV
	//D3D12_DESCRIPTOR_HEAP_TYPE_DSV
	//D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV

	std::vector<ComPtr<ID3D12DescriptorHeap>> DescriptorHeapPool;

	ID3D12DescriptorHeap* CurrentHeap;
	CD3DX12_CPU_DESCRIPTOR_HANDLE CurrentHandle;

	UINT32 RemainFreeCount;//��ǰ��ʣ��ռ�����
	UINT32 CurrentOffset;//��ǰ��ƫ�� ��λ�����

	//PositionCount -- Size
	std::unordered_map<UINT32, UINT32> FreeSizeMap;//��¼ɾ��������м�ճ�����λ��  λ�����--�ճ����ĸ���
	ComPtr<ID3D12Device> Device;
};