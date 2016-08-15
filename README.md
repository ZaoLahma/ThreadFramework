# MultiThreadEventFramework
A C++11 (should compile for Win and *NIX) multi threaded event and job execution framework with an easy interface which can be used for parallell programming

The framework was developed with one goal in mind; everything it does should be done in parallel. For the most part, it does. However, scheduling of jobs and raising events is protected by guard locks to ensure thread safety.

The amount of worker threads will automatically be adjusted for the worst case as follows: <br>
If no idling worker thread is found when a job is scheduled, a new one is created. This means it's perfectly fine to let a job take time as that will not be affecting the execution of other jobs.
<br>
<br>
Currently there's no mechanism removing workers when they are no longer needed. Not sure if such functionality will be included in the future.
<br>
<br>
Check out my https://github.com/ZaoLahma/simple2game project to see how the framework can be used.
