#pragma once


class GPUCommonBuffer
{
public:
	GPUCommonBuffer();
	virtual ~GPUCommonBuffer();

	virtual void CopyData(int elementIndex, std::shared_ptr<void> data) = 0;

public:
	virtual void SetHandleOffset(UINT32 Offset);
	virtual UINT32 GetHandleOffset(){return HandleOffset;};

protected:
	UINT32 HandleOffset;

};

