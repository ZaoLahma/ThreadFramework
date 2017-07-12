/*
 * eventtimer.cpp
 *
 *  Created on: Apr 27, 2016
 *      Author: janne
 */

#include "internal/eventtimer.h"
#include "jobdispatcher.h"

EventTimer::EventTimer(const uint32_t _eventNo, std::shared_ptr<EventDataBase> _dataPtr, const uint32_t _ms) :
TimerBase(_ms),
eventNo(_eventNo),
eventDataPtr(_dataPtr)
{

}

void EventTimer::TimerFunction()
{
	JobDispatcher::GetApi()->RaiseEvent(eventNo, eventDataPtr);
	eventDataPtr = nullptr;
}
