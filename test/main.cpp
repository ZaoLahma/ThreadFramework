/*
 * main.cpp
 *
 *  Created on: Feb 3, 2016
 *      Author: janne
 */

#include "wordfinder.h"
#include <iostream>
#include "jobbase.h"
#include "exec_groups.h"

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
	 * Limit the default exec group to 1 thread.
	 */
	JobDispatcher::GetApi()->AddExecGroup(DEFAULT_EXEC_GROUP_ID, 1);

	/*
	 * Create a "high prio" exec group with a maximum of 2 threads
	 */
	JobDispatcher::GetApi()->AddExecGroup(HIGH_PRIO_EXEC_GROUP, 2);

	/*
	 * Create a "low prio" exec group with a maximum of 1 thread
	 */
	JobDispatcher::GetApi()->AddExecGroup(LOW_PRIO_EXEC_GROUP, 1);


	/*
	 * This group is used to test executing a job in a group after
	 * a certain time.
	 */
	JobDispatcher::GetApi()->AddExecGroup(EXIT_JOB_EXEC_GROUP, 1);

	/*
	 * In this example instanceNo is used to make sure each WordFinder instance
	 * will raise its own unique event.
	 *
	 * The jobs related to finding the words are executed in the WordFinder
	 * constructor.
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

	WordFinder wf_5("Indeed, this wouldn't be much of a test case without more words to find!",
			        "Indeed",
					instanceNo++);

	WordFinder wf_6("Programming is heaven and hell, strangely at the same time.",
			        "heaven",
					instanceNo++);

	WordFinder wf_7("There are many ways in which one can make mistakes while programming. Tests must be written.",
			        "mistakes",
					instanceNo++);

	/*
	 * Just as an example I will let this particular program be ended by a
	 * job that will execute in 300ms. This should leave plenty of time for
	 * the framework to process the word finding jobs...
	 *
	 * Normally the end of the execution would be triggered by an external
	 * event for example triggered by a window manager
	 */
	EndExecutionJob* endExecutionJobPtr = new EndExecutionJob();
	const uint32_t msToSleep = 300;
	JobDispatcher::GetApi()->ExecuteJobInGroupIn(endExecutionJobPtr, EXIT_JOB_EXEC_GROUP, msToSleep);

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
