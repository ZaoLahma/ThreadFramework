/*
 * logjob.cpp
 *
 *  Created on: Apr 27, 2016
 *      Author: janne
 */

#include "internal/logjob.h"

std::mutex LogJob::fileAccessMutex;

LogJob::LogJob(const std::string& _stringToPrint):
stringToPrint(_stringToPrint)
{

}

void LogJob::Execute()
{
	std::unique_lock<std::mutex> fileAccessLock(fileAccessMutex);
	fileStream.open("log.txt", std::ios::app);
	fileStream<<stringToPrint;
	fileStream.close();
}
