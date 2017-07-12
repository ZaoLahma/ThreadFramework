/*
 * jobqueueworkercontainer.cpp
 *
 *  Created on: Sep 1, 2016
 *      Author: janne
 */

#include "../inc/internal/jobqueueworkercontainer.h"
#include <iostream>

JobQueueWorkerContainer::JobQueueWorkerContainer()
{

}

JobQueueWorkerContainer::~JobQueueWorkerContainer()
{
	JobQueueMapT::iterator queueIter = queueMap.begin();

	for( ; queueIter != queueMap.end(); ++queueIter)
	{
		if(0 != queueIter->second.workers.size())
		{
			std::cout<<"Exec group: "<<queueIter->first<<std::endl;
		}
		WorkerPtrVectorT::iterator workers = queueIter->second.workers.begin();

		uint32_t index = 0;
		for(; workers != queueIter->second.workers.end(); ++workers)
		{
			(*workers)->Stop();
			std::cout<<"  Worker: "<<index<<" executed "<<(*workers)->GetNoOfJobsExecuted()<<" jobs."<<std::endl;
			delete *workers;
			index++;
		}

		delete queueIter->second.jobQueuePtr;
	}

	queueMap.clear();
}


void JobQueueWorkerContainer::AddExecGroup(uint32_t groupId, uint32_t maxNoOfThreads)
{
	CreateExecGroup(groupId, maxNoOfThreads);
}

void JobQueueWorkerContainer::ScheduleJob(uint32_t groupId, std::shared_ptr<JobBase> jobPtr)
{
	JobQueueMapT::iterator queueIter = queueMap.find(groupId);

	if(queueIter == queueMap.end())
	{
		CreateExecGroup(groupId, 0);
		queueIter = queueMap.find(groupId);
	}

	queueIter->second.jobQueuePtr->QueueJob(jobPtr);

	std::lock_guard<std::mutex> workerCreationLock(workerCreationMutex);
	WorkerPtrVectorT::iterator workerIter = queueIter->second.workers.begin();

	for( ; workerIter != queueIter->second.workers.end(); ++workerIter)
	{
		if((*workerIter)->IsIdling())
		{
			(*workerIter)->Notify();
			return;
		}
	}

	/*
	 * No available worker was found, so let's
	 * check if we're allowed to create one.
	 * 0 implies an infinite number of workers is
	 * allowed.
	 */
	if(0 == queueIter->second.maxNoOfWorkers ||
	   queueIter->second.workers.size() < queueIter->second.maxNoOfWorkers)
	{
		Worker* worker = new Worker(queueIter->second.jobQueuePtr);
		worker->Start();
		queueIter->second.workers.push_back(worker);
	}
}


void JobQueueWorkerContainer::CreateExecGroup(uint32_t groupId, uint32_t maxNoOfThreads)
{
	std::lock_guard<std::mutex> jobQueueWorkerDataCreationLock(jobQueueWorkerDataCreationMutex);
	JobQueueMapT::iterator queueIter = queueMap.find(groupId);
	if(queueIter == queueMap.end())
	{
		JobQueueWorkerData jobQueueWorkerData;
		jobQueueWorkerData.jobQueuePtr = new JobQueue();
		jobQueueWorkerData.maxNoOfWorkers = maxNoOfThreads;

		queueMap[groupId] = jobQueueWorkerData;
	}
	else
	{
		queueIter->second.maxNoOfWorkers = maxNoOfThreads;
	}
}
