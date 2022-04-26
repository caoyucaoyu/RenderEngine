#pragma once
#include "Thread.h"

class Task;
class Renderer;
class RenderThread :public Thread
{
public:
	static void CreateRenderThread();//start 
	static void DestroyRenderThread();//stop

	static RenderThread* Get();

	virtual void OnThreadStart() override;
	virtual void Run() override;
	virtual void OnThreadEnd() override;

	void AddTask(Task* RenderTask);
	void DoTasks();

	

private:
	RenderThread();
	~RenderThread();

	static RenderThread* MRenderThread;
	Renderer* MRenderer;

};