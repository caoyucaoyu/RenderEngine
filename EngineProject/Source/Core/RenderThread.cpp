#include "stdafx.h"
#include "RHI/RHI.h"
#include "RenderThread.h"
#include "Renderer.h"
#include "ForwardRenderer.h"
#include "RenderScene.h"
#include "Task.h"


RenderThread* RenderThread::MRenderThread = nullptr;

RenderThread::RenderThread()
{

}

RenderThread::~RenderThread()
{

}

void RenderThread::CreateRenderThread()
{
	if (MRenderThread == nullptr)
	{
		MRenderThread = new RenderThread;
		//MRenderThread->Start();
	}
}

void RenderThread::DestroyRenderThread()
{
	if (MRenderThread != nullptr)
	{
		MRenderThread->Stop();
	}

}

void RenderThread::Start()
{
	Thread::Start();	
}

void RenderThread::Stop()
{
	if (pThread != nullptr)
	{
		{
			std::unique_lock<std::mutex> RenderLock(Rendermtx);
			++RenderNum;
			NextFrameIndex = (NextFrameIndex + 1) % 2;
			IsRunning = false;
			RenderCV.notify_one();
		}

		pThread->join();

		MRenderer->DestroyRenderer();

		if(pThread!=nullptr)
		{
			delete pThread;
			pThread=nullptr;
		}
	}
}

void RenderThread::OnThreadStart()
{
	std::unique_lock<std::mutex> RenderLock(Rendermtx);

	MRenderer = new ForwardRenderer();
	MRenderer->Init();
}

void RenderThread::Run()
{
	std::unique_lock<std::mutex> RenderLock(Rendermtx);
	while (RenderNum <= 0) 
	{
		RenderCV.wait(RenderLock);// Wait MainThread Give Task
	}
	DoTasks();
	RenderNum--;

	if (IsRunning)
	{
		MRenderer->RenderFrameBegin();
		MRenderer->Render();
		MRenderer->RenderFrameEnd();
	}
}

void RenderThread::OnThreadEnd()
{

}

void RenderThread::TriggerRender()//任务发完后调用 让渲染线程可以Run
{
	std::unique_lock<std::mutex> RenderLock(Rendermtx);
	++RenderNum;
	NextFrameIndex = (NextFrameIndex + 1) % 2;//使用另一个Task队列
	RenderCV.notify_one();
}

void RenderThread::AddTask(Task* RenderTask)
{
	if (IsRunning)
	{
		MRenderFrame[CurrentFrameIndex].TaskQueue.push(RenderTask);
	}
}

void RenderThread::DoTasks()
{
	Task* TaskE;
	while (!MRenderFrame[CurrentFrameIndex].TaskQueue.empty())
	{
		TaskE = MRenderFrame[CurrentFrameIndex].TaskQueue.front();
		TaskE->Execute();
		MRenderFrame[CurrentFrameIndex].TaskQueue.pop();
		if (TaskE != nullptr)
		{
			delete(TaskE);
			TaskE = nullptr;
		}
	}
	CurrentFrameIndex= (CurrentFrameIndex+1)%2;
}






RenderThread* RenderThread::Get()
{
	return MRenderThread;
}