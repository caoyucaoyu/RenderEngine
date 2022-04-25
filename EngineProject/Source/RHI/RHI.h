#pragma once

class RHI
{
public:
	static RHI* Get();
	static void CreateRHI();
	static void DestroyRHI();
	static RHI* MRHI;
	RHI();
	virtual ~RHI();
};