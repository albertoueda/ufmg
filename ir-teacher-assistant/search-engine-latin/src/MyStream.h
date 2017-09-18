/*
 * MyStream.h
 *
 *  Created on: Mar 21, 2013
 *      
 */

#ifndef MYSTREAM_H_
#define MYSTREAM_H_

#include "common.h"
#include <fstream>
#include <string>
#include <queue>
#include "Tuple.h"
#include "HeliasDeltaCompression.h"

/**
 * This class handles the input and output of the tuples and the compression.
 * Because the compression uses a variable number of bits and this
 * bits can not be aligned we use a tuple to signals the end of file.
 */
class MyStream {
public:
	static const IntermediateTuple4 END_MARK4;//this is used to mark the end of file
	static const Tuple3 END_MARK3;//this is used to mark the end of file


	/**
	 * @path Path where the file is located
	 * @isRead Indicates if this object will be use to read
	 * @pmustSaveTuple3 in the last execution of the sort the 4-tuples are transformed in 3-tuples
	 * @isTuple4 indicates if the object will deal with 4-tuple or 3-tuple
	 */
	MyStream(string  path, bool isRead, bool pmustSaveTuple3=false, bool isTuple4=true, string pprefixTupleName="");
    ~MyStream();

    /**
     *  Sets the position of the next character to be extracted from the input stream.
     */
    void seekg(ul offset){
    	file.seekg(offset, ios::beg);
    	reset();
    }

    /**
     * Because the compression uses a variable number of bits and this
     * bits can not be aligned we use a tuple to signals the end of file.
     */
    void putEndMark(void){
    	if(isTuple4 && !mustSaveTuple3){
    		compression << MyStream::END_MARK4.wordId << MyStream::END_MARK4.docId;
    		compression << MyStream::END_MARK4.wdocFrequency << MyStream::END_MARK4.wdocPosition;
		}else{
			compression  << MyStream::END_MARK3.docId << MyStream::END_MARK3.wdocFrequency << MyStream::END_MARK3.wdocPosition;
		}
    }

    void closeFile(void){
    	if(file.is_open()){
    		if(!isRead){
    			putEndMark();
    		}
    		if(mustSaveTuple3){
    			fwordsOffset.close();
    		}
    		compression.flush();
    		file.close();
    	}
    }

    bool isOpen(void) const{
    	return file.is_open();
    }

    /**
     * Resets the variables to the state of construction.
     */
    void reset(void){
    	compression.reset();
    	resetLastTuple();
    }

    void resetLastTuple(void){
    	lastTuple4.docId = /*lastTuple4.wordId =*/ lastTuple4.wdocFrequency = lastTuple4.wdocPosition = 0;
    	lastTuple3 = lastTuple4;
    }

    void endWordIndice();

    /**
     * Reads the 4-tuple using the differences with the last tuple.
     */
    MyStream& operator>>(IntermediateTuple4& tuple);
    /**
     * Writes the 4-tuple using the differences with the last tuple.
     */
    MyStream& operator<<(const IntermediateTuple4& tuple);

    /**
     * Reads the 3-tuple using the differences with the last tuple.
     */
    MyStream& operator>>(Tuple3& tuple);
    /**
     * Writes the 3-tuple using the differences with the last tuple.
     */
    MyStream& operator<<(const Tuple3& tuple);

	private:
    fstream file; //File that has the tuples to  be read/written.
    ofstream fwordsOffset;//If the conversion from the 4-tuples to 3-tuples is made, this file stores the offset in bytes of the beginning of each tuples that belongs to the word

    HeliasDeltaCompression compression;//Handles the compression by number
    IntermediateTuple4 lastTuple4;//Last tuple read/written
    Tuple3 lastTuple3;//Last tuple read/written
    bool isRead;//Indicates if this object will be use to read
    bool mustSaveTuple3;//in the last execution of the sort the 4-tuples are transformed in 3-tuples
    bool isTuple4;//indicates if the object will deal with 4-tuple or 3-tuple
    string prefixTupleName;

};

#endif /* MYSTREAM_H_ */
