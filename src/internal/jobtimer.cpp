/*
 * jobtimer.cpp
 *
 *  Created on: Apr 27, 2016
 *      Author: janne
 */

#include "internal/jobtimer.h"
#include "jobdispatcher.h"

JobTimer::JobTimer(JobBase* _jobPtr, const uint32_t _ms, uint32_t _groupId) :
TimerBase(_ms),
jobPtr(_jobPtr),
groupId(_groupId)
{

}

void JobTimer::TimerFunction()
{
	JobDispatcher::GetApi()->ExecuteJobInGroup(jobPtr, groupId);
}
