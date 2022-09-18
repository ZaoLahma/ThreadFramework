/*
 * uniqueidprovider.cpp
 *
 *  Created on: Feb 16, 2016
 *      Author: janne
 */
#include "uniqueidprovider.h"

UniqueIdProvider* UniqueIdProvider::instance = nullptr;
std::mutex UniqueIdProvider::instanceCreationMutex;


UniqueIdProvider::UniqueIdProvider() :
currentId(0)
{

}

UniqueIdProvider* UniqueIdProvider::GetApi()
{
	if(nullptr == instance)
	{
		instanceCreationMutex.lock();
		if(nullptr == instance)
		{
			instance = new UniqueIdProvider();
		}
		instanceCreationMutex.unlock();
	}

	return instance;
}

void UniqueIdProvider::DropInstance()
{
	if(nullptr != instance)
	{
		instanceCreationMutex.lock();
		if(nullptr != instance)
		{
			delete instance;
			instance = nullptr;
		}
		instanceCreationMutex.unlock();
	}
}

uint32_t UniqueIdProvider::GetUniqueId()
{
	std::unique_lock<std::mutex> getIdLock(getIdMutex);
	return currentId++;
}
