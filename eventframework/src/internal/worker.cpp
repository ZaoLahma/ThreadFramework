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
executionLock(executionNotificationMutex),
isIdling(false)
{

}

void Worker::Notify()
{
	executionNotification.notify_one();
}

void Worker::run()
{
	while(running)
	{
		isIdling = false;

		JobBase* jobPtr = queuePtr->GetNextJob();
		while(running && jobPtr != nullptr)
		{
			jobPtr->Execute();
			noOfJobsExecuted++;
			delete jobPtr;
			jobPtr = queuePtr->GetNextJob();
		}

		isIdling = true;
		if(!running)
		{
			return;
		}
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
