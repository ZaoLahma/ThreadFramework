/*
 * jobdispatcher.cpp
 *
 *  Created on: Feb 3, 2016
 *      Author: janne
 */

#include "jobdispatcher.h"
#include "uniqueidprovider.h"
#include <iostream>
#include <thread>

JobDispatcher* JobDispatcher::instance = nullptr;
std::mutex JobDispatcher::instanceCreationMutex;

JobDispatcher::JobDispatcher() :
noOfCores(std::thread::hardware_concurrency()),
executionFinishedNotificationLock(executionFinishedNotificationMutex)
{

}

JobDispatcher::~JobDispatcher()
{
	std::cout<<"JobDispatcher::DTOR called. No of workers: "<<workers.size()<<std::endl;

	WorkerPtrVector::iterator workerIter = workers.begin();

	uint32_t index = 0;
	for( ; workerIter != workers.end(); ++workerIter)
	{
		std::cout<<"Worker: "<<index<<" executed "<<(*workerIter)->GetNoOfJobsExecuted()<<" jobs."<<std::endl;
		(*workerIter)->Stop();
		delete (*workerIter);
		index++;
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
	/*
	 * Queue job and notify first idling worker (if any)
	 */
	jobQueue.QueueJob(jobPtr);

	std::lock_guard<std::mutex> workerCreationLock(workerCreationMutex);
	WorkerPtrVector::iterator workerIter = workers.begin();

	for( ; workerIter != workers.end(); ++workerIter)
	{
		if((*workerIter)->IsIdling())
		{
			(*workerIter)->Notify();
			return;
		}
	}

	/*
	 * If no idling worker found, we allocate a new one.
	 * The framework will adjust upwards to the worst
	 * case, but never downwards.
	 */
	Worker* worker = new Worker(&jobQueue);
	worker->Start();
	worker->Notify();
	workers.push_back(worker);
}

void JobDispatcher::ExecuteJobIn(JobBase* jobPtr, const uint32_t ms)
{
	JobDispatcher::JobTimer* timerPtr = new JobDispatcher::JobTimer(jobPtr, ms);

	timerStorage.StoreTimer(timerPtr);
}

void JobDispatcher::SubscribeToEvent(uint32_t eventNo, EventListenerBase* eventListenerPtr)
{
	std::lock_guard<std::mutex> subscribersLock(eventListenersAccessMutex);

	EventNoToEventListenersMap::iterator eventIter = eventEventListeners.find(eventNo);

	if(eventEventListeners.end() == eventIter)
	{
		EventListenerBasePtrVector newElement;
		eventEventListeners[eventNo] = newElement;
		eventIter = eventEventListeners.find(eventNo);
	}

	eventIter->second.push_back(eventListenerPtr);

	EventListenerToEventNoMap::iterator eventListenerIter = eventListenersEvents.find(eventListenerPtr);

	if(eventListenersEvents.end() == eventListenerIter)
	{
		std::vector<uint32_t> eventsVector;
		eventListenersEvents[eventListenerPtr] = eventsVector;
		eventListenerIter = eventListenersEvents.begin();
	}

	eventListenerIter->second.push_back(eventNo);
}

void JobDispatcher::UnsubscribeToEvent(uint32_t eventNo, EventListenerBase* eventListenerPtr)
{
	std::lock_guard<std::mutex> subscribersLock(eventListenersAccessMutex);

	EventNoToEventListenersMap::iterator eventIter = eventEventListeners.find(eventNo);

	if(eventEventListeners.end() != eventIter)
	{
		EventListenerBasePtrVector::iterator eventListenersIter = eventIter->second.begin();

		for( ; eventListenersIter != eventIter->second.end(); ++eventListenersIter)
		{
			if(*eventListenersIter == eventListenerPtr)
			{
				eventIter->second.erase(eventListenersIter);

				if(0 == eventIter->second.size())
				{
					eventEventListeners.erase(eventIter);
				}

				break;
			}
		}
	}
}

void JobDispatcher::RaiseEvent(uint32_t eventNo, const EventDataBase* eventDataPtr)
{
	std::lock_guard<std::mutex> subscribersLock(eventListenersAccessMutex);

	EventNoToEventListenersMap::iterator eventIter = eventEventListeners.find(eventNo);

	if(eventEventListeners.end() != eventIter)
	{
		EventListenerBasePtrVector::iterator eventListenerIter = eventIter->second.begin();

		for( ; eventListenerIter != eventIter->second.end(); ++eventListenerIter)
		{
			JobDispatcher::EventJob* eventJob = new JobDispatcher::EventJob(*eventListenerIter, eventNo, eventDataPtr);

			JobDispatcher::GetApi()->ExecuteJob(eventJob);
		}
		delete eventDataPtr;
		eventDataPtr = nullptr;
	}
}

void JobDispatcher::RaiseEventIn(const uint32_t eventNo, const EventDataBase* eventDataPtr, const uint32_t ms)
{
	JobDispatcher::EventTimer* eventTimerPtr = new JobDispatcher::EventTimer(eventNo, eventDataPtr, ms);

	timerStorage.StoreTimer(eventTimerPtr);
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
JobDispatcher::EventJob::EventJob(EventListenerBase* _eventListenerPtr,
		                          const uint32_t _eventNo,
								  const EventDataBase* _eventDataPtr) :
eventListenerPtr(_eventListenerPtr),
eventNo(_eventNo),
eventDataPtr(nullptr)
{
	if(nullptr != _eventDataPtr)
	{
		eventDataPtr = _eventDataPtr->clone();
	}
}

void JobDispatcher::EventJob::Execute()
{
	eventListenerPtr->HandleEvent(eventNo, eventDataPtr);
	if(nullptr != eventDataPtr)
	{
		delete eventDataPtr;
	}
}

//Worker

JobDispatcher::Worker::Worker(JobQueue* _queuePtr) :
noOfJobsExecuted(0),
queuePtr(_queuePtr),
executionLock(executionNotificationMutex),
isIdling(false)
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
		isIdling = true;
		executionNotification.wait(executionLock);
		isIdling = false;

		if(running)
		{
			JobBase* jobPtr = queuePtr->GetNextJob();
			while(jobPtr != nullptr)
			{
				jobPtr->Execute();
				noOfJobsExecuted++;
				delete jobPtr;
				jobPtr = queuePtr->GetNextJob();
			}
		}
	}
}

bool JobDispatcher::Worker::IsIdling()
{
	return isIdling;
}

uint32_t JobDispatcher::Worker::GetNoOfJobsExecuted()
{
	return noOfJobsExecuted;
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

//TimerEventData
JobDispatcher::TimerEventData::TimerEventData(const uint32_t _timerId) :
timerId(_timerId)
{

}

uint32_t JobDispatcher::TimerEventData::GetTimerId() const
{
	return timerId;
}

EventDataBase* JobDispatcher::TimerEventData::clone() const
{
	return new TimerEventData(*this);
}

//TimerBase
JobDispatcher::TimerBase::TimerBase(const uint32_t _ms) :
ms(_ms),
timerId(0)
{

}

void JobDispatcher::TimerBase::SetTimerId(const uint32_t _timerId)
{
	timerId = _timerId;
}

void JobDispatcher::TimerBase::run()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));

	if(running)
	{
		//Execute this particular timer type's function
		this->TimerFunction();

		//Notify timer triggered
		TimerEventData* eventDataPtr = new TimerEventData(timerId);
		JobDispatcher::GetApi()->RaiseEvent(TIMEOUT_EVENT_ID, eventDataPtr);
	}
}

//JobTimer
JobDispatcher::JobTimer::JobTimer(JobBase* _jobPtr, const uint32_t _ms) :
TimerBase(_ms),
jobPtr(_jobPtr)
{

}

void JobDispatcher::JobTimer::TimerFunction()
{
	JobDispatcher::GetApi()->ExecuteJob(jobPtr);
}

//EventTimer
JobDispatcher::EventTimer::EventTimer(const uint32_t _eventNo, const EventDataBase* _dataPtr, const uint32_t _ms) :
TimerBase(_ms),
eventNo(_eventNo),
eventDataPtr(_dataPtr)
{

}

void JobDispatcher::EventTimer::TimerFunction()
{
	JobDispatcher::GetApi()->RaiseEvent(eventNo, eventDataPtr);
}

//TimerStorage
JobDispatcher::TimerStorage::TimerStorage() :
subscribedToEvent(false)
{

}

JobDispatcher::TimerStorage::~TimerStorage()
{
	TimerBaseMap::iterator timerIter = timers.begin();

	for( ; timerIter != timers.end(); ++timerIter)
	{
		delete timerIter->second;
	}

	timers.clear();
}

void JobDispatcher::TimerStorage::StoreTimer(TimerBase* _timer)
{
	if(false == subscribedToEvent)
	{
		/*
		 *Can't do the subscription in the CTOR
		 *due to it trying to create another
		 *JobDispatcher instance
		 */
		subscribeMutex.lock();
		if(false == subscribedToEvent)
		{
			subscribedToEvent = true;
			JobDispatcher::GetApi()->SubscribeToEvent(TIMEOUT_EVENT_ID, this);
		}
		subscribeMutex.unlock();
	}

	const uint32_t timerId = UniqueIdProvider::GetApi()->GetUniqueId();

	_timer->SetTimerId(timerId);
	std::unique_lock<std::mutex> timersMapLock(timerMutex);
	timers[timerId] = _timer;
	_timer->Start();
}

void JobDispatcher::TimerStorage::HandleEvent(const uint32_t _eventNo, const EventDataBase* _dataPtr)
{
	if(TIMEOUT_EVENT_ID == _eventNo)
	{
		const TimerEventData* eventDataPtr = static_cast<const TimerEventData*>(_dataPtr);

		std::unique_lock<std::mutex> timersMapLock(timerMutex);

		TimerBaseMap::iterator timerIter = timers.find(eventDataPtr->GetTimerId());

		if(timers.end() != timerIter)
		{
			delete timerIter->second;
			timers.erase(timerIter);
		}
	}

}
