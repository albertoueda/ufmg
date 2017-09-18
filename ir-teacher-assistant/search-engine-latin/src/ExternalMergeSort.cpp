/*
 * ExternalMergeSort.cpp
 *
 *  Created on: Mar 21, 2013
 *      
 */

#include "ExternalMergeSort.h"
#include <queue>
#include <iostream>
#include <cmath>
#include <cstdio>


/**
 * @pnumberOfFilesToSort number of runs files to sort.
 */
ExternalMergeSort::ExternalMergeSort(ui pnumberOfFilesToSort): numberOfFilesToSort(pnumberOfFilesToSort),
lastSortedFile(0u), isLastPassed(false),actualPassed(0u)
{
	config = Config::Instance();
	//Estimates the memory used by the Heap in bytes.
	ui filesMemory = sizeof(pair<MyStream, ui>) * config->maxFiles2Sort;
	if(filesMemory >= config->availableMemory){
		cerr << "Out of memory to simulate!" << endl;
		exit(-1);
	}
	ui memoryToBuffer = config->availableMemory - filesMemory;
	asyncWritter = new AsyncWriter<IntermediateTuple4, MyStream>(memoryToBuffer, config->maxThreads);
}

ExternalMergeSort::~ExternalMergeSort() {
	delete asyncWritter;
}

/**
 * Merges the run's files and store the final 3-tuple in log_{maxFiles2Sort}(numberOfFilesToSort).
 */
void ExternalMergeSort::sort(void) {
	while(numberOfFilesToSort > 1){
		if(config->maxFiles2Sort >= numberOfFilesToSort){
			//last execution of the loop
			isLastPassed = true;
		}
		ui newFiles = 0;
		++actualPassed;
		string newPosfix = config->runsPosfix + to_string(actualPassed);
		//Calculates the number of files that will be generate in this execution
		ui npasses = ceil(static_cast<float>(numberOfFilesToSort)/config->maxFiles2Sort);
		ui shift = 0;
		for(ui i = 0; i < npasses; ++i){
			string path = config->workDirPath + to_string(newFiles) + newPosfix;
			MyStream writer(path, false, isLastPassed);
			asyncWritter->setWritter(&writer);
			ui beginFile = min(shift, numberOfFilesToSort-1);
			ui endFile =  min(shift+config->maxFiles2Sort-1, numberOfFilesToSort-1);
			partialSort(beginFile, endFile);//Merge
			++newFiles;
			shift+= config->maxFiles2Sort;
			asyncWritter->end();
		}
		numberOfFilesToSort = newFiles;
		cout << "END ROUND " << actualPassed << endl;
	}

	cout << "NUMBER OF ROUNDS TO SORT: " << actualPassed << endl;
}

/**
 * Merges the files in the interval [startFile, endFile) in one file
 */
void ExternalMergeSort::partialSort(ui startFile, ui endFile) {
	Heap heap;
	ui nfiles = endFile-startFile + 1;
	vector<MyStream* > filesToRead;
	filesToRead.reserve(nfiles);
	string p = config->runsPosfix + to_string(actualPassed-1);
	vector<string> filesToRemove;//Stores the name of the temporary file to be removed

	//Initializes the heap with one tuple from each input file.
	for(ui i = 0; i < nfiles; ++i){
		string f = config->workDirPath + to_string(startFile+i)+p;
		filesToRemove.push_back(f);
		//cout << "file name " << f << endl;
		filesToRead.push_back(new MyStream(f, true));
		read(filesToRead[i], i, heap);
	}

	while(!heap.empty()){
	    pair<IntermediateTuple4, ui>& p = const_cast< pair<IntermediateTuple4, ui>&>(heap.top());
	    (*asyncWritter) << p.first;
		ui id = p.second;
		heap.pop();

		//search for the first input file that has tuples to be read.
		for(ui i = 0; i < nfiles; ++i){
			ui idRead = (id + i) % nfiles;
			if(filesToRead[idRead]->isOpen() ){
				//reads one tuple
				read(filesToRead[idRead], idRead, heap);
				break;
			}
		}
	}
	for(MyStream* p : filesToRead){
		delete p;
	}
	//Removes the temporary files of the runs
	for(string& f : filesToRemove){
		if(remove(f.c_str()) != 0){
			cerr << "Fail to remove the file: " << f << endl;
		}
	}
}






