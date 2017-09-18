/*
 * AnswerKey.h
 *
 *  Created on: May 1, 2013
 *      
 */

#ifndef ANSWERKEY_H_
#define ANSWERKEY_H_

#include "common.h"
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <string>
#include "BasicModel.h"

class AnswerKey {
public:
	AnswerKey(vector<pair<BasicModel*, string> >* prankingModels);
	virtual ~AnswerKey();

void getanswers(string& query, unordered_set<ui>& answerKey);
void getChartData(stringstream& ss, vector<MODEL_ENUM>& models, string& query);
void getQueries(vector<string>& queries);

private:
	unordered_map<string, unordered_set<ui> > answerKeyMap;
	vector<pair<BasicModel*, string> >* rankingModels;
	Config* config;
	vector<unordered_map<string, vector<pair<float, float> >*> > mapRecallPrecisonChartLocal;//models: query, recallXprecision
	vector<unordered_map<string, float>> precisionAtLocal;
	unordered_map<ui, vector<pair<float, float> >*> mapRecallPrecisonChartGlobal;
	unordered_map<ui, float> precisionAtGlobal;

	void fillRecalPrecisionMap(ui modelId);

	static void precisionsToGuage(stringstream& ss, vector<float>& precisions, vector<string>& models);
	static void RecallVsPrecisionToLine(stringstream& ss, vector<vector<pair<float, float> >*>& results, vector<string>& models);
	static void createHiddenInput(stringstream& ss, stringstream &value, const string id);

};

#endif /* ANSWERKEY_H_ */
