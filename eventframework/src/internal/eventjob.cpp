/*
 * eventjob.cpp
 *
 *  Created on: Apr 27, 2016
 *      Author: janne
 */

#include "internal/eventjob.h"

EventJob::EventJob(EventListenerBase* _eventListenerPtr,
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

void EventJob::Execute()
{
	eventListenerPtr->HandleEvent(eventNo, eventDataPtr);
	if(nullptr != eventDataPtr)
	{
		delete eventDataPtr;
	}
}
