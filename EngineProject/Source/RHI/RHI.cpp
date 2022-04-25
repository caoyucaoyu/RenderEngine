#include "stdafx.h"
#include "RHI.h"
#include "DX12RHI.h"

RHI* RHI::MRHI = nullptr;

RHI* RHI::Get()
{
	return MRHI;
}

void RHI::CreateRHI()
{
	if (MRHI == nullptr)
	{
#if defined (RenderDeviceDX12)
		MRHI = new DX12RHI();

#elif defined (RenderDeviceVulkan)

#elif defined (RenderDeviceOpenGL)

#endif

	}
}

void RHI::DestroyRHI()
{
	if (MRHI != nullptr)
	{
		delete(MRHI);
		MRHI = nullptr;
	}
}

RHI::RHI()
{

}

RHI::~RHI()
{

}
