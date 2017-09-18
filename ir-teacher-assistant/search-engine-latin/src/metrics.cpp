/*
 * metrics.cpp
 *
 *  Created on: May 14, 2013
 *      
 */

#include "metrics.h"
#include <set>
#include <numeric>
#include <cmath>

pair<float, float> meanStd(vector<float>& values){
	pair<float, float> ms(0.0f, 0.0f);
	if(values.empty()){
		return ms;
	}
	float n = static_cast<float>(values.size());
	float mean = accumulate(values.begin(), values.end(), 0.0f)/n;
	ms.first = mean;

	if(n == 1){
		return ms;
	}

	float ss = 0.0f;
	for(float v : values){
		ss += (mean - v)*(mean - v);
	}


	ms.second = sqrt(ss/n-1);

	return ms;
}


float precisionAt(ui n, vector<pair<float, ui> >& result, unordered_set<ui>& answerKey) {
	float relevants = 0.f;
	auto end = answerKey.end();
	float precision = 0.0f;
	for(ui i = 0; i < n && i < result.size(); ++i){
		if(answerKey.find(result[i].second) != end){
			++relevants;
			precision += relevants/(i+1.0f);
		}
	}

	return static_cast<float>(precision)/min(static_cast<ui>(answerKey.size()), n);
}

vector<pair<float, float> >* recallVsPrecision(vector<pair<float, ui> >& result, unordered_set<ui>& answerKey) {
	auto recallxPrecision = new vector<pair<float, float> >();

	if(answerKey.empty()){
		recallxPrecision->push_back(make_pair(0.0f, 1.0f));
		recallxPrecision->push_back(make_pair(1.0f, 1.0f));
		return recallxPrecision;
	}

	float totalRelevants = answerKey.size();
	float relevants = 0;
	auto end = answerKey.end();
	ui n = 0;
	for(auto& r : result){
		++n;
		if(answerKey.find(r.second) != end){
			++relevants;
			float precI = relevants/n;
			float recI = relevants/totalRelevants;
			if(recallxPrecision->empty()){
				recallxPrecision->push_back(make_pair(0.0f, precI));
			}
			recallxPrecision->push_back(make_pair(recI, precI));
			if(cmp(recI, 1.0f) >= 0){
				break;
			}
		}
	}

	if(recallxPrecision->empty()){
		recallxPrecision->push_back(make_pair(0.0f, 0.0f));
		recallxPrecision->push_back(make_pair(1.0f, 0.0f));
		return recallxPrecision;
	}

	if(relevants < totalRelevants){
		recallxPrecision->push_back(make_pair(1.0f, 0.0f));
	}

	return recallxPrecision;
}

vector<pair<float, float> >* recallVsPrecisionAvg(vector<vector<pair<float, float> >*> values) {

	normalizeRecallVsPrecision(values);
	auto recallxPrecision = new vector<pair<float, float> >();
	if(values.empty()){
		return recallxPrecision;
	}

	float nResutls = values.size();

	for(ui i = 0; i < values[0]->size(); ++i){
		float recall = values[0]->at(i).first;
		float precision = 0.0f;
		for(ui j = 0; j < values.size(); ++j){
			precision += values[j]->at(i).second;
		}
		precision /= nResutls;
		recallxPrecision->push_back(make_pair(recall, precision));
	}

	return recallxPrecision;
}

void normalizeRecallVsPrecision(vector<vector<pair<float, float> > *>& results) {
	set<float> recalls;
	for(auto v: results){
		for(auto& f : *v){
			recalls.insert(f.first);
		}
	}

	vector<vector<pair<float, float> > *> newResults(results.size(), 0);
	vector<ui> index(results.size(), 0u);
	for(float recall : recalls){
		for(ui i = 0; i< results.size(); ++i){
			if(index[i] == 0){
				newResults[i] =  new vector<pair<float, float> >();
			}
			if(cmp(recall, results[i]->at(index[i]).first ) > 0){
				newResults[i]->push_back( make_pair(recall, results[i]->at(index[i-1]).second));
			}else{
				newResults[i]->push_back(results[i]->at(index[i]));
				if(index[i] < results[i]->size() - 1){
					++index[i];
				}
			}
		}
	}


	for(ui i = 0; i < results.size(); ++i){
		*results[i] = *newResults[i];
		delete newResults[i];
	}
}

float auc(vector<pair<float, float> >& recallVsPrecision) {
	float AUC = 0.0f;
	for(ui i = 1; i < recallVsPrecision.size(); ++i){
		//trapezoid
		AUC += (recallVsPrecision[i].second + recallVsPrecision[i-1].second)*(recallVsPrecision[i].first - recallVsPrecision[i-1].first)/2.0f;
	}

	return AUC;
}
