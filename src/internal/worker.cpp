/*
 * worker.cpp
 *
 *  Created on: Apr 27, 2016
 *      Author: janne
 */

#include "internal/worker.h"
#include <iostream>

Worker::Worker(JobQueue* _queuePtr) :
noOfJobsExecuted(0),
queuePtr(_queuePtr),
isIdling(false)
{

}

void Worker::Notify()
{
	std::unique_lock<std::mutex> executionLock(executionNotificationMutex);
	executionNotification.notify_one();
}

void Worker::run()
{
	while(running)
	{
		isIdling = false;

		std::shared_ptr<JobBase> jobPtr = queuePtr->GetNextJob();
		while(running && jobPtr != nullptr)
		{
			jobPtr->Execute();
			noOfJobsExecuted++;
			jobPtr = nullptr;
			jobPtr = queuePtr->GetNextJob();
		}

		isIdling = true;
		if(!running)
		{
			return;
		}
		std::unique_lock<std::mutex> executionLock(executionNotificationMutex);
		executionNotification.wait(executionLock);
	}
}

bool Worker::IsIdling()
{
	return isIdling;
}

uint32_t Worker::GetNoOfJobsExecuted()
{
	return noOfJobsExecuted;
}

void Worker::Stop()
{
	running = false;

	while(!isIdling)
	{
		/*
		 * We need to wait for the worker to
		 * finish its jobs before we can tell
		 * it to exit
		 */
		Notify();
	}

	Notify();

	if(this->executorThread.joinable())
	{
		this->executorThread.join();
	}
}
