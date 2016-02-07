#include "./threadobject.h"

ThreadObject::ThreadObject() :
running(false)
{
	
}

ThreadObject::~ThreadObject()
{
	stop();

	if(this->executorThread.joinable())
	{
		this->executorThread.join();
	}
}

void ThreadObject::start()
{
	running = true;
	this->executorThread = std::thread(&ThreadObject::run, this);
}

void ThreadObject::stop()
{
	running = false;
}
