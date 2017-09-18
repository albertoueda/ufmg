/*
 * CosineModel.cpp
 *
 *  Created on: May 1, 2013
 *      
 */

#include "CosineModel.h"
#include <fstream>
#include "metrics.h"

CosineModel::CosineModel(unordered_map<string, pair<ui,ui> >* pglobalWordsMap,
	     unordered_map<ui, vector<Tuple3> >* pcache,
	     vector<ul>* pwordsOffset, string prefixName) : BasicModel(pglobalWordsMap, pcache, pwordsOffset, prefixName), b(1.0f) {

	ifstream fcosine(config->workDirPath + prefixName + config->cosineName, ios::binary|ios::in);
	if(fcosine.fail()){
		cerr << "Fail to open file: " << (config->workDirPath + prefixName + config->cosineName) << endl;
		assert(fcosine.fail() == false);
		exit(-1);
	}

	float length;
	size_t size = sizeof(float);
	docSizes.reserve(100000);
	fcosine.read(reinterpret_cast<char*>(&length), size);
	while(fcosine){
		docSizes.push_back(length);
		fcosine.read(reinterpret_cast<char*>(&length), size);
	}
	fcosine.close();

	ndocs = static_cast<float>(docSizes.size());
}

CosineModel::~CosineModel() {

}


void CosineModel::getDocsWeights(string& text, unordered_map<ui, float>& docs){
	unordered_map<ui, tuple<ui, float, string> > wordsFreq;//word id, word freq query, doc freq, word text
	extractWordsIdAndFrequency(text, wordsFreq);

	for(auto& w : wordsFreq){//calculates idf
		float idf  = log2(ndocs/get<1>(w.second));
		float tfIdfQ2 = (1.0f + log2(static_cast<float>( get<1>(w.second) ))) * idf * idf;
		//cout << "starting loading tuples\n";
		cout.flush();
		auto tuples = loadTuples(w.first);
		//cout << "end load\n";
		cout.flush();
		ui lastDoc;
		bool first = true;
		for(Tuple3& tuple : *tuples){
			//cout << tuple << endl;
			if(first || tuple.docId != lastDoc){
				first = false;
				lastDoc = tuple.docId;
				auto doc = docs.find(lastDoc);
				float weight = (1.0f + log2(tuple.wdocFrequency)) * tfIdfQ2 ;
				if(doc != docs.end()){
					doc->second += weight;
				}else{
					docs.insert(make_pair(lastDoc, weight));
				}
			}
		}

		delete tuples;
	}


	//cout << "size of documents " << docSizes.size() << endl;

	for(auto& doc : docs){
		if(doc.first >= docSizes.size()){
			cerr << "ERROR " << doc.first << endl;
			cerr.flush();
		}
		doc.second /= ((1.0f - b) + (docSizes[doc.first]) * b);
	}

	//cout << "end calc\n";
	cout.flush();
}

//b=0  auc=0.325847
void CosineModel::train(unordered_map<string, unordered_set<ui> > answerKeyMap) {

	ofstream fout("cosine.txt"); // tmp
	float bestB = 0.0f;
	float bestAUC = 0.0f;

	for(float b = 0.0f; b <= 1.0f; b += 0.01f){
		cout << b*100 << "%\n";
		this->setB(b);
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
			cout  << b << " " << AUC << endl;
		}

		fout << b << " " << AUC << endl;

		delete avg;
		for(auto v : recalVsPrecisionValues){
			delete v;
		}
	}


	this->setB(bestB);

	cout << "Cosine Parameter: b = " << bestB  << endl;

	fout.close();
}



