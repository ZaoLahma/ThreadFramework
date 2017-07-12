/*
 * jobqueue.cpp
 *
 *  Created on: Apr 27, 2016
 *      Author: janne
 */

#include "internal/jobqueue.h"

JobQueue::JobQueue() :
currentQueue(&queue_1),
queueToExecute(&queue_2),
currentElement(queueToExecute->end())
{

}

JobQueue::~JobQueue()
{
	JobBasePtrVectorT::iterator jobIter = queue_1.begin();

	for( ; jobIter != queue_1.end(); ++jobIter)
	{
		*jobIter = nullptr;
	}

	queue_1.clear();

	jobIter = queue_2.begin();

	for( ; jobIter != queue_2.end(); ++jobIter)
	{
		*jobIter = nullptr;
	}

	queue_2.clear();
}

 void JobQueue::QueueJob(std::shared_ptr<JobBase> jobPtr)
 {
	 std::lock_guard<std::mutex> lockGuard(queueAccessMutex);
	 currentQueue->push_back(jobPtr);
 }

 std::shared_ptr<JobBase> JobQueue::GetNextJob()
 {
	 std::lock_guard<std::mutex> getJobLock(getJobMutex);

	 if(currentElement == (*queueToExecute).end())
	 {
		 queueAccessMutex.lock();

		 if(currentElement == (*queueToExecute).end())
		 {
			 JobBasePtrVectorT* _currentQueue = currentQueue;

			 queueToExecute->clear();
			 currentQueue = queueToExecute;

			 queueToExecute = _currentQueue;

			 currentElement = (*queueToExecute).begin();
		 }

		 queueAccessMutex.unlock();
	 }

	 if((*queueToExecute).size())
	 {
		 std::shared_ptr<JobBase> jobToExecutePtr = (*currentElement);
		 currentElement++;
		 return jobToExecutePtr;
	 }
	 return nullptr;
 }
