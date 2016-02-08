/*
 * eventlistenerbase.h
 *
 *  Created on: Feb 7, 2016
 *      Author: janne
 */

#ifndef INC_EVENTLISTENERBASE_H_
#define INC_EVENTLISTENERBASE_H_

#include <vector>
#include <map>

class EventListenerBase
{
public:
	virtual ~EventListenerBase() {}
	virtual void HandleEvent() = 0;

protected:

private:

};

typedef std::vector<EventListenerBase*> EventListenerBasePtrVector;

typedef std::map<uint32_t, EventListenerBasePtrVector> SubscriberEventMap;


#endif /* INC_EVENTLISTENERBASE_H_ */
