/*
 * jobdispatcher.cpp
 *
 *  Created on: Feb 3, 2016
 *      Author: janne
 */

#include "jobdispatcher.h"

JobDispatcher* JobDispatcher::instance = nullptr;
std::mutex JobDispatcher::instanceCreationMutex;

JobDispatcher::JobDispatcher() :
currentWorkerIndex(0)
{
	uint32_t noOfCores = std::thread::hardware_concurrency();

	for(uint32_t i = 0; i < noOfCores; ++i)
	{
		Worker* worker = new Worker(&jobQueue);
		worker->start();
		workers.push_back(worker);
	}
}

JobDispatcher::~JobDispatcher()
{
	WorkerPtrVector::iterator workerIter = workers.begin();

	for( ; workerIter != workers.end(); ++workerIter)
	{
		(*workerIter)->stop();
		(*workerIter)->Notify();
	}
}

JobDispatcher* JobDispatcher::GetApi()
{
	if(nullptr == instance)
	{
		instanceCreationMutex.lock();
		if(nullptr == instance)
		{
			instance = new JobDispatcher();
		}
		instanceCreationMutex.unlock();
	}

	return instance;
}

void JobDispatcher::DropInstance()
{
	delete instance;
	instance = nullptr;
}

void JobDispatcher::ExecuteJob(JobBase* jobPtr)
{
	jobQueue.QueueJob(jobPtr);

	workers[currentWorkerIndex]->Notify();

	currentWorkerIndex++;

	if(currentWorkerIndex == workers.size())
	{
		currentWorkerIndex = 0;
	}
}

void JobDispatcher::SubscribeToEvent(uint32_t eventNo, EventListenerBase* eventListenerPtr)
{
	std::lock_guard<std::mutex> subscribersLock(eventListenersAccessMutex);

	SubscriberEventMap::iterator eventIter = eventListeners.find(eventNo);

	if(eventListeners.end() == eventIter)
	{
		EventListenerBasePtrVector newElement;
		eventListeners[eventNo] = newElement;
		eventIter = eventListeners.find(eventNo);
	}

	eventIter->second.push_back(eventListenerPtr);
}

void JobDispatcher::UnsubscribeToEvent(uint32_t eventNo, EventListenerBase* eventListenerPtr)
{
	std::lock_guard<std::mutex> subscribersLock(eventListenersAccessMutex);

	SubscriberEventMap::iterator eventIter = eventListeners.find(eventNo);

	if(eventListeners.end() != eventIter)
	{
		EventListenerBasePtrVector::iterator eventListenersIter = eventIter->second.begin();

		for( ; eventListenersIter != eventIter->second.end(); ++eventListenersIter)
		{
			if(*eventListenersIter == eventListenerPtr)
			{
				eventIter->second.erase(eventListenersIter);
				break;
			}
		}
	}
}

void JobDispatcher::RaiseEvent(uint32_t eventNo)
{
	std::lock_guard<std::mutex> subscribersLock(eventListenersAccessMutex);

	SubscriberEventMap::iterator eventIter = eventListeners.find(eventNo);

	if(eventListeners.end() != eventIter)
	{
		EventListenerBasePtrVector::iterator eventListenerIter = eventIter->second.begin();

		for( ; eventListenerIter != eventIter->second.end(); ++eventListenerIter)
		{
			JobDispatcher::EventJob* eventJob = new JobDispatcher::EventJob(*eventListenerIter);

			JobDispatcher::GetApi()->ExecuteJob(eventJob);
		}
	}
}

//JobQueue
JobDispatcher::JobQueue::JobQueue() :
indexToExecute(0),
currentQueue(&queue_1),
queueToExecute(&queue_2)
{

}

JobDispatcher::JobQueue::~JobQueue()
{
	JobBasePtrVectorT::iterator jobIter = queue_1.begin();

	for( ; jobIter != queue_1.end(); ++jobIter)
	{
		delete *jobIter;
		*jobIter = nullptr;
	}

	queue_1.clear();

	jobIter = queue_2.begin();

	for( ; jobIter != queue_2.end(); ++jobIter)
	{
		delete *jobIter;
		*jobIter = nullptr;
	}

	queue_2.clear();
}

 void JobDispatcher::JobQueue::QueueJob(JobBase* jobPtr)
 {
	 std::lock_guard<std::mutex> lockGuard(queueAccessMutex);
	 currentQueue->push_back(jobPtr);
 }

 JobBase* JobDispatcher::JobQueue::GetNextJob()
 {
	 std::lock_guard<std::mutex> getJobLock(getJobMutex);

	 if(indexToExecute == (*queueToExecute).size())
	 {
		 queueAccessMutex.lock();

		 if(indexToExecute == (*queueToExecute).size())
		 {
			 JobBasePtrVectorT* _currentQueue = currentQueue;

			 queueToExecute->clear();
			 currentQueue = queueToExecute;

			 queueToExecute = _currentQueue;

			 indexToExecute = 0;
		 }

		 queueAccessMutex.unlock();
	 }

	 if((*queueToExecute).size())
	 {
		 JobBase* jobToExecutePtr = (*queueToExecute)[indexToExecute];
		 indexToExecute++;

		 return jobToExecutePtr;
	 }

	 return nullptr;
 }

//EventJob
JobDispatcher::EventJob::EventJob(EventListenerBase* _eventListenerPtr) :
eventListenerPtr(_eventListenerPtr)
{

}

void JobDispatcher::EventJob::Execute()
{
	eventListenerPtr->HandleEvent();
}

//Worker

JobDispatcher::Worker::Worker(JobQueue* _queuePtr) :
queuePtr(_queuePtr),
executionLock(executionNotificationMutex)
{

}

void JobDispatcher::Worker::Notify()
{
	executionNotification.notify_one();
}

void JobDispatcher::Worker::run()
{
	while(running)
	{
		executionNotification.wait(executionLock);

		if(!running)
		{
			break;
		}

		JobBase* jobPtr = queuePtr->GetNextJob();
		while(jobPtr != nullptr)
		{
			jobPtr->Execute();
			delete jobPtr;
			jobPtr = queuePtr->GetNextJob();
		}
	}
}

