/*
 * timerwheel.h
 *
 *  Created on: Aug 10, 2016
 *      Author: janne
 */

#ifndef INC_INTERNAL_TIMERWHEEL_H_
#define INC_INTERNAL_TIMERWHEEL_H_

#include "../jobbase.h"
#include "threadobject.h"
#include <mutex>

class TimerWheel : public ThreadObject
{
public:
	TimerWheel();
	~TimerWheel();

	void AddJob(uint32_t ms, JobBase* jobPtr);

protected:

private:
	void run();
	std::mutex addJobMutex;
	uint32_t arrayIndex;
	JobBasePtrVectorT msArray[1000];
};



#endif /* INC_INTERNAL_TIMERWHEEL_H_ */
