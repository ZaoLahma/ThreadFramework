/*
 * timerwheel.cpp
 *
 *  Created on: Aug 10, 2016
 *      Author: janne
 */

#include "../../inc/internal/timerwheel.h"
#include "../../inc/jobdispatcher.h"

TimerWheel::TimerWheel() :
arrayIndex(0)
{
}

TimerWheel::~TimerWheel() {

	std::lock_guard<std::mutex> addJobLock(addJobMutex);

	for(uint32_t i = 0; i < 1000; ++i) {
		JobBasePtrVectorT::iterator jobIter = msArray[arrayIndex].begin();
		for( ; jobIter != msArray[arrayIndex].end(); ++jobIter) {
			printf("Deleting job\n");
			delete *jobIter;
		}
		msArray[i].clear();
	}

	LongRunningTimerVectorT::iterator timerIter = longRunningTimers.begin();

	for( ; timerIter != longRunningTimers.end(); ++timerIter) {
		delete (*timerIter).jobPtr;
	}

	longRunningTimers.clear();
}

void TimerWheel::AddJob(uint32_t ms, JobBase* jobPtr) {
	std::lock_guard<std::mutex> addJobLock(addJobMutex);

	InsertJobToTimerArray(ms, jobPtr);
}

void TimerWheel::InsertJobToTimerArray(uint32_t ms, JobBase* jobPtr) {
	printf("Started new timer set to expire in %d ms\n", ms);

	if(ms > 999) {
		LongRunningTimer newTimer;
		newTimer.ms = ms;
		newTimer.jobPtr = jobPtr;
		longRunningTimers.push_back(newTimer);
		return;
	}

	uint32_t insertIndex = arrayIndex + ms;

	if(insertIndex > 999) {
	 insertIndex = insertIndex - 1000;
	}

	msArray[insertIndex].push_back(jobPtr);
}

void TimerWheel::run() {
	while(running) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		std::lock_guard<std::mutex> addJobLock(addJobMutex);

		if(!running) {
			break;
		}

		JobBasePtrVectorT::iterator jobIter = msArray[arrayIndex].begin();
		for( ; jobIter != msArray[arrayIndex].end(); ++jobIter) {
			JobDispatcher::GetApi()->ExecuteJob(*jobIter);
		}

		msArray[arrayIndex].clear();

		arrayIndex++;

		if(arrayIndex > 999) {
			arrayIndex = 0;

			//Check if any of the long running timers should be scheduled
			LongRunningTimerVectorT::iterator timerIter = longRunningTimers.begin();
			while (timerIter != longRunningTimers.end()) {
				printf("Checking if long running timer should be scheduled\n");
				(*timerIter).ms -= 1000;
				if((*timerIter).ms < 1000) {
					InsertJobToTimerArray((*timerIter).ms, (*timerIter).jobPtr);
					timerIter = longRunningTimers.erase(timerIter);
					continue;
				}
				timerIter++;
			}
		}
	}
}
