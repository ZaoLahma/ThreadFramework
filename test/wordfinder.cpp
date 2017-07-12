/*
 * wordfinder.cpp
 *
 *  Created on: Feb 8, 2016
 *      Author: janne
 */


#include "wordfinder.h"
#include "exec_groups.h"

#include <iostream>

std::mutex WordFinder::instanceMutex;

WordFinder::WordFinder(const std::string& string, const std::string& wordToFind, const uint32_t _instanceNo) :
instanceNo(_instanceNo)
{
	jobData = std::make_shared<WordFinderData>(string, wordToFind);

	JobDispatcher::GetApi()->SubscribeToEvent(WORD_FINDER_JOB_FINISHED, this);

	std::shared_ptr<JobBase> jobPtr = std::make_shared<WordFinderJob>(instanceNo);

	jobPtr->SetJobData(jobData);

	uint32_t execGroup = LOW_PRIO_EXEC_GROUP;

	if(instanceNo == 2 || instanceNo == 3 || instanceNo == 4)
	{
		execGroup = HIGH_PRIO_EXEC_GROUP;
	}

	JobDispatcher::GetApi()->ExecuteJobInGroup(jobPtr, execGroup);
}

void WordFinder::HandleEvent(const uint32_t eventNo, std::shared_ptr<EventDataBase> dataPtr)
{
	if(WORD_FINDER_JOB_FINISHED == eventNo)
	{
		std::shared_ptr<WordFinderJobFinishedEventData> eventDataPtr = std::static_pointer_cast<WordFinderJobFinishedEventData>(dataPtr);

		if(eventDataPtr->instanceId == instanceNo)
		{
			JobDispatcher::GetApi()->UnsubscribeToEvent(WORD_FINDER_JOB_FINISHED, this);
			JobDispatcher::GetApi()->Log("String: %s, Word: %s, No of occurrences: %d",
					                      jobData->wordString.c_str(),
										  jobData->wordToFind.c_str(),
										  jobData->noOfOccurances);

			std::lock_guard<std::mutex> printLock(instanceMutex);
			std::cout<<"String: "<<jobData->wordString<<". Word: "<<jobData->wordToFind<<". No of occurrences: "<<jobData->noOfOccurances<<std::endl;
		}
	}
}
