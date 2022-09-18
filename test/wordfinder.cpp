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

WordFinder::WordFinder(const std::string& string, const std::string& wordToFind, const uint32_t _instanceNo, const uint32_t _execGroup) :
instanceNo(_instanceNo), execGroup(_execGroup)
{
	jobData = std::make_shared<WordFinderData>(string, wordToFind);

	JobDispatcher::GetApi()->SubscribeToEvent(WORD_FINDER_JOB_FINISHED, this);

	std::shared_ptr<JobBase> jobPtr = std::make_shared<WordFinderJob>(instanceNo);

	jobPtr->SetJobData(jobData);

    /*
	 *  Trigger the job in the specified execution group
	 */
	JobDispatcher::GetApi()->ExecuteJobInGroup(jobPtr, execGroup);
}

void WordFinder::HandleEvent(const uint32_t eventNo, std::shared_ptr<EventDataBase> dataPtr)
{
	/*
	 *  Wait for the event that indicates that a WordFinder job is finished,
	 *  check if it belongs to this particular instance and if so print
	 *  the outcome of the job to the log and to the terminal
	 * 
	 *  The result of the job is found in the jobData pointer that we passed to
	 *  the WordFinderJob instance.
	 */
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
