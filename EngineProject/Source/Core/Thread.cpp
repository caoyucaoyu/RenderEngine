#include "stdafx.h"
#include "Thread.h"

Thread::Thread() : IsRunning(false), pThread(nullptr)
{

}

Thread::~Thread()
{
	Stop();
}

void Thread::Start()
{
	if (IsRunning)
	{
		return;
	}

	IsRunning = true;

	pThread = new std::thread(Thread::ThreadExecute, this);
}

void Thread::Stop()
{
	if (pThread != nullptr)
	{
		IsRunning = false;
		pThread->join();

		if (pThread != nullptr)
		{
			delete(pThread);
			pThread = nullptr;
		}
	}
}

void* Thread::ThreadExecute(void* param)
{
	Thread* t = (Thread*)param;

	t->OnThreadStart();

	while (t->IsRunning)
	{
		t->Run();
	}

	t->OnThreadEnd();

	return 0;
}

