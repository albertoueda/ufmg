/*
 * LinearMixture.cpp
 *
 *  Created on: May 8, 2013
 *      
 */

#include "LinearMixture.h"
#include <algorithm>
#include <numeric>
#include <queue>
#include <cmath>

LinearMixture::LinearMixture(unordered_map<string, pair<ui,ui> >* pglobalWordsMap,
		   unordered_map<ui, vector<Tuple3> >* pcache,
		   vector<ul>* pwordsOffset, BM25Model* pbm25, CosineModel* pcosine) : BasicModel(pglobalWordsMap, pcache, pwordsOffset),
		   bm25(pbm25), cosine(pcosine)
{
	fdocInfo.open(config->workDirPath +  config->docInfoName,  ifstream::in | ifstream::binary);
	if(fdocInfo.fail()){
			cerr << "Fail to open file: " << (config->workDirPath +  config->docInfoName) << endl;
			assert(fdocInfo.fail() == false);
	}

}

LinearMixture::~LinearMixture() {
	fdocInfo.close();
}


void condocertFusion( unordered_map<ui, float>& docs, unordered_map<ui, vector<float> >& weights){
	ui n = 10000;
	priority_queue< pair<float, ui>, vector<pair<float, ui> >, greater<pair<float, ui> > > topNdocs;
	for(auto& w : weights){
//		docs.insert(make_pair(w.first, w.second[4]));
		docs.insert(make_pair(w.first, accumulate(w.second.begin(), w.second.end(), 0.0f)));

		if(topNdocs.size() < n){
			topNdocs.push(make_pair(w.second[1], w.first));
		}else if(cmp(w.second[3], topNdocs.top().first) > 0){
			topNdocs.pop();
			topNdocs.push(make_pair(w.second[1], w.first));
		}
	}


	vector<ui> tmp(topNdocs.size());
	ui i = 0;
	while(!topNdocs.empty()){
		tmp[i] = topNdocs.top().second;
		++i;
		topNdocs.pop();
	}

	cout << "begin sort\n";
	//condorcet fusion
	vector<float> rankWeights = {1.0f, 1.0, 1.0, 1.0f};
	sort(tmp.begin(), tmp.end(), [&](ui a, ui b){
			vector<float>& wa = weights[a];
			vector<float>& wb = weights[b];

			float count = 0.0f;
			for(ui j = 0; j < wa.size(); ++j){
				int c = cmp(wa[j], wb[j]);
				if(c > 0){
					count += rankWeights[j];
				}else if(c < 0){
					count -= rankWeights[j];
				}
			}

			return cmp(count) > 0;
		});
	cout << "end sort\n";

	float numberOfDocs = static_cast<float>(tmp.size());
	for(i = 0; i <  tmp.size(); ++i){
		docs[tmp[i]] = 1e5 + numberOfDocs - i;
	}

}

void onlyOneModel(ui model, unordered_map<ui, float>& docs, unordered_map<ui, vector<float> >& weights){
	for(auto& w : weights){
		docs.insert(make_pair(w.first, w.second[model]));
	}

}

void sumModels(unordered_map<ui, float>& docs, unordered_map<ui, vector<float> >& weights){
	vector<float> rankWeights = {0.329481f, 0.332988f, 0.365551f,  0.367763f};
	//vector<float> rankWeights = {0.0f, 0, 0, 1};

	for(auto& w : weights){
		float m = 0.0f;
		for(ui i = 0; i < w.second.size(); ++i){
			m += rankWeights[i] * w.second[i];
		}
		docs.insert(make_pair(w.first, m));
	}

}


void trees( unordered_map<ui, float>& docs, unordered_map<ui, vector<float> >& weights){
	ui n = 10000;
	priority_queue< pair<float, ui>, vector<pair<float, ui> >, greater<pair<float, ui> > > topNdocs;
	for(auto& w : weights){
//		docs.insert(make_pair(w.first, w.second[4]));
		docs.insert(make_pair(w.first, accumulate(w.second.begin(), w.second.end(), 0.0f)));

		if(topNdocs.size() < n){
			topNdocs.push(make_pair(w.second[1], w.first));
		}else if(cmp(w.second[1], topNdocs.top().first) > 0){
			topNdocs.pop();
			topNdocs.push(make_pair(w.second[1], w.first));
		}
	}

	cout << "begin sort\n";
	string test("/home/itamar/workspacePython/tpRi/test.txt");
	ofstream output(test);
	vector<ui> ids(topNdocs.size());


	ui i = 0;
	while(!topNdocs.empty()){
		auto& td = topNdocs.top();
		auto& w = weights[td.second];
		ids[i] = td.second;
		++i;
		for(ui j = 0; j < w.size(); ++j){
			output << w[j]  << " ";
		}
		output << 1 << endl;

		topNdocs.pop();
	}

	cout << "Test instances " << i << endl;

	output.close();


	string predictions("/home/itamar/workspacePython/tpRi/prediction.txt");
	string sklearnPath("python /home/itamar/workspacePython/tpRi/test.py");

	string cmd = sklearnPath;
	cout << cmd << endl;
	cout << exec(cmd);

	ifstream fpredictions(predictions);
	ui f;
	fpredictions >> f;
	i = 0;
	while(fpredictions){
		docs[ids[i]] +=  10*f;
		fpredictions >> f;
		++i;
	}
	fpredictions.close();

}

void svmRank( unordered_map<ui, float>& docs, unordered_map<ui, vector<float> >& weights){

	ui n = 10000;
	priority_queue< pair<float, ui>, vector<pair<float, ui> >, greater<pair<float, ui> > > topNdocs;
	for(auto& w : weights){
		docs.insert(make_pair(w.first, accumulate(w.second.begin(), w.second.end(), 0.0f)));
		if(topNdocs.size() < n){
			topNdocs.push(make_pair(w.second[3], w.first));
		}else if(cmp(w.second[3], topNdocs.top().first) > 0){
			topNdocs.pop();
			topNdocs.push(make_pair(w.second[3], w.first));
		}
	}

	string test("test.txt"); // tmp
	ofstream output(test);
	vector<ui> ids(n);


	ui i = 0;
	while(!topNdocs.empty()){
		auto& td = topNdocs.top();
		auto& w = weights[td.second];
		ids[i] = td.second;
		++i;
		output << i << " qid:1";
		for(ui j = 0; j < w.size(); ++j){
			output << " " << (j+1) <<  ":" << w[j];
		}
		output << endl;

		topNdocs.pop();
	}

	output.close();


	string model("model.txt"); // tmp
	string predictions("predictions.txt"); // tmp
	string svmrankpath("/mnt/hd0/itamar/RI/svm_rank/svm_rank_classify ");

	string cmd = svmrankpath  + " " + test + " " + model + " " + predictions;
	cout << cmd << endl;
	cout << exec(cmd);

	ifstream fpredictions(predictions);
	ui f;
	fpredictions >> f;
	i = 0;
	while(fpredictions){
		docs[ids[i]] +=  10*f;
		fpredictions >> f;
		++i;
	}
	fpredictions.close();
}


void LinearMixture::getDocsWeights(string& text, unordered_map<ui, float>& docs){
	unordered_map<ui, vector<float> > weights;
	getWeights(weights, text);
	docs.reserve(weights.size());

	//cout << text << endl;

	//condocertFusion(docs, weights);
	//onlyOneModel(3, docs, weights);
	sumModels(docs, weights);
	//logisticRegression(docs, weights);
	//svmRank(docs, weights);

}



//void LinearMixture::train(unordered_map<string, unordered_set<ui> > answerKeyMap) {
//	string train("/home/itamar/workspacePython/tpRi/train.txt");
//	ofstream output(train);
//
//	for(auto& a : answerKeyMap){
//		unordered_map<ui, vector<float> > weights;
//		getWeights(weights, const_cast<string&>(a.first));
//		auto end = a.second.end();
//		ui i = 0;
//		for(auto& w : weights){
//			++i;
//
//			bool relevant = a.second.find(w.first) != end;
//			if( !relevant && ((rand()%100) > 5 || i > 500)){
//				continue;
//			}
//
//			for(ui j = 0; j < w.second.size(); ++j){
//				output  << w.second[j] << " ";
//			}
//
//			if(relevant){
//				output <<  1 << endl ;
//			}else{
//
//				output << 0 << endl;
//			}
//
//		}
//
//	}
//	output.close();
//	cout << "Training logistic\n";
//	string sklearnPath("python /home/itamar/workspacePython/tpRi/train.py");
//	string cmd = sklearnPath;
//	cout << exec(cmd);
//}

void LinearMixture::train(unordered_map<string, unordered_set<ui> >& answerKeyMap) {
	string train("train.txt");  // tmp
	string model("model.txt"); // tmp
	ofstream output(train);

	ui qid = 0;
	for(auto& a : answerKeyMap){
		++qid;
		output << "#" << qid << " " << a.first << endl;
		unordered_map<ui, vector<float> > weights;
		getWeights(weights, const_cast<string&>(a.first));
		auto end = a.second.end();
		ui i = 0;
		for(auto& w : weights){
			++i;
			if(a.second.find(w.first) != end){
				output <<  2;
			}else{
				if( (rand()%100) > 5 || i > 500){
					continue;
				}
				output << 1;
			}

			output << " qid:" << qid;
			for(ui i = 0; i < w.second.size(); ++i){
				output << " " << (i+1) << ":" << w.second[i];
			}
			output << endl;
		}

	}
	output.close();
	cout << "Training rank svm\n";
	string svmrankpath("/mnt/hd0/itamar/RI/svm_rank/svm_rank_learn ");
	string cmd = svmrankpath + " -c " + to_string(qid) + " " + train + " " + model;
	cout << exec(cmd);
}

void LinearMixture::getDocInfo(ui id, DocInfo& docInfo) {
	auto it = mapDocs.find(id);
	if(it != mapDocs.end()){
		docInfo = it->second;
		return;
	}

	DocInfo::readDoc(fdocInfo, id, docInfo);
	transform(docInfo.url.begin(), docInfo.url.end(), docInfo.url.begin(), ::tolower);

	string tmp;

	stringUtil->convertToIso88591(docInfo.title, tmp);
	transform(tmp.begin(),  tmp.end(), tmp.begin(), ::tolower);
	for(char &c : tmp){
		stringUtil->normalize(c);
	}
	docInfo.title = tmp;// + docInfo.description;


	mapDocs.insert(make_pair(id, docInfo));
}


void LinearMixture::getTextWeights(string& text,  unordered_map<ui, float>& title, unordered_map<ui, float>& url, float b){
	unordered_map < ui, tuple<ui, float, string>> wordsFreq; //word id, word freq query, doc freq
	extractWordsIdAndFrequency(text, wordsFreq);

	DocInfo docInfo;

	for(auto& w : wordsFreq){//calculates idf
		auto tuples = loadTuples(w.first);
		ui lastDoc;
		float idf  = log2(cosine->ndocs/get<1>(w.second));
		bool first = true;
		for(Tuple3& tuple : *tuples){
			if(first || tuple.docId != lastDoc){
				float tf = 1.0f + log2(tuple.wdocFrequency);

				first = false;
				lastDoc = tuple.docId;
				getDocInfo(tuple.docId, docInfo);
				string& word = get<2>(w.second);
				float weightTitle = tf*idf;
				float weightURL = weightTitle;

				if(docInfo.url.find(word) != string::npos){
					weightURL += b;
				}
				if(docInfo.title.find(word) != string::npos){
					weightTitle += b;
				}

				auto doc = title.find(lastDoc);
				if(doc != title.end()){
					doc->second += weightTitle;
				}else{
					title.insert(make_pair(lastDoc, weightTitle));
				}

				doc = url.find(lastDoc);
				if(doc != url.end()){
					doc->second += weightURL;
				}else{
					url.insert(make_pair(lastDoc, weightURL));
				}
			}
		}

		delete tuples;
	}

}

void LinearMixture::getWeights(unordered_map<ui, vector<float> >& weights, string& text) {

	unordered_map<ui, float> docsBM25;
	unordered_map<ui, float> docsCosine;
	unordered_map<ui, float> docsTitle;
	unordered_map<ui, float> docsUrl;



	bm25->getDocsWeights(text, docsBM25);
	cosine->setB(0.0f);
	cosine->getDocsWeights(text, docsCosine);
	cosine->setB(1.0f);

	getTextWeights(text, docsTitle, docsUrl, 100.0f);


	vector<float> maximuns({0.0f, 0.0f, 0.0f, 0.0f});


	for(auto& pCosineAll : docsCosine){
		ui id = pCosineAll.first;

		vector<float> v({
							pCosineAll.second , docsBM25[id], 	 docsTitle[id], docsUrl[id]
						});

		for(ui i = 0; i < v.size(); ++i){
			if(cmp( abs(v[i]), maximuns[i]) > 0){
				maximuns[i] = abs(v[i]);
			}
		}

		weights.insert(make_pair(id, v));
	}

	for(auto& w : weights){
		for(ui i = 0; i < w.second.size(); ++i){
			w.second[i] /= maximuns[i];
		}
	}
}



