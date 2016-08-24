/*
 * timereventdata.cpp
 *
 *  Created on: Apr 27, 2016
 *      Author: janne
 */

#include "internal/timereventdata.h"

TimerEventData::TimerEventData(const uint32_t _timerId) :
timerId(_timerId)
{

}

uint32_t TimerEventData::GetTimerId() const
{
	return timerId;
}

EventDataBase* TimerEventData::clone() const
{
	return new TimerEventData(*this);
}
