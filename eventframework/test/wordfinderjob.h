/*
 * wordfinderjob.h
 *
 *  Created on: Feb 8, 2016
 *      Author: janne
 */

#ifndef TEST_WORDFINDERJOB_H_
#define TEST_WORDFINDERJOB_H_

#include "jobbase.h"
#include "eventlistenerbase.h"

#include <string>


#define WORD_FINDER_JOB_FINISHED 0x00000001

class WordFinderData : public JobDataBase
{
public:
	WordFinderData(const std::string& _wordString,
			       const std::string& _wordToFind);

	const std::string wordString;
	const std::string wordToFind;
	uint32_t noOfOccurances;

private:
	WordFinderData();
};

class WordFinderJob : public JobBase
{
public:
	WordFinderJob(const uint32_t _instanceNo);

	void Execute();
protected:

private:
	WordFinderJob();

	const uint32_t instanceNo;

};


#endif /* TEST_WORDFINDERJOB_H_ */
