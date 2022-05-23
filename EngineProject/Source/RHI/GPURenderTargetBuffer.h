#pragma once

enum class RTBufferType
{
	Color = 0,
	DepthStencil = 1,
};

class GPURenderTargetBuffer
{
public:
	GPURenderTargetBuffer();
	virtual ~GPURenderTargetBuffer();
	virtual void ResetResource();

	virtual void SetResourceName(std::string ResourceName);

	virtual void SetHandleOffset(UINT32 Offset) 
	{ 
		HandleOffset = Offset;
	};
	virtual UINT32 GetHandleOffset() { return HandleOffset; }


	UINT32 Width;
	UINT32 Hight;
	RTBufferType Type;

private:
	UINT32 HandleOffset;

};



