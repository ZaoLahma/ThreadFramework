/*
 * timerbase.cpp
 *
 *  Created on: Apr 27, 2016
 *      Author: janne
 */

#include "internal/timerbase.h"
#include "jobdispatcher.h"

TimerBase::TimerBase(const uint32_t _ms) :
ms(_ms),
timerId(0)
{

}

void TimerBase::SetTimerId(const uint32_t _timerId)
{
	timerId = _timerId;
}

void TimerBase::run()
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
