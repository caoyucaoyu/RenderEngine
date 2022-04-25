#pragma once
#include <functional>

class Task
{
public:
	Task(const std::function<void()>& Func);
	virtual ~Task();

	virtual void Execute();

protected:
	//typedef std::function<void()> TaskFunc;
	//TaskFunc ExecuteFunc;

	std::function<void()> ExecuteFunc;

};


void MPrint(int a)
{

}

Task* task = new Task( [=]() {MPrint(1);} );
