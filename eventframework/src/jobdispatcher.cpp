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
executionFinishedNotificationLock(executionFinishedNotificationMutex),
currentWorkerIndex(0)
{
	uint32_t noOfCores = std::thread::hardware_concurrency();

	for(uint32_t i = 0; i < noOfCores; ++i)
	{
		Worker* worker = new Worker(&jobQueue);
		worker->Start();
		workers.push_back(worker);
	}
}

JobDispatcher::~JobDispatcher()
{
	WorkerPtrVector::iterator workerIter = workers.begin();

	for( ; workerIter != workers.end(); ++workerIter)
	{
		(*workerIter)->Stop();
		delete (*workerIter);
	}

	workers.clear();
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

void JobDispatcher::ExecuteJobIn(JobBase* jobPtr, const uint32_t ms)
{
	JobDispatcher::JobTimer* timerPtr = new JobDispatcher::JobTimer(jobPtr, ms);
	/*
	 * TODO: Fix memory leak as described below:
	 *
	 * Store timer in timers vector
	 * Need to assign a unique ID to the timer which it will raise as an event.
	 * When the event is received, delete the corresponding timer.
	 */
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

				if(0 == eventIter->second.size())
				{
					eventListeners.erase(eventIter);
				}

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
			JobDispatcher::EventJob* eventJob = new JobDispatcher::EventJob(*eventListenerIter, eventNo);

			JobDispatcher::GetApi()->ExecuteJob(eventJob);
		}
	}
}

void JobDispatcher::HandleEvent(const uint32_t eventNo)
{
	//TODO: Find timer when it has fired and delete it
}

void JobDispatcher::WaitForExecutionFinished()
{
	executionFinishedNotification.wait(executionFinishedNotificationLock);
}

void JobDispatcher::NotifyExecutionFinished()
{
	executionFinishedNotification.notify_one();
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
JobDispatcher::EventJob::EventJob(EventListenerBase* _eventListenerPtr, const uint32_t _eventNo) :
eventListenerPtr(_eventListenerPtr),
eventNo(_eventNo)
{

}

void JobDispatcher::EventJob::Execute()
{
	eventListenerPtr->HandleEvent(eventNo);
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
		JobBase* jobPtr = queuePtr->GetNextJob();
		while(jobPtr != nullptr)
		{
			jobPtr->Execute();
			delete jobPtr;
			jobPtr = queuePtr->GetNextJob();
		}

		if(running)
		{
			executionNotification.wait(executionLock);
		}
	}
}

void JobDispatcher::Worker::Stop()
{
	running = false;
	Notify();

	if(this->executorThread.joinable())
	{
		this->executorThread.join();
	}
}

//JobTimer
JobDispatcher::JobTimer::JobTimer(JobBase* _jobPtr, const uint32_t _ms) :
ms(_ms),
jobPtr(_jobPtr)
{
	this->Start();
}

void JobDispatcher::JobTimer::run()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	JobDispatcher::GetApi()->ExecuteJob(jobPtr);
}

