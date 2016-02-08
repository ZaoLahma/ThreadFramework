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

WordFinderJob::WordFinderJob(const uint32_t _eventOffset) :
eventOffset(_eventOffset)
{

}

void WordFinderJob::Execute()
{
	//std::cout<<"Execute called"<<std::endl;

	WordFinderData* wordFinderDataPtr = static_cast<WordFinderData*>(dataPtr);

	std::size_t pos = wordFinderDataPtr->wordString.find(wordFinderDataPtr->wordToFind);

	while(std::string::npos != pos)
	{
		wordFinderDataPtr->noOfOccurances++;
		pos = wordFinderDataPtr->wordString.find(wordFinderDataPtr->wordToFind, pos + 1);
	}

	JobDispatcher::GetApi()->RaiseEvent(WORD_FINDER_JOB_FINISHED + eventOffset);
}
