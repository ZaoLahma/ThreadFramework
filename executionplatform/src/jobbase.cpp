/*
 * job.cpp
 *
 *  Created on: Feb 3, 2016
 *      Author: janne
 */

#include "../inc/jobbase.h"

JobBase::~JobBase()
{

}

void JobBase::SetJobData(JobDataBase* _dataPtr)
{
	dataPtr = _dataPtr;
}