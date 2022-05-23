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
		//out
		if (Type == RTBufferType::Color)
			cout << "Set Color RTBuffer Offset: " << Offset << endl << endl;
		else if (Type == RTBufferType::DepthStencil)
			cout << "Set Depth RTBuffer Offset: " << Offset << endl << endl;

		HandleOffset = Offset;
	};
	virtual UINT32 GetHandleOffset() { return HandleOffset; }


	UINT32 Width;
	UINT32 Hight;
	RTBufferType Type;

private:
	UINT32 HandleOffset;

};



