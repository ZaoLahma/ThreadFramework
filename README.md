# MultiThreadEventFramework
A multi threaded event and job execution framework with an easy interface which can be used for parallell programming

The framework was developed with one goal in mind; everything it does should be done in parallel. For the most part, it does. However, scheduling of jobs and raising events is protected by guard locks to ensure thread safety.

The amount of worker threads will automatically be adjusted for the worst case: 
If no idling worker thread is found, a new one is created. This means it's perfectly fine to let a job execution take time as that will not be affecting the execution of other jobs.
