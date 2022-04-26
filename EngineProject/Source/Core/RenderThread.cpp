#include "stdafx.h"
#include "RHI/RHI.h"
#include "RenderThread.h"
#include "Renderer.h"

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

RenderThread* RenderThread::Get()
{
	return MRenderThread;
}

void RenderThread::OnThreadStart()
{
	MRenderer = new Renderer;
	MRenderer->Init();
}

void RenderThread::Run()
{
	if (IsRunning)
	{
		MRenderer->Render();
	}
}

void RenderThread::OnThreadEnd()
{

}

void RenderThread::AddTask(Task* RenderTask)
{

}

void RenderThread::DoTasks()
{

}
