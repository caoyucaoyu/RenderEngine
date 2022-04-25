#pragma once

class Thread
{
public:
	Thread();
	virtual ~Thread();

	virtual void Start();
	virtual void Stop();

	virtual void OnThreadStart() {};
	virtual void Run() = 0;
	virtual void OnThreadEnd() {};

	virtual bool GetIsThreadRunning()
	{
		return IsRunning;
	}

	static void* ThreadExecute(void* param);

protected:
	bool IsRunning;
	std::thread* pThread;

};