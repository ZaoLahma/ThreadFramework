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
#include <map>

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

	void ExecuteJobIn(JobBase* jobPtr, const uint32_t ms);

	void RaiseEventIn(const uint32_t eventNo, const uint32_t ms); //TODO: Implement

	void SubscribeToEvent(const uint32_t eventNo, EventListenerBase* eventListenerPtr);

	void UnsubscribeToEvent(const uint32_t eventNo, EventListenerBase* eventListenerPtr);

	void RaiseEvent(const uint32_t eventNo);

	void WaitForExecutionFinished();

	void NotifyExecutionFinished();

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
		EventJob(EventListenerBase* _eventListenerPtr, const uint32_t _eventNo);

		void Execute();
	protected:
	private:
		EventJob();

		EventListenerBase* eventListenerPtr;

		const uint32_t eventNo;
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

	class TimerBase : public ThreadObject
	{
	public:
		virtual ~TimerBase() {}

		void SetTimerId(const uint32_t _timerId)
		{
			timerId = _timerId;
		}

	protected:
		uint32_t timerId;
	};

	typedef std::map<uint32_t, TimerBase*> TimerBaseMap;

	class JobTimer : public TimerBase
	{
	public:
		JobTimer(JobBase* _jobPtr, const uint32_t _ms);

		void run();

	protected:

	private:
		const uint32_t ms;
		JobBase* jobPtr;
	};

	class TimerStorage : public EventListenerBase
	{
	public:
		TimerStorage();
		void StoreTimer(TimerBase* _timer);
		void HandleEvent(const uint32_t _eventNo);

	protected:

	private:
		uint32_t idBase;
		uint32_t currentId;
		TimerBaseMap timers;
		std::mutex timerMutex;
	};

	std::mutex eventListenersAccessMutex;

	std::mutex executionFinishedNotificationMutex;
	std::unique_lock<std::mutex> executionFinishedNotificationLock;
	std::condition_variable executionFinishedNotification;

	typedef std::vector<Worker*> WorkerPtrVector;

	uint32_t currentWorkerIndex;

	WorkerPtrVector workers;

	JobQueue jobQueue;

	EventListenersMap eventListeners;

	TimerStorage timerStorage;

	JobDispatcher();
	static std::mutex instanceCreationMutex;
	static JobDispatcher* instance;

};



#endif /* INC_JOBDISPATCHER_H_ */
