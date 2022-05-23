#include "stdafx.h"
#include "RHI.h"
#include "GPURenderTarget.h"

GPURenderTarget::GPURenderTarget()
{

}

GPURenderTarget::GPURenderTarget(std::string Name, UINT W, UINT H) :RenderTargetName(Name), Width(W), Hight(H)
{
	cout<<"GPURenderTarget: "<<Name<<" "<<W<<" "<<H<<endl<<endl;
}

GPURenderTarget::~GPURenderTarget()
{
	for (auto it : RTColorBuffers)
	{
		if (it != nullptr)
		{
			delete it;
			it = nullptr;
		}
	}
	if (RTDepthStencilBuffer!=nullptr)
	{
		delete RTDepthStencilBuffer;
		RTDepthStencilBuffer = nullptr;
	}
}

void GPURenderTarget::AddColorBuffer(UINT32 BufferNum)
{
	for (int i = 0; i < BufferNum; i++)
	{
		//GPURenderTargetBuffer* NewColorBuffer = new GPURenderTargetBuffer();//这里的创建要使用RHI 是因为需要创建DX的Buffer

		GPURenderTargetBuffer* NewColorBuffer = RHI::Get()->CreateRenderTargetBuffer(RTBufferType::Color, Width, Hight);
		RTColorBuffers.push_back(NewColorBuffer);
	}
}

void GPURenderTarget::AddDepthStencilBuffer()
{
	RTDepthStencilBuffer = RHI::Get()->CreateRenderTargetBuffer(RTBufferType::DepthStencil, Width, Hight);
}

GPURenderTargetBuffer* GPURenderTarget::GetColorBuffer(int Index)
{
	if (Index >= RTColorBuffers.size())
	{
		OutputDebugStringA("Cant Get ColorBuffer OverIndex\n");
		return nullptr;
	}
	return RTColorBuffers[Index];
}

GPURenderTargetBuffer* GPURenderTarget::GetDepthStencilBuffer()
{
	return RTDepthStencilBuffer;
}
