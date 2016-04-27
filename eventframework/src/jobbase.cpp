/*
 * job.cpp
 *
 *  Created on: Feb 3, 2016
 *      Author: janne
 */

#include "jobbase.h"

JobBase::~JobBase()
{

}

void JobBase::SetJobData(JobDataBase* _dataPtr)
{
	dataPtr = _dataPtr;
}
