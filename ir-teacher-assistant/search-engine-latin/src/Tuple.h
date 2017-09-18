/*
 * Tuple.h
 *
 *  Created on: Mar 19, 2013
 *      
 */

#ifndef TUPLE_H_
#define TUPLE_H_

#include "common.h"
#include <fstream>
#include <queue>

//This class not inheritance Tuple3 because of performance
/**
 * 4-Tuple used in the intermediate sort phases
 */
class IntermediateTuple4 {
public:
	ui wordId;//Id of the word
	ui docId;//Id of the document
	ui wdocFrequency;//Frequency of the word in the document
	ui wdocPosition;//Position of the word in the document

	explicit IntermediateTuple4(): wordId(0ul), docId(0ul), wdocFrequency(0l), wdocPosition(0l){}
	IntermediateTuple4(ui pwordId, ui pdocId, ui pwdocFrequency, ui pwdocPosition):
		wordId(pwordId), docId(pdocId), wdocFrequency(pwdocFrequency), wdocPosition(pwdocPosition){}
	IntermediateTuple4(const IntermediateTuple4 & rhs) :
		wordId(rhs.wordId), docId(rhs.docId), wdocFrequency(rhs.wdocFrequency), wdocPosition(rhs.wdocPosition){}

	IntermediateTuple4& operator=(const IntermediateTuple4& rhs){
		this->wordId = rhs.wordId;
		this->docId = rhs.docId;
		this->wdocFrequency = rhs.wdocFrequency;
		this->wdocPosition = rhs.wdocPosition;
		return *this;
	}

     bool operator <(const IntermediateTuple4& rhs) const
     {
        if(this->wordId == rhs.wordId){
        	if(this->docId == rhs.docId){
        		if(this->wdocFrequency == rhs.wdocFrequency){
        			return this->wdocPosition < rhs.wdocPosition;
        		}
        		return this->wdocFrequency < rhs.wdocFrequency;
        	}
        	return this->docId < rhs.docId;
        }

    	return this->wordId < rhs.wordId;
     }


    friend bool operator ==(const IntermediateTuple4& lhs, const IntermediateTuple4& rhs)
	 {
		if(lhs.wordId != rhs.wordId || lhs.docId != rhs.docId ||
		   lhs.wdocFrequency != rhs.wdocFrequency || lhs.wdocPosition != rhs.wdocPosition){
			return false;
		}

		return true;
	 }
	friend ostream& operator<< (ostream &out, const IntermediateTuple4 &tuple){
		out << tuple.wordId << " " << tuple.docId << " " << tuple.wdocFrequency << " " << tuple.wdocPosition;
		return out;
	}

	friend istream& operator>> (istream &in, IntermediateTuple4 &tuple){
		in >> tuple.wordId >> tuple.docId >> tuple.wdocFrequency >> tuple.wdocPosition;
		return in;
	}

};

//This comparator is used in head sort
class __Greater {
public:
	bool operator()(const pair<IntermediateTuple4, ui>  &lhs, const pair<IntermediateTuple4, ui>  &rhs) const{
	   if(lhs.first.wordId == rhs.first.wordId){
			if(lhs.first.docId == rhs.first.docId){
				if(lhs.first.wdocFrequency == rhs.first.wdocFrequency){
					return lhs.first.wdocPosition > rhs.first.wdocPosition;
				}
				return lhs.first.wdocFrequency > rhs.first.wdocFrequency;
			}
			return lhs.first.docId > rhs.first.docId;
		}

		return lhs.first.wordId > rhs.first.wordId;
	}
};

/**
 * Final 3-tuple used in the index file
 */
class Tuple3{
public:
	ui docId;//Document id
	ui wdocFrequency;//Frequency of the word in the document
	ui wdocPosition;//Position of the word in the document

	explicit Tuple3(): docId(0ul), wdocFrequency(0l), wdocPosition(0l){}
	Tuple3(ui pdocId, ui pwdocFrequency, ui pwdocPosition):
		   docId(pdocId), wdocFrequency(pwdocFrequency), wdocPosition(pwdocPosition){}
	Tuple3(const Tuple3 & rhs) :
		     docId(rhs.docId), wdocFrequency(rhs.wdocFrequency), wdocPosition(rhs.wdocPosition){}

	Tuple3(const IntermediateTuple4 & rhs) :
		     docId(rhs.docId), wdocFrequency(rhs.wdocFrequency), wdocPosition(rhs.wdocPosition){}

	Tuple3& operator=(const Tuple3& rhs){
		this->docId = rhs.docId;
		this->wdocFrequency = rhs.wdocFrequency;
		this->wdocPosition = rhs.wdocPosition;
		return *this;
	}
	Tuple3& operator=(const IntermediateTuple4& rhs){
		this->docId = rhs.docId;
		this->wdocFrequency = rhs.wdocFrequency;
		this->wdocPosition = rhs.wdocPosition;
		return *this;
	}

     bool operator <(const Tuple3& rhs) const
     {
        	if(this->docId == rhs.docId){
        		if(this->wdocFrequency == rhs.wdocFrequency){
        			return this->wdocPosition < rhs.wdocPosition;
        		}
        		return this->wdocFrequency < rhs.wdocFrequency;
        	}
        	return this->docId < rhs.docId;
     }


    friend bool operator ==(const Tuple3& lhs, const Tuple3& rhs)
	 {
		if(lhs.docId != rhs.docId || lhs.wdocFrequency != rhs.wdocFrequency || lhs.wdocPosition != rhs.wdocPosition){
			return false;
		}

		return true;
	 }
	friend ostream& operator<< (ostream &out, const Tuple3 &tuple){
		out << tuple.docId << " " << tuple.wdocFrequency << " " << tuple.wdocPosition;
		return out;
	}

	friend istream& operator>> (istream &in, Tuple3 &tuple){
		in  >> tuple.docId >> tuple.wdocFrequency >> tuple.wdocPosition;
		return in;
	}
};


#endif /* TUPLE_H_ */
