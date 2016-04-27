#include "../inc/internal/threadobject.h"

ThreadObject::ThreadObject() :
running(false)
{
	
}

ThreadObject::~ThreadObject()
{
	Stop();
}

void ThreadObject::Start()
{
	running = true;
	this->executorThread = std::thread(&ThreadObject::run, this);
}

void ThreadObject::Stop()
{
	running = false;

	if(this->executorThread.joinable())
	{
		this->executorThread.join();
	}
}
