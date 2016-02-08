/*
 * jobdispatcher.h
 *
 *  Created on: Feb 3, 2016
 *      Author: janne
 */

#ifndef INC_JOBDISPATCHER_H_
#define INC_JOBDISPATCHER_H_

#include <mutex>
#include <atomic>

#include "jobbase.h"
#include "eventlistenerbase.h"

//For worker
#include <vector>
#include <condition_variable>
#include "threadobject.h"


class JobDispatcher
{
public:
	static JobDispatcher* GetApi();

	static void DropInstance();

	virtual ~JobDispatcher();

	void ExecuteJob(JobBase* jobPtr);

	void SubscribeToEvent(const uint32_t eventNo, EventListenerBase* eventListenerPtr);

	void UnsubscribeToEvent(const uint32_t eventNo, EventListenerBase* eventListenerPtr);

	void RaiseEvent(const uint32_t eventNo);

protected:

private:
	class JobQueue
	{
	public:
		JobQueue();

		~JobQueue();

		void QueueJob(JobBase* jobPtr);
		JobBase* GetNextJob();
	protected:
	private:
		std::mutex queueAccessMutex;
		std::mutex getJobMutex;

		JobBasePtrVectorT queue_1;
		JobBasePtrVectorT queue_2;

		std::atomic<uint32_t> indexToExecute;

		JobBasePtrVectorT* currentQueue;
		JobBasePtrVectorT* queueToExecute;

	};

	class EventJob : public JobBase
	{
	public:
		EventJob(EventListenerBase* _eventListenerPtr);

		void Execute();
	protected:
	private:
		EventJob();

		EventListenerBase* eventListenerPtr;
	};

	class Worker : public ThreadObject
	{
	public:
		Worker(JobQueue* _queuePtr);

		void Notify();

		void Stop();

	protected:

	private:
		Worker();
		JobQueue* queuePtr;
		std::mutex executionNotificationMutex;
		std::unique_lock<std::mutex> executionLock;
		std::condition_variable executionNotification;

		void run();
	};

	std::mutex eventListenersAccessMutex;

	typedef std::vector<Worker*> WorkerPtrVector;

	uint32_t currentWorkerIndex;

	WorkerPtrVector workers;

	JobQueue jobQueue;

	SubscriberEventMap eventListeners;

	JobDispatcher();
	static std::mutex instanceCreationMutex;

	static JobDispatcher* instance;

};



#endif /* INC_JOBDISPATCHER_H_ */
