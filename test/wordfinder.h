/*
 * wordfinder.h
 *
 *  Created on: Feb 8, 2016
 *      Author: janne
 */

#ifndef TEST_WORDFINDER_H_
#define TEST_WORDFINDER_H_

#include "wordfinderjob.h"

#include "jobdispatcher.h"

#include <mutex>

class WordFinder : public EventListenerBase
{
public:
	WordFinder(const std::string& string, const std::string& wordToFind, const uint32_t _instanceNo);

	void HandleEvent(const uint32_t eventNo, std::shared_ptr<EventDataBase> dataPtr);

private:
	WordFinder();

	std::shared_ptr<WordFinderData> jobData;

	const uint32_t instanceNo;

	static std::mutex instanceMutex;
};



#endif /* TEST_WORDFINDER_H_ */
