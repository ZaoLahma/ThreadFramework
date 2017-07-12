/*
 * eventjob.cpp
 *
 *  Created on: Apr 27, 2016
 *      Author: janne
 */

#include "internal/eventjob.h"

EventJob::EventJob(EventListenerBase* _eventListenerPtr,
				   const uint32_t _eventNo,
				   std::shared_ptr<EventDataBase> _eventDataPtr) :
eventListenerPtr(_eventListenerPtr),
eventNo(_eventNo),
eventDataPtr(_eventDataPtr)
{

}

void EventJob::Execute()
{
	eventListenerPtr->HandleEvent(eventNo, eventDataPtr);
	eventDataPtr = nullptr;
}
