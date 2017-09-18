/*
 * LinearMixture2.cpp
 *
 *  Created on: Jun 22, 2013
 *      
 */

#include "LinearMixture2.h"
#include "metrics.h"

LinearMixture2::LinearMixture2(unordered_map<string, pair<ui,ui> >* panchorWordsMap,
		   unordered_map<ui, vector<Tuple3> >* panchorCache,
		   vector<ul>* panchorWordsOffset, BM25Model* panchorbm25, CosineModel* panchorcosine,
		   PageRankModel* ppagerank, LinearMixture* plinear) : BasicModel(panchorWordsMap, panchorCache, panchorWordsOffset),
		   bm25anchor(panchorbm25), cosineanchor(panchorcosine), pagerank(ppagerank), linear(plinear),
		   //weigthlinear(0.375508f), weigthcosine(0.385199f), weigthbm25(0.37784f), weigthpagerank(0.0765739f)
		   weigthlinear(0.8f), weigthcosine(0.55f), weigthbm25(0.0f), weigthpagerank(0.26f)

{

}

LinearMixture2::~LinearMixture2() {

}


void LinearMixture2::getDocsWeights(string& text, unordered_map<ui, float>& docs){
	unordered_map<ui, vector<float> > weights;
	getWeights(weights, text);
	docs.reserve(weights.size());

	for(auto& w : weights){
		float m = w.second[0] * weigthlinear + w.second[1] * weigthcosine +
				w.second[2]*weigthbm25 + w.second[3]*weigthpagerank;
		docs.insert(make_pair(w.first, m));
	}

}


void LinearMixture2::trainPagerankXCosineAnchor(unordered_map<string, unordered_set<ui> >& answerKeyMap){

	ofstream fout("pagerankcosine.txt"); // tmp
	float bestPrank = 0.0f;
	float bestCosine = 0.0f;
	float bestAUC = 0.0f;

	for(float wprank = 0.0f; wprank <= 1.0f; wprank += 0.01f){
		cout << wprank*100 << "% auc " << bestAUC << endl;
		this->weigthpagerank = wprank;
		for(float wcosine = 0.0f; wcosine <= 1.0f; wcosine += 0.01f){
			this->weigthcosine = wcosine;
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
				bestPrank = wprank;
				bestCosine = wcosine;
			}

			fout << wcosine << " " << wprank << " " << AUC << endl;

			delete avg;
			for(auto v : recalVsPrecisionValues){
				delete v;
			}
		}
	}


	this->weigthpagerank = bestPrank;
	this->weigthcosine = bestCosine;

	cout << "Page rank Parameter = " << bestPrank << " Cosine = " << bestCosine << " AUC " << bestAUC <<  endl;

	fout.close();
}

void LinearMixture2::trainBm25Anchor(unordered_map<string, unordered_set<ui> >& answerKeyMap){
	ofstream fout("mbm25.txt"); // tmp
	float bestBm25 = 0.0f;
	float bestAUC = 0.0f;

	for(float wbm25 = 0.0f; wbm25 <= 1.0f; wbm25 += 0.01f){
		cout << wbm25*100 << "%\n";
		this->weigthbm25 = wbm25;
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
			bestBm25 = wbm25;
			cout  << wbm25 << " " << AUC << endl;
		}

		fout << wbm25 << " " << AUC << endl;

		delete avg;
		for(auto v : recalVsPrecisionValues){
			delete v;
		}
	}


	this->weigthbm25 = bestBm25;

	cout << "Bm25 Parameter: b = " << bestBm25 << " AUC " << bestAUC <<  endl;

	fout.close();
}

void LinearMixture2::trainLinear(unordered_map<string, unordered_set<ui> >& answerKeyMap){
	ofstream fout("mlinear.txt"); // tmp
	float bestLinear = 0.0f;
	float bestAUC = 0.0f;

	for(float wlinear = 0.0f; wlinear <= 1.0f; wlinear += 0.1f){
		cout << wlinear*100 << "%\n";
		this->weigthlinear = wlinear;
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
			bestLinear = wlinear;
			cout  << wlinear << " " << AUC << endl;
		}

		fout << wlinear << " " << AUC << endl;

		delete avg;
		for(auto v : recalVsPrecisionValues){
			delete v;
		}
	}


	this->weigthlinear = bestLinear;

	cout << "Linear Parameter:  = " << bestLinear  << " AUC " << bestAUC <<  endl;

	fout.close();
}

void LinearMixture2::train(unordered_map<string, unordered_set<ui> >& answerKeyMap) {
	cout << "Training Pagerank x Cosine\n";
	trainPagerankXCosineAnchor(answerKeyMap);
	cout << "Training BM25\n";
	trainBm25Anchor(answerKeyMap);
	cout << "Training Linear\n";
	trainLinear(answerKeyMap);

}



void LinearMixture2::getWeights(unordered_map<ui, vector<float> >& weights, string& text) {

	unordered_map<ui, float> docsBM25anchor;
	unordered_map<ui, float> docsCosineAnchor;
	unordered_map<ui, float> docsLinear;
	unordered_map<ui, float> docsPagerank;

	unordered_map<ui, float>* rdocs;
	cosineanchor->setB(0.0f);
	cosineanchor->getDocsWeights(text, docsCosineAnchor);
	cosineanchor->setB(1.0f);
	rdocs = &docsCosineAnchor;

	if(cmp(weigthbm25)){
		bm25anchor->getDocsWeights(text, docsBM25anchor);
	}

	if(cmp(weigthpagerank)){
		pagerank->getDocsWeights(text, docsPagerank);
	}
	if(cmp(weigthlinear)){
		linear->getDocsWeights(text, docsLinear);
		rdocs = &docsLinear;
	}

	vector<float> maximuns({0.0f, 0.0f, 0.0f, 0.0f});

	weights.reserve(rdocs->size());

	for(auto& d : *rdocs){
		ui id = d.first;

		vector<float> v({
						getDocValue(docsLinear, id), getDocValue(docsCosineAnchor, id) ,
						getDocValue(docsBM25anchor, id), getDocValue(docsPagerank, id)
						});

		for(ui i = 0; i < v.size(); ++i){
			if(cmp( abs(v[i]), maximuns[i]) > 0){
				maximuns[i] = abs(v[i]);
			}
		}

		weights.insert(make_pair(id, v));

	}

	for(float &m : maximuns){
		if(cmp(m) == 0){
			m = 1.0f;
		}
	}

	for(auto& w : weights){
		for(ui i = 0; i < w.second.size(); ++i){
			w.second[i] /= maximuns[i];
		}
	}
}

