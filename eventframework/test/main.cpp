/*
 * main.cpp
 *
 *  Created on: Feb 3, 2016
 *      Author: janne
 */

#include "jobdispatcher.h"
#include "jobbase.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

class TestJob : public JobBase
{
public:
	void Execute()
	{
		noOfExecutions++;
	}

	static std::atomic<uint32_t> noOfExecutions;
};

std::atomic<uint32_t> TestJob::noOfExecutions;

class TestEventListener : public EventListenerBase
{
public:
	void HandleEvent()
	{
		for(unsigned int i = 0; i < 10; ++i)
		{
			TestJob* jobPtr = new TestJob();
			JobDispatcher::GetApi()->ExecuteJob(jobPtr);
		}
	}
};

int main(void)
{
	TestEventListener testEventListener;
	JobDispatcher::GetApi()->SubscribeToEvent(0x0, &testEventListener);

	JobDispatcher::GetApi()->RaiseEvent(0x0);

	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	JobDispatcher::GetApi()->RaiseEvent(0x0);

	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	std::cout<<"Unsubscribed to events"<<std::endl;

	JobDispatcher::GetApi()->UnsubscribeToEvent(0x0, &testEventListener);

	JobDispatcher::GetApi()->RaiseEvent(0x0);

	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	std::cout<<"TestJob::noOfExecutions: "<<TestJob::noOfExecutions<<std::endl;

	JobDispatcher::DropInstance();
}
