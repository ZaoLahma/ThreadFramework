/*
 * main.cpp
 *
 *  Created on: Feb 3, 2016
 *      Author: janne
 */

#include "wordfinder.h"

int main(void)
{
	uint32_t instanceNo = 0;

	WordFinder wf_1("this is a test string where we try to find the word test",
			        "test",
					instanceNo++);

	WordFinder wf_2("here we try to find the word word for the sake of finding word",
			        "word",
					instanceNo++);

	WordFinder wf_3("Another semi random sentence just to get a semi parallel execution. Why not, I must ask.",
			        "sentence",
					instanceNo++);

	WordFinder wf_4("There are many traps when doing things in parallel. Doing things in parallel can be a very quick way of making things go wrong, in parallel.",
			        "parallel",
					instanceNo++);

	JobDispatcher::DropInstance();
}
