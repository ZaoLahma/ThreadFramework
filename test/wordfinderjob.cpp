/*
 * wordfinderjob.cpp
 *
 *  Created on: Feb 8, 2016
 *      Author: janne
 */

#include "wordfinderjob.h"
#include "jobdispatcher.h"

#include <iostream>

WordFinderData::WordFinderData(const std::string& _wordString,
			                   const std::string& _wordToFind) :
wordString(_wordString),
wordToFind(_wordToFind),
noOfOccurances(0)
{

}

WordFinderJob::WordFinderJob(const uint32_t _instanceNo) :
instanceNo(_instanceNo)
{

}

void WordFinderJob::Execute()
{
	std::shared_ptr<WordFinderData> wordFinderDataPtr = std::static_pointer_cast<WordFinderData>(dataPtr);

	std::size_t pos = wordFinderDataPtr->wordString.find(wordFinderDataPtr->wordToFind);

	while(std::string::npos != pos)
	{
		wordFinderDataPtr->noOfOccurances++;
		pos = wordFinderDataPtr->wordString.find(wordFinderDataPtr->wordToFind, pos + 1);
	}

    /*
	 *  Raise the event that the WordFinder class is waiting for, indicating
	 *  that this particular instance of the job is finished.
	 */
	std::shared_ptr<WordFinderJobFinishedEventData> eventDataPtr = std::make_shared<WordFinderJobFinishedEventData>();
	eventDataPtr->instanceId = instanceNo;

	JobDispatcher::GetApi()->RaiseEvent(WORD_FINDER_JOB_FINISHED, eventDataPtr);
}
