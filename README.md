# MultiThreadEventFramework
WHAT?
<br>
A C++11 (should compile for Win and *NIX) multi threaded event and job execution framework with an easy interface which can be used for parallell programming. The framework is still under development.

HOW?
<br>
Simply put, a number of worker threads will execute jobs which they take from a queue. The threads can be grouped together so that for example one worker thread executes jobs from one job queue, while three worker threads execute jobs from another job queue.

EXAMPLES PLEASE?
<br>
Righteo.<br>
<br>
Create exec group (a number of threads executing jobs from the same queue):<br>
const uint32_t PRIO_EXEC_GROUP = 1;<br>
uint32_t maxNoOfThreads = 2;<br>
JobDispatcher::GetApi()->AddExecGroup(PRIO_EXEC_GROUP, maxNoOfThreads);
<br>
<br>
Schedule a job in a group:<br>
std::shared_ptr<JobBase> job = std::make_shared\<Job>();<br>
JobDispatcher::GetApi()->ExecuteJobInGroup(job, PRIO_EXEC_GROUP);
<br>
<br>
Subscribe to an event:<br>
uint32_t EVENT_NO = 10;<br>
JobDispatcher::GetApi()->SubscribeToEvent(EVENT_NO, this);
<br>
<br>
Raise an event:<br>
uint32_t EVENT_NO = 10;<br>
std::shared_ptr<EventData> eventData = std::make_shared\<EventData>();<br>
JobDispatcher::GetApi()->RaiseEvent(EVENT_NO, eventData);
<br>
<br>
Still confused? Have a look at the test of the framework where it's used for finding words in strings in parallel.<br>
https://github.com/ZaoLahma/ThreadFramework/tree/master/test
