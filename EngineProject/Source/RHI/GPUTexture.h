#pragma once

class GPUTexture
{
public:
	GPUTexture();
	virtual ~GPUTexture();

	void SetHandleOffset(UINT32 offset) { HandleOffset = offset; }
	UINT32 GetHandleOffset() { return HandleOffset; }

public:
	std::string Name;
	std::wstring Filename;
protected:
	UINT32 HandleOffset;
};