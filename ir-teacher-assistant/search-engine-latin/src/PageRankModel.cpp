/*
 * PageRankModel.cpp
 *
 *  Created on: Jun 14, 2013
 *      
 */

#include "PageRankModel.h"
#include <fstream>
#include <cassert>
#include <cmath>
#include "DocInfo.h"
#include "metrics.h"

PageRankModel::PageRankModel(unordered_map<string, pair<ui,ui> >* pglobalWordsMap,
	     unordered_map<ui, vector<Tuple3> >* pcache,
	     vector<ul>* pwordsOffset, string prefixName): BasicModel(pglobalWordsMap, pcache, pwordsOffset, prefixName) {
	ifstream fpagerank(config->workDirPath + prefixName + config->pageRankName, ios::binary|ios::in);
		if(fpagerank.fail()){
			cerr << "Fail to open file: " << (config->workDirPath  + prefixName + config->pageRankName) << endl;
			assert(fpagerank.fail() == false);
			exit(-1);
		}


		float rank;
		size_t size = sizeof(float);
		pageRanks.reserve(1000);
		fpagerank.read(reinterpret_cast<char*>(&rank), size);
		while(fpagerank){
			pageRanks.push_back(rank);
			fpagerank.read(reinterpret_cast<char*>(&rank), size);
		}
		fpagerank.close();

		vector<pair<float, ui> > topR(pageRanks.size(), pair<float,ui>(0.0f, 0u));
// ALBERTO
//		for(ui i = 0; i < pageRanks.size(); ++i){
//			topR[i].first = pageRanks[i];
//			topR[i].second = i;
//		}
//
//
//
//		partial_sort(topR.begin(), topR.begin() + 20, topR.end(), greater<pair<float, ui> >());
//		DocInfo docInfo;
//		ifstream fdocInfo(config->workDirPath +  config->docInfoName,  ifstream::in | ifstream::binary);
//		if(fdocInfo.fail()){
//				cerr << "Fail to open file: " << (config->workDirPath +  config->docInfoName) << endl;
//				assert(fdocInfo.fail() == false);
//		}
//		for(ui i = 0; i < 20; ++i){
//			DocInfo::readDoc(fdocInfo, topR[i].second, docInfo);
//			cout << i << " " << topR[i].first << " " << topR[i].second << " " << docInfo.url << endl;
//		}
//
//		fdocInfo.close();
}

PageRankModel::~PageRankModel() {
	// TODO Auto-generated destructor stub
}


void PageRankModel::getDocsWeights(string& text, unordered_map<ui, float>& docs) {
	unordered_map < ui, tuple<ui, float, string>> wordsFreq; //word id, word freq query, doc freq
	extractWordsIdAndFrequency(text, wordsFreq);
	for (auto& w : wordsFreq) {
		//calculates idf
		auto tuples = loadTuples(w.first);
		ui lastDoc;
		bool first = true;
		for (Tuple3& tuple : *tuples) {
			if (  first || tuple.docId != lastDoc) {
				first = false;
				lastDoc = tuple.docId;

				auto doc = docs.find(lastDoc);

				if (doc == docs.end()) {
					docs.insert(make_pair(lastDoc, pageRanks[lastDoc]));
				}
			}

		}

		delete tuples;
	}


}


