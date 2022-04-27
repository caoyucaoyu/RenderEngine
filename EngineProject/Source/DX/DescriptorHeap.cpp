#include "stdafx.h"
#include "DescriptorHeap.h"

DescriptorHeap::DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE HeapType, ComPtr<ID3D12Device> Device, UINT NumDescriptorPerHeap)
{
	this->HeapType = HeapType;
	this->Device = Device;
	this->NumDescriptorPerHeap = NumDescriptorPerHeap;
	DescriptorSize = Device->GetDescriptorHandleIncrementSize(HeapType);
}

DescriptorHeap::~DescriptorHeap()
{
	DescriptorHeapPool.clear();
}

FAllocation DescriptorHeap::Allocate(UINT32 Count)
{
	//使用中间空出来的位置
	if (FreeSizeMap.size() > 0)
	{
		for (auto MapIt : FreeSizeMap)
		{
			if (MapIt.second == Count)//大小匹配
			{
				auto Handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(CurrentHeap->GetCPUDescriptorHandleForHeapStart());

				//Handle.ptr += size_t(MapIt.first) * DescriptorSize;
				Handle.Offset(MapIt.first, DescriptorSize);

				FreeSizeMap.erase(MapIt.first);

				return { Handle, MapIt.first };
			}
		}
	}

	if (CurrentHeap == nullptr || RemainFreeCount < Count)
	{
		CurrentHeap = NeedANewHeap();
		CurrentHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(CurrentHeap->GetCPUDescriptorHandleForHeapStart());
		RemainFreeCount = NumDescriptorPerHeap;
	}

	auto Handle = CurrentHandle;
	UINT32 Offset = CurrentOffset;

	//CurrentHandle.ptr += size_t(Count) * DescriptorSize;
	CurrentHandle.Offset(Count, DescriptorSize);

	RemainFreeCount -= Count;
	CurrentOffset += Count;

	return { Handle, Offset };
}

void DescriptorHeap::Deallocate(UINT32 Offset, UINT32 Count)
{
	FreeSizeMap.insert({ Offset, Count });
}

ID3D12DescriptorHeap* DescriptorHeap::NeedANewHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC Desc;
	Desc.NumDescriptors = NumDescriptorPerHeap;
	Desc.Type = HeapType;

	//????
	Desc.Flags = HeapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	//????
	Desc.NodeMask = 0;

	ComPtr<ID3D12DescriptorHeap> Heap;
	Device->CreateDescriptorHeap(&Desc, IID_PPV_ARGS(Heap.GetAddressOf()));
	DescriptorHeapPool.emplace_back(Heap);
	return Heap.Get();
}

void DescriptorHeap::ClearHeap()
{
	DescriptorHeapPool.clear();
	CurrentHeap = nullptr;
}


