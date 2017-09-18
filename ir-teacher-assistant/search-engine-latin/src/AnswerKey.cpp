/*
 * AnswerKey.cpp
 *
 *  Created on: May 1, 2013
 *      
 */

#include "AnswerKey.h"
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "BM25Model.h"
#include "LinearMixture.h"
#include "LinearMixture2.h"
#include "CosineModel.h"
#include "PageRankModel.h"
#include "DocInfo.h"
#include "metrics.h"
#include "Timer.h"
#include "htmlcxx/html/utils.h"
#include "htmlcxx/html/Uri.h"



AnswerKey::AnswerKey(vector<pair<BasicModel*, string> >* prankingModels) : rankingModels(prankingModels), config(Config::Instance()),
mapRecallPrecisonChartLocal(rankingModels->size()), precisionAtLocal(rankingModels->size())
{
	auto config = Config::Instance();

	DIR *dpdf;
	struct dirent *epdf;
	string path = config->compressedBasePath + "/RELEVANTES_RI2011/";

	unordered_map<string, pair<vector<string>, ui> > answers;//url,queries, docId

	dpdf = opendir(path.c_str());
	if (dpdf != NULL){
	   while ( (epdf = readdir(dpdf)) ){
	      string query(epdf->d_name);
	      if("." == query || ".." == query){
	    	  continue;
	      }
	      ifstream input(path + query);
	      string url;
	      input >> url;
  		 string empty("");
	      while(input){
	    	   htmlcxx::Uri absoluteUrl = htmlcxx::Uri(url);
	    		url = htmlcxx::HTML::convert_link(empty, absoluteUrl);
	    		transform(url.begin(), url.end(), url.begin(), ::tolower);

	    	  auto it = answers.find(url);
	    	  if(it != answers.end()){
	    		  (*it).second.first.push_back(query);
	    	  }else{
	    		  answers.insert(make_pair(url, make_pair( vector<string>(1, query), 0u)));
	    	  }

	    	  input >> url;
	      }

	      input.close();
	   }
	}else{
		cerr << "ERROR while opening the directory " << path << endl;
	}
	closedir(dpdf);

	ifstream fdocInfo(config->workDirPath +  config->docInfoName,  ifstream::in | ifstream::binary);
	if(fdocInfo.fail()){
			cerr << "Fail to open file: " << (config->workDirPath +  config->docInfoName) << endl;
			assert(fdocInfo.fail() == false);
	}

	cout << "Loading answer key\n";

	DocInfo doc;
	ui id = 0;
	fdocInfo >> doc;
	while(fdocInfo){
		auto itAns = answers.find(doc.url);
		if(itAns != answers.end()){
			(*itAns).second.second = id;
			for(string& query : (*itAns).second.first){
			//	cout << query << " " << doc.url <<  " " << id << endl;
				auto itKey = answerKeyMap.find(query);
				if(itKey != answerKeyMap.end()){
					(*itKey).second.insert(id);
				}else{
					answerKeyMap.insert(make_pair(query, unordered_set<ui>({id})));
				}
			}
		}
		fdocInfo >> doc;
		++id;
	}


	fdocInfo.close();



//	LinearMixture* lm = dynamic_cast<LinearMixture*>(rankingModels->at(static_cast<ui>(MODEL_ENUM::MIX)).first);
//	lm->train(answerKeyMap);
//
//	CosineModel* cos = dynamic_cast<CosineModel*>(rankingModels->at(static_cast<ui>(MODEL_ENUM::ANCHOR_COSINE)).first);
//	cos->train(answerKeyMap);

//	BM25Model* bm25Anchor = dynamic_cast<BM25Model*>(rankingModels->at(static_cast<ui>(MODEL_ENUM::ANCHOR_BM25)).first);
//	bm25Anchor->train(answerKeyMap);

//	LinearMixture2* mix2 = dynamic_cast<LinearMixture2*>(rankingModels->at(static_cast<ui>(MODEL_ENUM::MIX2)).first);
//	mix2->train(answerKeyMap);

	cout << "Caching answer key\n";

	for(ui modelId = 0; modelId < rankingModels->size(); ++modelId){
		//if(modelId <= 3){continue;}
		cout << "Caching for model " << modelId << endl;
		fillRecalPrecisionMap(modelId);
	}

}

AnswerKey::~AnswerKey() {
	for(auto& m : mapRecallPrecisonChartLocal){
		for(auto& p : m){
			delete p.second;
		}
	}

	for(auto&p : mapRecallPrecisonChartGlobal){
		delete p.second;
	}
}

void AnswerKey::getanswers(string& query, unordered_set<ui>& answerKey){
	auto it = answerKeyMap.find(query);
	if(it != answerKeyMap.end()){
		answerKey = it->second;
	}
}

void AnswerKey::getQueries(vector<string>& queries){
	for(auto& a: answerKeyMap){
		queries.push_back(a.first);
	}
}



void AnswerKey::precisionsToGuage(stringstream& ss, vector<float>& precisions,
		vector<string>& models) {
	ss.precision(3);

	ss  << "[ ['Label', 'Value']";
	for(ui i = 0; i < models.size(); ++i){
		ss << ",['" << models[i] << "'," << (precisions[i]*100.0f) << "]";
	}

	ss << "]";
}

void AnswerKey::RecallVsPrecisionToLine(stringstream& ss,
		vector<vector<pair<float, float> > *>& results, vector<string>& models) {
	ss.precision(3);

	ss  << "[ ['Recall'";
	for(string& m : models){
		ss << ",'" << m << "'";
	}
	ss << "]";
	for(ui i = 0; i < results[0]->size(); ++i){
		ss << ",[" << results[0]->at(i).first*100.0f;
		for(ui j = 0; j < results.size(); ++j){
			ss << "," << results[j]->at(i).second*100.0f;
		}
		ss << "] ";
	}

	ss << "]";
}

void writteResults(string& modelName, float precisionAt, vector<pair<float, float> >& recallVsPrecision){
	float a = auc(recallVsPrecision);
	ofstream fauc("auc_" + modelName); // tmp
	fauc << a << " " << precisionAt << endl;
	fauc.close();

	ofstream fpreXrec("preXrec_" + modelName); // tmp
	for(auto& p : recallVsPrecision){
		fpreXrec << p.first << " " << p.second << endl;
	}

	fpreXrec.close();

}

void AnswerKey::fillRecalPrecisionMap(ui modelId) {
	vector<vector<pair<float, float> >*> recalVsPrecisionValues;
	float avgPrecision = 0.0f;

	//ofstream fstats("/tmp/queriesAuc_" + rankingModels->at(modelId).second);

	for(auto& a: answerKeyMap){
		//cout << a.first << endl;
		auto result = rankingModels->at(modelId).first->search(const_cast<string&>(a.first));

		if(static_cast<MODEL_ENUM>(modelId) != MODEL_ENUM::BOOLEAN){
			sort(result->begin(), result->end(), greater<pair<float, ui> >());
		//	partial_sort(result->begin(),  result->end(), result->end(), greater<pair<float, ui> >());
		}
		vector<pair<float, float> >* recpre = recallVsPrecision(*result, a.second);

		mapRecallPrecisonChartLocal[modelId].insert(make_pair( a.first, recpre));
		recalVsPrecisionValues.push_back(recpre);
		float precision = precisionAt(10, *result, a.second);

//		string name = "_" + a.first + "_" + rankingModels->at(modelId).second;
//		for(char& c : name){
//			if(c == ' '){
//				c = '-';
//			}
//		}
//		writteResults(name, precision, *recpre);

//		float au = auc(*recpre);
//		fstats << a.first << " " << au << " " << precision << endl;
		precisionAtLocal[modelId].insert(make_pair(a.first, precision));
		avgPrecision += precision;
	}

//	fstats.close();


	mapRecallPrecisonChartGlobal.insert(make_pair(modelId, recallVsPrecisionAvg(recalVsPrecisionValues)));
	precisionAtGlobal.insert(make_pair(modelId, avgPrecision/answerKeyMap.size()));

//	ofstream fstats("/tmp/queriesTime_" + rankingModels->at(modelId).second);
//	for(auto& a: answerKeyMap){
//		Timer timer;
//		vector<float> times(10, 0.0f);
//		ui ndocs = 0;
//		for(ui x = 0; x < 10; ++x){
//			timer.start();
//			auto result = rankingModels->at(modelId).first->search(const_cast<string&>(a.first));
//			ndocs = result->size();
//			if(static_cast<MODEL_ENUM>(modelId) != MODEL_ENUM::BOOLEAN){
//
//				//partial_sort(result->begin(),  result->begin()+10, result->end(), greater<pair<float, ui> >());
//			}
//			times[x] = timer.stop();
//			delete result;
//		}
//		pair<float, float> mstd = meanStd(times);
//		fstats << ndocs << " " << mstd.first << " " << mstd.second << endl;
//	}
//
//	fstats.close();
}



void AnswerKey::getChartData(stringstream& inputs, vector<MODEL_ENUM>& models, string& query) {
	vector<float> precisionsAtLocal(models.size(), 0.0f);
	vector<float> precisionsAtGlobal(models.size(), 0.0f);
	vector<vector<pair<float, float> >*> recallVsPrecisionsLocal(models.size(), 0);
	vector<vector<pair<float, float> >*> recallVsPrecisionsGlobal(models.size(), 0);
	vector<string> modelsName;


	ui i = 0;
	for(MODEL_ENUM model : models){
		ui modelId = static_cast<ui>(model);
		modelsName.push_back((*rankingModels)[modelId].second);
		precisionsAtLocal[i] = precisionAtLocal[modelId][query];
		precisionsAtGlobal[i] = precisionAtGlobal[modelId];
		recallVsPrecisionsLocal[i] = mapRecallPrecisonChartLocal[modelId][query];
		recallVsPrecisionsGlobal[i] = mapRecallPrecisonChartGlobal[modelId];
		writteResults(rankingModels->at(modelId).second, precisionsAtGlobal[i], *recallVsPrecisionsGlobal[i]);
		++i;
	}

	normalizeRecallVsPrecision(recallVsPrecisionsLocal);
	normalizeRecallVsPrecision(recallVsPrecisionsGlobal);

	stringstream tmp;
	precisionsToGuage(tmp, precisionsAtLocal, modelsName);
	createHiddenInput(inputs, tmp, "localGauge");
	tmp.str(string());

	precisionsToGuage(tmp, precisionsAtGlobal, modelsName);
	createHiddenInput(inputs, tmp, "globalGauge");
	tmp.str(string());

	RecallVsPrecisionToLine(tmp, recallVsPrecisionsLocal, modelsName);
	createHiddenInput(inputs, tmp, "localLine");
	tmp.str(string());

	RecallVsPrecisionToLine(tmp, recallVsPrecisionsGlobal, modelsName);
	createHiddenInput(inputs, tmp, "globalLine");
}


void AnswerKey::createHiddenInput(stringstream& ss, stringstream& value, const string id) {
	ss << "<input type='hidden' id='" << id << "' value=\"" << value.str() << "\">";
}





