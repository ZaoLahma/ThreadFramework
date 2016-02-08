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
	WordFinder(const std::string& string, const std::string& wordToFind, uint32_t eventOffset);

	void HandleEvent();

private:
	WordFinder();

	WordFinderData jobData;

	uint32_t eventOffset;

	static std::mutex printMutex;
};



#endif /* TEST_WORDFINDER_H_ */
