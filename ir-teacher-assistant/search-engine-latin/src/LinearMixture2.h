/*
 * LinearMixture2.h
 *
 *  Created on: Jun 22, 2013
 *      
 */

#ifndef LINEARMIXTURE2_H_
#define LINEARMIXTURE2_H_

#include "BM25Model.h"
#include "CosineModel.h"
#include "DocInfo.h"
#include "LinearMixture.h"
#include "PageRankModel.h"

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <cmath>

class LinearMixture2 : public BasicModel {
public:
	LinearMixture2(unordered_map<string, pair<ui,ui> >* panchorWordsMap,
			   unordered_map<ui, vector<Tuple3> >* panchorCache,
			   vector<ul>* panchorWordsOffset, BM25Model* panchorbm25,
			   CosineModel* panchorcosine, PageRankModel* ppagerank, LinearMixture* plinear);
	virtual ~LinearMixture2();
	void train(unordered_map<string, unordered_set<ui> >& answerKeyMap);
	virtual void getDocsWeights(string& text, unordered_map<ui, float>& docs);


private:

	BM25Model* bm25anchor;
	CosineModel* cosineanchor;
	PageRankModel* pagerank;
	LinearMixture* linear;
	float weigthlinear;
	float weigthcosine;
	float weigthbm25;
	float weigthpagerank, weigthpagerankK, weigthpagerankA; //Relevance Weighting for Query Independent Evidence SIGIR 2005


	void getWeights(unordered_map<ui, vector<float> >& weights, string& text);

	float getDocValue(unordered_map<ui, float>& docs, ui id){
		auto it = docs.find(id);
		return it == docs.end() ? 0.0f : it->second;
	}

	void trainPagerankXCosineAnchor(unordered_map<string, unordered_set<ui> >& answerKeyMap);
	void trainBm25Anchor(unordered_map<string, unordered_set<ui> >& answerKeyMap);
	void trainLinear(unordered_map<string, unordered_set<ui> >& answerKeyMap);


};

#endif /* LINEARMIXTURE2_H_ */
