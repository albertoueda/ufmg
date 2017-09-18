/*
 * BasicModel.cpp
 *
 *  Created on: Apr 30, 2013
 *      
 */

#include "BasicModel.h"


BasicModel::BasicModel(unordered_map<string, pair<ui,ui> >* pglobalWordsMap,
					   unordered_map<ui, vector<Tuple3> >* pcache,
		               vector<ul>* pwordsOffset, string prefixName):
globalWordsMap(pglobalWordsMap), cache(pcache), wordsOffset(pwordsOffset),
stringUtil( StringUtil::Instance()), config(Config::Instance())
{
	tuplesStream = new MyStream(config->workDirPath + prefixName + config->tuplesName, true, false, false);
}

BasicModel::~BasicModel() {
	delete tuplesStream;
}


void BasicModel::extractWordsIdAndFrequency(string& text, unordered_map<ui,  tuple<ui, float, string> >& wordsFreq){
	vector<string> words;
	extractWords(text, words);
	for(auto& w : words){
		auto wi = globalWordsMap->find(w);
		if(wi != globalWordsMap->end()){
			auto wf = wordsFreq.find((*wi).second.first);
			if(wf != wordsFreq.end()){
				++get<0>((*wf).second);
			}else{
				wordsFreq.insert(make_pair((*wi).second.first, make_tuple(1u, (*wi).second.second, w)));
			}
		}
	}

}


vector<pair<float, ui> >* BasicModel::search(string& text) {
	unordered_map<ui, float> docs;
	getDocsWeights(text, docs);

	auto ret = new vector<pair<float, ui> >(docs.size(), make_pair(0.0f, 0u));
	ui i = 0;
	for(auto& doc : docs){
		ret->at(i).first = doc.second;
		ret->at(i).second = doc.first ;
		++i;
	}

	return ret;
}

vector<Tuple3>* BasicModel::loadTuples(ui wordId) {
	vector<Tuple3>* tuples = new vector<Tuple3>();
	Tuple3 tuple;
	unordered_map<ui, vector<Tuple3> >::iterator cacheIt = cache->find(wordId);

	if (cacheIt != cache->end()) {
		//The tuples are in the cache
		(*tuples) = cacheIt->second;
	} else {
		//Sets the position of the first tuple
		tuplesStream->seekg((*wordsOffset)[wordId]);
		(*tuplesStream) >> tuple;
		while (!(tuple == MyStream::END_MARK3)) {
			tuples->push_back(tuple);
			(*tuplesStream) >> tuple;
		}
		cache->insert(pair<const ui, vector<Tuple3> >(wordId, *tuples));
	}

	return tuples;
}

