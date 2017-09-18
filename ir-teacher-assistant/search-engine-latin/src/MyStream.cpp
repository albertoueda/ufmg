/*
 * MyReader.cpp
 *
 *  Created on: Mar 21, 2013
 *      
 */

#include "MyStream.h"
#include <iostream>
#include <cassert>

const IntermediateTuple4 MyStream::END_MARK4(0u, 0u, 0u, 0u);//No tuple can have frequency 0
const Tuple3 MyStream::END_MARK3(0u, 0u, 0u);//No tuple can have frequency 0

/**
 * @path Path where the file is located
 * @isRead Indicates if this object will be use to read
 * @pmustSaveTuple3 in the last execution of the sort the 4-tuples are transformed in 3-tuples
 * @isTuple4 indicates if the object will deal with 4-tuple or 3-tuple
 */
MyStream::MyStream(string path, bool pisRead, bool pmustSaveTuple3, bool pisTuple4, string pprefixTupleName): compression(file, pisRead),
												lastTuple4(0u,0u,0u,0u), lastTuple3(0u, 0u, 0u),
												isRead(pisRead), mustSaveTuple3(pmustSaveTuple3), isTuple4(pisTuple4), prefixTupleName(pprefixTupleName)
{
	if(isRead){
		file.open(path, ios_base::in|ios::binary);
	}else{
		if(mustSaveTuple3){
			Config* config = Config::Instance();

			file.open(config->workDirPath + prefixTupleName + config->tuplesName, ios_base::out|ios::binary);
			fwordsOffset.open(config->workDirPath + prefixTupleName + config->wordsOffsetName, ios_base::out|ios::binary);
			if(fwordsOffset.fail()){
				cerr << "Fail to open file: " << config->workDirPath + prefixTupleName + config->wordsOffsetName << endl;
				assert(fwordsOffset.fail() == false);
				exit(-1);
			}
			ul offset = 0ul;
			fwordsOffset.write(reinterpret_cast<char*>(&offset), sizeof(ul));
		}else{
			file.open(path, ios_base::out|ios::binary);
		}
	}
	if(file.fail()){
		cerr << "Fail to open file: " << path << endl;
		assert(file.fail() == false);
		exit(-1);
	}
}

MyStream::~MyStream() {
	closeFile();
}

/**
 * Reads the 4-tuple using the differences with the last tuple.
 */
MyStream& MyStream::operator >>(IntermediateTuple4& tuple) {
	compression >> tuple.wordId >> tuple.docId >> tuple.wdocFrequency >> tuple.wdocPosition;
	if(!(tuple == MyStream::END_MARK4)){
		tuple.wordId += lastTuple4.wordId;
		if(tuple.wordId == lastTuple4.wordId){
			tuple.docId += lastTuple4.docId;
			if(tuple.docId == lastTuple4.docId){
				tuple.wdocFrequency += lastTuple4.wdocFrequency;
				if(tuple.wdocFrequency == lastTuple4.wdocFrequency){
					tuple.wdocPosition += lastTuple4.wdocPosition;
				}
			}
		}
	}
	lastTuple4 = tuple;
	return *this;
}

void MyStream::endWordIndice() {
	//sets the offset of the new wordId
	putEndMark();
	resetLastTuple();
	//aligning the bytes
	compression.flush();
	ul offset = compression.getNumberOfBytesWritten();
	//stores the offset in bytes of the beginning of each tuples that belongs to the word
	fwordsOffset.write(reinterpret_cast<char*>(&offset), sizeof(ul));
}

/**
 * Writes the 4-tuple using the differences with the last tuple.
 */
MyStream& MyStream::operator <<(const IntermediateTuple4& tuple) {
	if(mustSaveTuple3){
		//converts the 4-tuple in 3-tuple
		Tuple3 t = tuple;
		if(tuple.wordId != lastTuple4.wordId){
			//sets the offset of the new wordId
			endWordIndice();
		}
		(*this) << t;

		lastTuple4.wordId = tuple.wordId;
		return *this;
	}

	IntermediateTuple4 newTupple = tuple;
	//Writes the 4-tuple using the differences with the last tuple.
	if(tuple.wordId == lastTuple4.wordId){
		if(tuple.docId == lastTuple4.docId){
			if(tuple.wdocFrequency == lastTuple4.wdocFrequency){
				newTupple.wdocPosition -= lastTuple4.wdocPosition;
			}
			newTupple.wdocFrequency -= lastTuple4.wdocFrequency;
		}
		newTupple.docId -= lastTuple4.docId;
	}
	newTupple.wordId -= lastTuple4.wordId;

	compression << newTupple.wordId << newTupple.docId << newTupple.wdocFrequency << newTupple.wdocPosition;
	lastTuple4 = tuple;

	return *this;
}

/**
 * Reads the 3-tuple using the differences with the last tuple.
 */
MyStream& MyStream::operator >>(Tuple3& tuple) {
	compression >> tuple.docId >> tuple.wdocFrequency >> tuple.wdocPosition;
	if(!(tuple == MyStream::END_MARK3)){
		tuple.docId += lastTuple3.docId;
		if(tuple.docId == lastTuple3.docId){
			tuple.wdocFrequency += lastTuple3.wdocFrequency;
			if(tuple.wdocFrequency == lastTuple3.wdocFrequency){
				tuple.wdocPosition += lastTuple3.wdocPosition;
			}
		}
	}
	lastTuple3 = tuple;
	return *this;
}

/**
 * Writes the 3-tuple using the differences with the last tuple.
 */
MyStream& MyStream::operator <<(const Tuple3& tuple) {
	//static ofstream o("tuples.txt");

	Tuple3 newTupple = tuple;
	if(tuple.docId == lastTuple3.docId){
		if(tuple.wdocFrequency == lastTuple3.wdocFrequency){
			newTupple.wdocPosition -= lastTuple3.wdocPosition;
		}
		newTupple.wdocFrequency -= lastTuple3.wdocFrequency;
	}
	newTupple.docId -= lastTuple3.docId;

	//o << newTupple << endl;

	compression << newTupple.docId << newTupple.wdocFrequency << newTupple.wdocPosition;
	lastTuple3 = tuple;

	return *this;
}





