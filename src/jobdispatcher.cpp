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
#include "internal/jobtimer.h"
#include "internal/eventjob.h"
#include "internal/eventtimer.h"
#include "internal/timereventdata.h"

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
jobQueueContainer(new JobQueueWorkerContainer())
{
	std::ofstream fileStream;
	fileStream.open("log.txt", std::ios::trunc);
	fileStream.clear();

	/*
	 * Nothing from within the framework is executed on the
	 * default exec group. It's for legacy applications and
	 * has an unlimited max amount of threads.
	 */
	jobQueueContainer->AddExecGroup(DEFAULT_EXEC_GROUP_ID, 0);

	/*
	 * Create logging exec group with one thread by default
	 * and an event exec group with 2 threads. These values
	 * can be overridden by a later call to AddExecGroup.
	 */
	jobQueueContainer->AddExecGroup(LOGGING_EXEC_GROUP_ID, 1);
	jobQueueContainer->AddExecGroup(EVENT_EXEC_GROUP_ID, 2);
}

JobDispatcher::~JobDispatcher()
{
	delete jobQueueContainer;
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
	if(nullptr != instance)
	{
		instanceCreationMutex.lock();
		if(nullptr != instance)
		{
			delete instance;
			instance = nullptr;
		}
		instanceCreationMutex.unlock();
	}
}

void JobDispatcher::Log(const char* formatString, ...)
{
	std::stringstream stringToPrint;

	//std::time_t time_c = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	//stringToPrint<<std::put_time(std::localtime(&time_c), "[%F %T] ");

	stringToPrint<<GetTimeStamp();

	char buf[2048];

	va_list args;
	va_start(args, formatString);
	vsprintf(buf, formatString, args);
	va_end (args);

	stringToPrint<<std::string(buf)<<std::endl;

	std::shared_ptr<JobBase> logJob = std::make_shared<LogJob>(stringToPrint.str());

	JobDispatcher::GetApi()->ExecuteJobInGroup(logJob, LOGGING_EXEC_GROUP_ID);
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

void JobDispatcher::AddExecGroup(uint32_t groupId, uint32_t maxNoOfThreads)
{
	jobQueueContainer->AddExecGroup(groupId, maxNoOfThreads);
}

void JobDispatcher::ExecuteJob(std::shared_ptr<JobBase> jobPtr)
{
	jobQueueContainer->ScheduleJob(DEFAULT_EXEC_GROUP_ID, jobPtr);
}

void JobDispatcher::ExecuteJobInGroup(std::shared_ptr<JobBase> jobPtr, uint32_t groupId)
{
	jobQueueContainer->ScheduleJob(groupId, jobPtr);
}

void JobDispatcher::ExecuteJobIn(std::shared_ptr<JobBase> jobPtr, const uint32_t ms)
{
	std::shared_ptr<JobTimer> timerPtr = std::make_shared<JobTimer>(jobPtr, ms);

	timerStorage.StoreTimer(timerPtr);
}

void JobDispatcher::ExecuteJobInGroupIn(std::shared_ptr<JobBase> jobPtr, uint32_t groupId, uint32_t ms)
{
	std::shared_ptr<TimerBase> timerPtr = std::make_shared<JobTimer>(jobPtr, ms, groupId);

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

void JobDispatcher::RaiseEvent(uint32_t eventNo, std::shared_ptr<EventDataBase> eventDataPtr)
{
	std::lock_guard<std::mutex> subscribersLock(eventListenersAccessMutex);

	EventNoToEventListenersMap::iterator eventIter = eventEventListeners.find(eventNo);

	if(eventEventListeners.end() != eventIter)
	{
		EventListenerBasePtrVector::iterator eventListenerIter = eventIter->second.begin();

		for( ; eventListenerIter != eventIter->second.end(); ++eventListenerIter)
		{
			std::shared_ptr<JobBase> eventJob = std::make_shared<EventJob>(*eventListenerIter, eventNo, eventDataPtr);

			JobDispatcher::GetApi()->ExecuteJobInGroup(eventJob, EVENT_EXEC_GROUP_ID);
		}
		eventDataPtr = nullptr;
	}
}

void JobDispatcher::RaiseEventIn(const uint32_t eventNo, std::shared_ptr<EventDataBase> eventDataPtr, const uint32_t ms)
{
	std::shared_ptr<TimerBase> eventTimerPtr = std::make_shared<EventTimer>(eventNo, eventDataPtr, ms);

	timerStorage.StoreTimer(eventTimerPtr);
}


void JobDispatcher::WaitForExecutionFinished()
{
	std::unique_lock<std::mutex> executionFinishedNotificationLock = std::unique_lock<std::mutex>(executionFinishedNotificationMutex);
	executionFinishedNotification.wait(executionFinishedNotificationLock);
}

void JobDispatcher::NotifyExecutionFinished()
{
	std::unique_lock<std::mutex> executionFinishedNotificationLock = std::unique_lock<std::mutex>(executionFinishedNotificationMutex);
	executionFinishedNotification.notify_one();
}
