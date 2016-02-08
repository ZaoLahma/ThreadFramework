/*
 * main.cpp
 *
 *  Created on: Feb 3, 2016
 *      Author: janne
 */

#include "jobdispatcher.h"
#include "wordfinder.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

int main(void)
{
	WordFinder wf_1("this is a test string where we try to find the word test", "test", 0);

	WordFinder wf_2("here we try to find the word word for the sake of finding word", "word", 1);

	/*
	 * Sleep a bit to allow the framework perform its job.
	 * Normally there would be a blocking call here to keep things running
	 */
	std::this_thread::sleep_for(std::chrono::milliseconds(20));
}
