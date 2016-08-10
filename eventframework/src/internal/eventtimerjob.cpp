/*
 * eventtimerjob.cpp
 *
 *  Created on: Aug 10, 2016
 *      Author: janne
 */

#include "../../inc/internal/eventtimerjob.h"
#include "../../inc/jobdispatcher.h"

EventTimerJob::EventTimerJob(uint32_t eventNo, const EventDataBase* eventDataPtr) :
eventNo(eventNo),
eventDataPtr(eventDataPtr)
{

}

void EventTimerJob::Execute() {
	JobDispatcher::GetApi()->RaiseEvent(eventNo, eventDataPtr);
}
