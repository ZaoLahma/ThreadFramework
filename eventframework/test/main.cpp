/*
 * main.cpp
 *
 *  Created on: Feb 3, 2016
 *      Author: janne
 */

#include "wordfinder.h"
#include <iostream>
#include "jobbase.h"

class EndExecutionJob : public JobBase
{
public:
	void Execute()
	{
		JobDispatcher::GetApi()->NotifyExecutionFinished();
	}
};

int main(void)
{
	JobDispatcher::GetApi()->Log("This is a test log entry with an int: %d", 98);

	/*
	 * In this example instanceNo is used to make sure each WordFinder instance
	 * will raise its own unique event.
	 */
	uint32_t instanceNo = 0;

	WordFinder wf_1("this is a test string where we try to find the word test",
			        "test",
					instanceNo++);

	WordFinder wf_2("here we try to find the word word for the sake of finding word",
			        "word",
					instanceNo++);

	WordFinder wf_3("Another semi random sentence just to get a semi parallel execution. Why not, I must ask.",
			        "sentence",
					instanceNo++);

	WordFinder wf_4("There are many traps when doing things in parallel. Doing things in parallel can be a very quick way of making things go wrong, in parallel.",
			        "parallel",
					instanceNo++);

	/*
	 * Just as an example I will let this particular program be ended by a
	 * job that will execute in 1600ms. This should leave plenty of time for
	 * the framework to process the word finding jobs and tests the framework's
	 * ability to handle long running timers...
	 *
	 * Normally the end of the execution would be triggered by an external
	 * event for example triggered by a window manager
	 */
	EndExecutionJob* endExecutionJobPtr = new EndExecutionJob();
	const uint32_t msToSleep = 1600;
	JobDispatcher::GetApi()->ExecuteJobIn(endExecutionJobPtr, msToSleep);

	/*
	 * Give up control of the execution to the job that will finish it
	 * by calling this blocking function
	 */
	JobDispatcher::GetApi()->WaitForExecutionFinished();


	/*
	 * Just to clean up
	 */
	JobDispatcher::DropInstance();
}
