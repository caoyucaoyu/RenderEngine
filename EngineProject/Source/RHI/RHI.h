#pragma once

class RHI
{
public:
	RHI();
	virtual ~RHI();

	static RHI* Get();
	static void CreateRHI();
	static void DestroyRHI();

	static RHI* MRHI;

public:
	virtual void Init() = 0;
};