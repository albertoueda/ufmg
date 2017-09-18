/*
 * PageRankModel.h
 *
 *  Created on: Jun 14, 2013
 *      
 */

#ifndef PAGERANKMODEL_H_
#define PAGERANKMODEL_H_

#include "BasicModel.h"

class PageRankModel : public BasicModel{
public:
	PageRankModel(unordered_map<string, pair<ui,ui> >* pglobalWordsMap,
		     unordered_map<ui, vector<Tuple3> >* pcache,
		     vector<ul>* pwordsOffset, string prefixName="");
	virtual ~PageRankModel();

	virtual void getDocsWeights(string& text, unordered_map<ui, float>& docs);

	void train(unordered_map<string, unordered_set<ui> >& answerKeyMap);

private:
	vector<float> pageRanks;


};

#endif /* PAGERANKMODEL_H_ */
