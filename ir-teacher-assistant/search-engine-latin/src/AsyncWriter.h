/*
 * AsyncWriter.h
 *
 *  Created on: Apr 5, 2013
 *      
 */

#ifndef ASYNCWRITER_H_
#define ASYNCWRITER_H_

#include "common.h"
#include <future>

/**
 * This class is used to write asynchronous. The data of the client is stored in a buffer
 * and when this buffer gets full, this data is copied to another buffer and one thread is
 * created to write the old buffer.
 * @Tbuffer type of the data
 * @Tostream type of the class that the results will be written
 */
template<typename Tbuffer, typename Tostream> class AsyncWriter{
	public:

		/**
		 * Splits the availableMemory between the two buffers: one that the client
		 * is writing and other that is used to write asynchronous.
		 *
		 * @availableMemory memory available to user in the buffer in bytes
		 */
		AsyncWriter(ui availableMemory, ui maxThread) : currentBufferSize(0), newBufferSize(0), currentBuffer(0), waitingResult(false){
			capacity = availableMemory/sizeof(Tbuffer)/2; //Number of objects that fits in buffer
			buffer = new vector<Tbuffer>();
			newBuffer = new vector<Tbuffer>();
			buffer->reserve(capacity);
			newBuffer->reserve(capacity);
			(*buffer) = vector<Tbuffer>(capacity, Tbuffer());
			(*newBuffer) = vector<Tbuffer>(capacity, Tbuffer());
			launchMode = maxThread > 1 ? launch::async : launch::deferred;
		}
		virtual ~AsyncWriter(){
			delete buffer;
			delete newBuffer;
		}

		/**
		 * Stores data in buffer.
		 */
		AsyncWriter& operator<<(Tbuffer &data){
			(*buffer)[currentBuffer++] = data;
			if(currentBuffer == capacity){
				flush();
			}
			return *this;
		}

		/*
		 * Writes all the data to the target output class.
		 */
		 void end(void){
			flush();
			if(waitingResult){
				result.get();
				waitingResult = false;
			}
		}

		 /**
		  * Sets the target output object.
		  */
		void setWritter(Tostream* pwritter){
			writter = pwritter;
		}

	protected:
		Tostream* writter;
		ui capacity;
		ui currentBufferSize;
		ui newBufferSize;
		ui currentBuffer;
		vector<Tbuffer>* buffer;
		vector<Tbuffer>* newBuffer;
		future<void> result;
		bool waitingResult;
		std::launch launchMode;
		/**
		 * Is called in the beginning of flush method
		 */
		virtual void beforeFlush(){}
		/**
		 * Is called in the beginning of the asynchronous written.
		 */
		virtual void startingAssinc(){}

		/**
		 * Is called in the end of the asynchronous written.
		 */
		virtual void endingAssinc(){}

		/*
		 * Writes all the data in the buffer to the target output class.
		 */
		void flush(void){
			//If the last thread doesn't finish to write the data, wait.
			if(waitingResult){
				result.get();//Ask for the last thread to write the data.
				waitingResult = false;
			}

			beforeFlush();

			if(currentBuffer){
				waitingResult = true;
				swap(buffer, newBuffer);
				newBufferSize = currentBuffer;
				//starts a new thread asynchronously or synchronously  depending on the launch mode
				result = async(launchMode, [&,this](){
					this->startingAssinc();
					for(ui i = 0; i < newBufferSize; ++i){
						(*writter) << (*newBuffer)[i];
					}
					this->endingAssinc();
				});
			}

			currentBuffer = 0;
		}

	};


#endif /* ASYNCWRITER_H_ */
