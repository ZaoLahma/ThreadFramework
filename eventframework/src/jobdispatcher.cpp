/*
 * jobdispatcher.cpp
 *
 *  Created on: Feb 3, 2016
 *      Author: janne
 */

#include <sstream>
#include <iostream>
#include <thread>
#include <iomanip>
#include <cstdarg>

#include "jobdispatcher.h"
#include "uniqueidprovider.h"

#include "internal/logjob.h"
#include "internal/eventjob.h"
#include "internal/eventtimerjob.h"

#ifdef __APPLE__
#include <mach/mach.h>
#include <mach/mach_init.h>
#include <mach/mach_time.h>
#include <mach/clock.h>
#endif

JobDispatcher* JobDispatcher::instance = nullptr;
std::mutex JobDispatcher::instanceCreationMutex;

JobDispatcher::JobDispatcher() :
noOfCores(std::thread::hardware_concurrency()),
executionFinishedNotificationLock(executionFinishedNotificationMutex),
jobQueuePtr(new JobQueue())
{
	timerWheelPtr = new TimerWheel();
	timerWheelPtr->Start();
	std::ofstream fileStream;
	fileStream.open("log.txt", std::ios::trunc);
	fileStream.clear();
}

JobDispatcher::~JobDispatcher()
{
	std::cout<<"JobDispatcher::DTOR called. No of workers: "<<workers.size()<<std::endl;

	timerWheelPtr->Stop();

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


	delete timerWheelPtr;
	timerWheelPtr = nullptr;
	delete jobQueuePtr;
	jobQueuePtr = nullptr;
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

void JobDispatcher::Log(const char* formatString, ...)
{
	std::stringstream stringToPrint;

	//std::time_t time_c = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	//stringToPrint<<std::put_time(std::localtime(&time_c), "[%F %T] ");

	stringToPrint<<GetTimeStamp();

	char buf[1024];

	va_list args;
	va_start (args, formatString);
	vsprintf (buf, formatString, args);
	va_end (args);

	stringToPrint<<std::string(buf)<<std::endl;

	JobDispatcher::GetApi()->ExecuteJob(new LogJob(stringToPrint.str()));
}

std::string JobDispatcher::GetTimeStamp()
{
    timespec ts;
    struct tm* currentTime;

#ifdef WIN32
	time_t now = time(0);
	currentTime = (tm*)malloc(sizeof(tm));
	localtime_s(currentTime, &now);
	ts.tv_nsec = 0;
#else
	#ifdef __APPLE__
		clock_serv_t cclock;
		mach_timespec_t mts;
		host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
		clock_get_time(cclock, &mts);
		mach_port_deallocate(mach_task_self(), cclock);
		ts.tv_sec = mts.tv_sec;
		ts.tv_nsec = mts.tv_nsec;
	#else
		clock_gettime(CLOCK_REALTIME, &ts);
	#endif
	
	currentTime = localtime(&(ts.tv_sec));
#endif

    unsigned int year = currentTime->tm_year + 1900;
    unsigned int month = currentTime->tm_mon + 1;
    unsigned int day = currentTime->tm_mday;
    unsigned int hour = currentTime->tm_hour;
    unsigned int minute = currentTime->tm_min;
    unsigned int second = currentTime->tm_sec;
    unsigned int nsecond = ts.tv_nsec;
    
#ifdef WIN32
	free(currentTime);
#endif

	const uint8_t buf_size = 31;
	char buf[buf_size];
    sprintf(buf, 
		    "[%d-%02d-%02d %02d:%02d:%02d.%.9d]",
		    year,
		    month,
		    day,
		    hour,
		    minute,
		    second,
		    nsecond);

    return std::string(buf);
}

void JobDispatcher::ExecuteJob(JobBase* jobPtr)
{
	/*
	 * Queue job and notify first idling worker (if any)
	 */
	jobQueuePtr->QueueJob(jobPtr);

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
	Worker* worker = new Worker(jobQueuePtr);
	worker->Start();
	workers.push_back(worker);
}

void JobDispatcher::ExecuteJobIn(JobBase* jobPtr, const uint32_t ms)
{
	timerWheelPtr->AddJob(ms, jobPtr);
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
			EventJob* eventJob = new EventJob(*eventListenerIter, eventNo, eventDataPtr);

			JobDispatcher::GetApi()->ExecuteJob(eventJob);
		}
		delete eventDataPtr;
		eventDataPtr = nullptr;
	}
}

void JobDispatcher::RaiseEventIn(const uint32_t eventNo, const EventDataBase* eventDataPtr, const uint32_t ms)
{
	timerWheelPtr->AddJob(ms, new EventTimerJob(eventNo, eventDataPtr));
}


void JobDispatcher::WaitForExecutionFinished()
{
	executionFinishedNotification.wait(executionFinishedNotificationLock);
}

void JobDispatcher::NotifyExecutionFinished()
{
	executionFinishedNotification.notify_one();
}
