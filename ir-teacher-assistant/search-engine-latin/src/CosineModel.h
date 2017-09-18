/*
 * CosineModel.h
 *
 *  Created on: May 1, 2013
 *      
 */

#ifndef COSINEMODEL_H_
#define COSINEMODEL_H_

#include "BasicModel.h"
#include <cmath>

class CosineModel : public BasicModel {
public:
	CosineModel(unordered_map<string, pair<ui,ui> >* pglobalWordsMap,
		     unordered_map<ui, vector<Tuple3> >* pcache,
		     vector<ul>* pwordsOffset, string prefixName="");
	virtual ~CosineModel();
	virtual void getDocsWeights(string& text, unordered_map<ui, float>& docs);

	float getB() const {
		return b;
	}

	void setB(float b) {
		this->b = b;
	}


	void train(unordered_map<string, unordered_set<ui> > answerKeyMap);

	float ndocs;

private:
	vector<float> docSizes;
	float b;//doc weight


	float tfIdf(float tf, float df){
		return (1.0f * log2(tf) * log2(ndocs/df));
	}

};

#endif /* COSINEMODEL_H_ */
