/*
 * LinearMixture.h
 *
 *  Created on: May 8, 2013
 *      
 */

#ifndef LINEARMIXTURE_H_
#define LINEARMIXTURE_H_

#include "BM25Model.h"
#include "CosineModel.h"
#include "DocInfo.h"

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <cmath>


class LinearMixture  : public BasicModel {
public:
	LinearMixture(unordered_map<string, pair<ui,ui> >* pglobalWordsMap,
			   unordered_map<ui, vector<Tuple3> >* pcache,
			   vector<ul>* pwordsOffset, BM25Model* pbm25, CosineModel* pcosine);
	virtual ~LinearMixture();

	void train(unordered_map<string, unordered_set<ui> >& answerKeyMap);
	virtual void getDocsWeights(string& text, unordered_map<ui, float>& docs);


private:
	BM25Model* bm25;
	CosineModel* cosine;
	ifstream fdocInfo;//Files that stores the informations of each document, like title, url
	unordered_map<ui, DocInfo> mapDocs;

	void getDocInfo( ui id, DocInfo &docInfo);
	void getWeights(unordered_map<ui, vector<float> >& weights, string& text);
	void getTextWeights(string& text, unordered_map<ui, float>& title, unordered_map<ui, float>& url, float b);
};

#endif /* LINEARMIXTURE_H_ */
