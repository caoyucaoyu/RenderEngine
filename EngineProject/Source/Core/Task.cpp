#include "stdafx.h"
#include "Task.h"

Task::Task(const std::function<void()>& Func)
{

}

Task::~Task()
{

}

void Task::Execute()
{
	if (ExecuteFunc)
	{
		ExecuteFunc();
	}
}
