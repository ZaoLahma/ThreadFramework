/*
 * timerstorage.cpp
 *
 *  Created on: Apr 27, 2016
 *      Author: janne
 */

#include "internal/timerstorage.h"

#include "uniqueidprovider.h"
#include "jobdispatcher.h"

TimerStorage::TimerStorage() :
subscribedToEvent(false)
{

}

TimerStorage::~TimerStorage()
{
	JobDispatcher::GetApi()->UnsubscribeToEvent(TIMEOUT_EVENT_ID, this);

	TimerBaseMap::iterator timerIter = timers.begin();

	for( ; timerIter != timers.end(); ++timerIter)
	{
		delete timerIter->second;
	}

	timers.clear();
}

void TimerStorage::StoreTimer(TimerBase* _timer)
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

void TimerStorage::HandleEvent(const uint32_t _eventNo, std::shared_ptr<EventDataBase> _dataPtr)
{
	if(TIMEOUT_EVENT_ID == _eventNo)
	{
		std::shared_ptr<TimerEventData> eventDataPtr = std::static_pointer_cast<TimerEventData>(_dataPtr);

		std::unique_lock<std::mutex> timersMapLock(timerMutex);

		TimerBaseMap::iterator timerIter = timers.find(eventDataPtr->GetTimerId());

		if(timers.end() != timerIter)
		{
			delete timerIter->second;
			timers.erase(timerIter);
		}
	}
}
