#pragma once
#include "Thread.h"

class Task;
class Renderer;
class ForwardRenderer;
class RenderThread :public Thread
{
public:
	static void CreateRenderThread();//start 
	static void DestroyRenderThread();//stop

	static RenderThread* Get();

	virtual void Start() override;
	virtual void Stop() override;
	virtual void OnThreadStart() override;
	virtual void Run() override;
	virtual void OnThreadEnd() override;

	void TriggerRender();
	void AddTask(Task* RenderTask);
	void DoTasks();

	int GetRenderNum() { return RenderNum; }
	Renderer* GetRenderer() { return MRenderer; }


protected:
	std::mutex Rendermtx;
	std::condition_variable RenderCV;

	//TaskQueue
	struct RenderFrameTaskQueue
	{
		std::queue<Task*> TaskQueue;
	};
	RenderFrameTaskQueue MRenderFrame[2];

	UINT32 CurrentFrameIndex;
	UINT32 NextFrameIndex;
	//����0 �ʼAddTask��Next=0�������꣬��Next=1���ٽ���1��DoTaskִ��0�ģ����±�1��


private:
	RenderThread();
	virtual ~RenderThread();

	static RenderThread* MRenderThread;

	Renderer* MRenderer;

	int RenderNum = 0;
};