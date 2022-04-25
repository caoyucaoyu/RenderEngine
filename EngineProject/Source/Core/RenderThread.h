#pragma once
#include "Thread.h"

class RenderThread :public Thread
{
public:
	RenderThread();
	~RenderThread();

protected:
	static RenderThread MRenderThread;

};