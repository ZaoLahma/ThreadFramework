/*
 * wordfinder.cpp
 *
 *  Created on: Feb 8, 2016
 *      Author: janne
 */


#include "wordfinder.h"

#include <iostream>

std::mutex WordFinder::printMutex;

WordFinder::WordFinder(const std::string& string, const std::string& wordToFind, uint32_t _eventOffset) :
jobData(string, wordToFind),
eventOffset(_eventOffset)
{
	JobDispatcher::GetApi()->SubscribeToEvent(WORD_FINDER_JOB_FINISHED + eventOffset, this);

	WordFinderJob* jobPtr = new WordFinderJob(eventOffset);

	jobPtr->SetJobData(&jobData);

	JobDispatcher::GetApi()->ExecuteJob(jobPtr);
}

void WordFinder::HandleEvent()
{
	JobDispatcher::GetApi()->UnsubscribeToEvent(WORD_FINDER_JOB_FINISHED + eventOffset, this);
	std::lock_guard<std::mutex> printLock(printMutex);
	std::cout<<"String: "<<jobData.wordString<<". Word: "<<jobData.wordToFind<<". No of occurances: "<<jobData.noOfOccurances<<std::endl;
}
