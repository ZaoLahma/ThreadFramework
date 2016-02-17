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
#include <cinttypes>

#include "jobbase.h"
#include "eventlistenerbase.h"

//For worker
#include <vector>
#include <condition_variable>
#include "threadobject.h"

#define TIMEOUT_EVENT_ID 0x0 //Move me to appropriate place

class JobDispatcher
{
public:
	static void DropInstance();

	virtual ~JobDispatcher();

	//Main access point
	static JobDispatcher* GetApi();

	//API
	void ExecuteJob(JobBase* jobPtr);

	void ExecuteJobIn(JobBase* jobPtr, const uint32_t ms);

	void SubscribeToEvent(const uint32_t eventNo, EventListenerBase* eventListenerPtr);

	void UnsubscribeToEvent(const uint32_t eventNo, EventListenerBase* eventListenerPtr);

	void RaiseEvent(const uint32_t eventNo, const EventDataBase* eventDataPtr);

	void RaiseEventIn(const uint32_t eventNo, const EventDataBase* eventDataPtr, const uint32_t ms);

	void WaitForExecutionFinished();

	void NotifyExecutionFinished();

protected:

private:
	//Helper class definitions
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
		EventJob(EventListenerBase* _eventListenerPtr,
				 const uint32_t _eventNo,
				 const EventDataBase* _eventDataPtr);

		void Execute();
	protected:
	private:
		EventJob();

		EventListenerBase* eventListenerPtr;

		const uint32_t eventNo;

		EventDataBase* eventDataPtr;
	};

	class Worker : public ThreadObject
	{
	public:
		Worker(JobQueue* _queuePtr);

		void Notify();

		bool IsIdling();

		void Stop();

	protected:

	private:
		Worker();
		JobQueue* queuePtr;
		std::mutex executionNotificationMutex;
		std::unique_lock<std::mutex> executionLock;
		std::condition_variable executionNotification;

		std::atomic<bool> isIdling;

		void run();
	};

	class TimerEventData : public EventDataBase
	{
	public:
		TimerEventData(const uint32_t);

		EventDataBase* clone() const;

		uint32_t GetTimerId() const;

	protected:

	private:
		const uint32_t timerId;
		TimerEventData();
	};

	class TimerBase : public ThreadObject
	{
	public:
		TimerBase(const uint32_t _ms);
		virtual ~TimerBase() {}

		void SetTimerId(const uint32_t _timerId);

		void run();

		virtual void TimerFunction() = 0;

	protected:
		const uint32_t ms;
		uint32_t timerId;

	private:
		TimerBase();
	};

	typedef std::map<uint32_t, TimerBase*> TimerBaseMap;

	class JobTimer : public TimerBase
	{
	public:
		JobTimer(JobBase* _jobPtr, const uint32_t _ms);

		void TimerFunction();

	protected:

	private:
		JobTimer();
		JobBase* jobPtr;
	};

	class EventTimer : public TimerBase
	{
	public:
		EventTimer(const uint32_t eventNo, const EventDataBase* _dataPtr, const uint32_t _ms);
		void TimerFunction();

	protected:

	private:
		EventTimer();
		const uint32_t eventNo;
		const EventDataBase* eventDataPtr;
	};

	class TimerStorage : public EventListenerBase
	{
	public:
		TimerStorage();
		~TimerStorage();
		void StoreTimer(TimerBase* _timer);
		void HandleEvent(const uint32_t _eventNo, const EventDataBase* _dataPtr);

	protected:

	private:
		bool subscribedToEvent;
		std::mutex subscribeMutex;
		TimerBaseMap timers;
		std::mutex timerMutex;
	};

	//Private attrbutes
	uint32_t noOfCores;

	std::mutex workerCreationMutex;

	std::mutex eventListenersAccessMutex;

	std::mutex executionFinishedNotificationMutex;
	std::unique_lock<std::mutex> executionFinishedNotificationLock;
	std::condition_variable executionFinishedNotification;

	typedef std::vector<Worker*> WorkerPtrVector;

	WorkerPtrVector workers;

	JobQueue jobQueue;

	EventListenersMap eventListeners;

	TimerStorage timerStorage;

	JobDispatcher();
	static std::mutex instanceCreationMutex;
	static JobDispatcher* instance;
};



#endif /* INC_JOBDISPATCHER_H_ */
