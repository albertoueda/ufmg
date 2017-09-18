/*
 * BM25Model.h
 *
 *  Created on: May 1, 2013
 *      
 */

#ifndef BM25MODEL_H_
#define BM25MODEL_H_

#include "BasicModel.h"
#include <functional>

class BM25Model : public BasicModel {
public:
	BM25Model(unordered_map<string, pair<ui,ui> >* pglobalWordsMap,
		     unordered_map<ui, vector<Tuple3> >* pcache,
		     vector<ul>* pwordsOffset, float pk1, float pb, string prefixName="");
	virtual ~BM25Model();

	virtual void getDocsWeights(string& text, unordered_map<ui, float>& docs);


	float getB() const {
		return b;
	}

	void setB(float b) {
		this->b = b;
	}

	float getK1() const {
		return k1;
	}

	void setK1(float k1) {
		this->k1 = k1;
	}

	void train(unordered_map<string, unordered_set<ui> >& answerKeyMap);

private:
	vector<float> docSizes;
	float avgDocSize;
	float ndocs;
	float k1;
	float b;
};

#endif /* BM25MODEL_H_ */
