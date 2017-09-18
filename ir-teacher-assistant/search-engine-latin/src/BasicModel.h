/*
 * BasicModel.h
 *
 *  Created on: Apr 30, 2013
 *      
 */

#ifndef BASICMODEL_H_
#define BASICMODEL_H_

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <sstream>
#include <vector>
#include <tuple>
#include "common.h"
#include "Tuple.h"
#include "StringUtil.h"
#include "MyStream.h"

enum class MODEL_ENUM {BOOLEAN=0, COSINE=1, BM25=2, MIX=3, ANCHOR_COSINE=4, ANCHOR_BM25=5, PAGERANK_MODEL=6, MIX2=7};


class BasicModel {
public:
	BasicModel(unordered_map<string, pair<ui,ui> >* pglobalWordsMap,
			   unordered_map<ui, vector<Tuple3> >* pcache,
			   vector<ul>* pwordsOffset, string prefixName="");
	virtual ~BasicModel();

	 vector<pair<float, ui> >* search(string& text);
	 virtual void getDocsWeights(string& text, unordered_map<ui, float>& docs) = 0;


protected:
	unordered_map<string, pair<ui,ui> >* globalWordsMap; //word, <id, frequency in the collection>
	unordered_map<ui, vector<Tuple3> >* cache; //wordId, Tuples; If a word is searched, its tuples are stored in cache

	vector<ul>* wordsOffset;//offsets that indicates the position in bytes of the tuples for each word
	StringUtil* stringUtil;
	Config* config;//stores the paremeters of the application
	MyStream* tuplesStream;//Stores the 3-tuples

	void extractWords(string& text, vector<string>& words){
		string isoText;
		stringUtil->convertToIso88591(text, isoText);
		stringUtil->extractWords(isoText, words);
	}

	void extractWordsIdAndFrequency(string& text, unordered_map<ui, tuple<ui, float, string> >& wordsFreq);//word id, word freq query, doc freq

	/**
	 * Reads all the tuples of one WordId from a file and stores in vector a.
	 */
	vector<Tuple3>* loadTuples(ui wordId);
};

#endif /* BASICMODEL_H_ */
