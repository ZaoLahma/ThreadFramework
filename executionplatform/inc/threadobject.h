#ifndef HEADER_THREADOBJECT
#define HEADER_THREADOBJECT

#include <thread>
#include <atomic>

class ThreadObject
{
	private:
	
	std::thread executorThread;
	
	virtual void run() = 0;
	
	protected:
	
	std::atomic<bool> running;
	
	public:
	
	ThreadObject();
	virtual ~ThreadObject();
	
	virtual void start();
	
	virtual void stop();
};

#endif
