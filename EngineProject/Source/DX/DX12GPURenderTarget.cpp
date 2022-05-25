#include "stdafx.h"
#include "DX12GPURenderTarget.h"

DX12GPURenderTarget::DX12GPURenderTarget(std::string Name, UINT W, UINT H) :GPURenderTarget(Name, W, H)
{
	Viewport.TopLeftX = 0.0f;
	Viewport.TopLeftY = 0.0f;
	Viewport.Width = static_cast<float>(W);
	Viewport.Height = static_cast<float>(H);
	Viewport.MinDepth = 0.0f;
	Viewport.MaxDepth = 1.0f;
	ScissorRect = { 0,0,(int)W,(int)H };
	//cout<<"Init Viewport ScissorRect\n";
}
