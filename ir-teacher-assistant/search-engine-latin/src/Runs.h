/*
 * OutBuffer.h
 *
 *  Created on: Mar 20, 2013
 *      
 */

#ifndef RUNS_H_
#define RUNS_H_

#include "common.h"
#include "Tuple.h"
#include "MyStream.h"
#include "AsyncWriter.h"
#include <string>
#include <vector>
#include <future>

/**
 * Gets the tuples from the HTML parser and stores then.
 * When available memory is full it sorts the tuples and write then.
 * The class AsyncWriter is responsible to controls the buffer, the memory restriction
 * and to write the data.
 */
class Runs : protected AsyncWriter<IntermediateTuple4, MyStream>{
public:
	/**
	 * @availableMemory memory in bytes available to the buffer.
	 * @pdirPath Path of the directory where the runs will be saved.
	 * @pposfix posfix of the file name of the runs.
	 * @pnthreads maximal number of threads that can be used
	 */
	Runs(ui availableMemory, string pdirPath, string pposfix, ui pnthreads) : AsyncWriter<IntermediateTuple4, MyStream>(availableMemory, pnthreads),
	dirPath(pdirPath), posfix(pposfix), countFiles(0), writter(NULL), isTheOnlyFile(false), nthreads(pnthreads)
	{
	}
	~Runs(){
	}

	/**
	 * Saves the tuple in the buffer
	 */
	void insert(IntermediateTuple4& tuple){
		(*this) <<(tuple);
	}

	/**
	 * Number of created runs
	 */
	ui getNumberOfFiles(void) const { return countFiles;}

	/**
	 * Flush the data
	 */
	void end(void){
		if(countFiles == 0){
			isTheOnlyFile = true;
		}
		AsyncWriter<IntermediateTuple4, MyStream>::end();
	}


protected:
	/**
	 * Is called in the beginning of flush method
	 */
	virtual void beforeFlush(){
		if(writter == NULL && currentBuffer){
			string t (dirPath);
			t.append(to_string(countFiles));
			t.append(posfix);
			writter = new MyStream(t, false, isTheOnlyFile);
			++countFiles;
			setWritter(writter);
		}
	}
	/**
	 * Is called in the beginning of the asynchronous written.
	 */
	virtual void startingAssinc(){
		pararellalSort(nthreads-1, *newBuffer, newBufferSize);
	}
	/**
	 * Is called in the end of the asynchronous written.
	 */
	virtual void endingAssinc(){
		if(writter != NULL){
			delete writter;
			writter = NULL;
			setWritter(NULL);
		}
	}


private:
	string dirPath; // Path of the directory where the runs will be saved.
	string posfix; //posfix of the file name of the runs.
	ui countFiles;// Number of runs created
	MyStream* writter;//Output stream
	bool isTheOnlyFile;//If it is set, the 4-tuple will be converted in 3-tuple
	ui nthreads;//Number of threads available.
};

#endif /* RUNS_H_ */
