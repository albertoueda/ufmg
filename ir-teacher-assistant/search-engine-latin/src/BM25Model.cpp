/*
 * BM25Model.cpp
 *
 *  Created on: May 1, 2013
 *      
 */

#include "BM25Model.h"
#include "metrics.h"


BM25Model::BM25Model(unordered_map<string, pair<ui,ui> >* pglobalWordsMap,
	     unordered_map<ui, vector<Tuple3> >* pcache,
	     vector<ul>* pwordsOffset, float pk1, float pb, string prefixName)  : BasicModel(pglobalWordsMap, pcache, pwordsOffset, prefixName),
	     k1(pk1), b(pb){

	ifstream fbm25(config->workDirPath + prefixName + config->bm25Name, ios::binary|ios::in);
	if(fbm25.fail()){
		cerr << "Fail to open file: " << (config->workDirPath  + prefixName + config->bm25Name) << endl;
		assert(fbm25.fail() == false);
		exit(-1);
	}

	float length;
	avgDocSize = 0;
	size_t size = sizeof(float);
	docSizes.reserve(100000);
	fbm25.read(reinterpret_cast<char*>(&length), size);
	while(fbm25){
		avgDocSize += length;
		docSizes.push_back(length);
		fbm25.read(reinterpret_cast<char*>(&length), size);
	}
	fbm25.close();

	ndocs = static_cast<float>(docSizes.size());
	avgDocSize /= length;
}

BM25Model::~BM25Model() {
}

void BM25Model::getDocsWeights(string& text, unordered_map<ui, float>& docs) {
	unordered_map < ui, tuple<ui, float, string>> wordsFreq; //word id, word freq query, doc freq
	extractWordsIdAndFrequency(text, wordsFreq);
	for (auto& w : wordsFreq) {
		//calculates idf
		float idf = log2(
				(ndocs - get<1>(w.second) + 0.5f) / (get<1>(w.second)  + 0.5f));
		auto tuples = loadTuples(w.first);
		ui lastDoc;
		bool first = true;
		for (Tuple3& tuple : *tuples) {
			if (  first || tuple.docId != lastDoc) {
				first = false;
				lastDoc = tuple.docId;

				auto doc = docs.find(lastDoc);
				float bij = (k1 + 1.0f) * tuple.wdocFrequency;
				bij /= k1 * ((1.0f - b) + b * (docSizes[lastDoc] / avgDocSize))
						+ tuple.wdocFrequency;
				float weight = bij * idf;
				if (doc != docs.end()) {
					doc->second += weight;
				} else {
					docs.insert(make_pair(lastDoc, weight));
				}
			}

		}

		delete tuples;
	}


}

//BM25 Parameters: b = 0.48 k1 = 0.1
void BM25Model::train(unordered_map<string, unordered_set<ui> >& answerKeyMap) {
	ofstream fout("bm25.txt"); // tmp
	float bestB = 0.0f;
	float bestK1 = 0.0f;
	float bestAUC = 0.0f;

	for(float b = 0.0f; b <= 1.0f; b += 0.01f){
		cout << b*100 << "%\n";
		this->setB(b);
		for(float k1 = 0.0f; k1 <= 4.0f; k1 += 0.1f){
			this->setK1(k1);
			vector<vector<pair<float, float> >*> recalVsPrecisionValues;
			for(auto& a: answerKeyMap){
					auto result = this->search(const_cast<string&>(a.first));
					sort(result->begin(), result->end(), greater<pair<float, ui> >());
					vector<pair<float, float> >* recpre = recallVsPrecision(*result, a.second);
					recalVsPrecisionValues.push_back(recpre);
					delete result;
			}
			auto avg = recallVsPrecisionAvg(recalVsPrecisionValues);
			float AUC = auc(*avg);
			if(cmp(bestAUC, AUC) < 0){
				bestAUC = AUC;
				bestB = b;
				bestK1 = k1;
			}

			fout << k1 << " " << b << " " << AUC << endl;

			delete avg;
			for(auto v : recalVsPrecisionValues){
				delete v;
			}
		}
	}


	this->setB(bestB);
	this->setK1(bestK1);

	cout << "BM25 Parameters: b = " << bestB << " k1 = " << bestK1 << endl;

	fout.close();
}




