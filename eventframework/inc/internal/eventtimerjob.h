/*
 * eventtimerjob.h
 *
 *  Created on: Aug 10, 2016
 *      Author: janne
 */

#ifndef INC_INTERNAL_EVENTTIMERJOB_H_
#define INC_INTERNAL_EVENTTIMERJOB_H_

#include "jobbase.h"
#include "../eventdatabase.h"

class EventTimerJob : public JobBase {
public:
	EventTimerJob(uint32_t eventNo, const EventDataBase* eventDataPtr);
	void Execute();

protected:

private:
	EventTimerJob();
	uint32_t eventNo;
	const EventDataBase* eventDataPtr;
};



#endif /* INC_INTERNAL_EVENTTIMERJOB_H_ */
