/*
 * ExternalMergeSort.h
 *
 *  Created on: Mar 21, 2013
 *      
 */

#ifndef EXTERNALMERGESORT_H_
#define EXTERNALMERGESORT_H_

#include "common.h"
#include "MyStream.h"
#include "AsyncWriter.h"
#include <string>
#include <future>


//Definition of the Heap type. Each object in this heap is a pair that the first member
//stores the 4-tuple and the second member stores the id of the origin file of the 4-tuple.
//The functor __Greater is used to maintain the Heap in decreasing order.
typedef priority_queue<pair<IntermediateTuple4, ui>, vector<pair<IntermediateTuple4, ui>>, __Greater > Heap;



/**
 * Merges the run's files and store the final 3-tuple.
 */
class ExternalMergeSort {
public:
	/**
	 * @pnumberOfFilesToSort number of runs files to sort.
	 */
	ExternalMergeSort(ui pnumberOfFilesToSort);
	~ExternalMergeSort();

	/**
	 * Merges the run's files and store the final 3-tuple in log_{maxFiles2Sort}(numberOfFilesToSort).
	 */
	void sort(void);

private:
	ui numberOfFilesToSort;//Number of files to merge
	ui lastSortedFile;//Id of the last sorted file
	bool isLastPassed;//Indicates if it is the last execution of the loop
	ui actualPassed;//Indicates the number of the execution
	AsyncWriter<IntermediateTuple4, MyStream> *asyncWritter;//Used to writes the results asynchronously
	Config* config;//Stores the configuration of the application

	/**
	 * Merges the files in the interval [startFile, endFile) in one file
	 */
	void partialSort(ui startFile, ui endFile);

	/**
	 * Reads one tuple and stores it in heap.
	 * @reader the stream where the tuple will be read.
	 * @id the identifier of the stream.
	 */
	void  read(MyStream* reader, ui id, Heap& heap) {
		IntermediateTuple4 tuple;
		(*reader) >> tuple;
		if(tuple == MyStream::END_MARK4){
			reader->closeFile();
			return;
		}
		heap.push(pair<IntermediateTuple4, ui>(tuple, id));
	}


};

#endif /* EXTERNALMERGESORT_H_ */
