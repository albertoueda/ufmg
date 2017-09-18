/*
 * BooleanModel.cpp
 *
 *  Created on: Apr 30, 2013
 *      
 */

#include "BooleanModel.h"

BooleanModel::BooleanModel(unordered_map<string, pair<ui,ui> >* pglobalWordsMap,
	     unordered_map<ui, vector<Tuple3> >* pcache,
	     vector<ul>* pwordsOffset, string prefixName): BasicModel(pglobalWordsMap, pcache, pwordsOffset, prefixName) {

}

BooleanModel::~BooleanModel() {
}



/**
 * Intercept tuples from a and b considering the docId
 */
void BooleanModel::AND(vector<Tuple3>& a, vector<Tuple3> &b, vector<Tuple3>& result) {
	ui ia = 0, ib = 0;
	while(ia < a.size() && ib < b.size()){
		if(a[ia].docId < b[ib].docId){
			++ia;
		}else if(b[ib].docId < a[ia].docId){
			++ib;
		}else{
			writeResultsSameDocId(ia, ib, result, b, a);
		}
	}
}

void BooleanModel::getDocsWeights(string& text, unordered_map<ui, float>& docs) {
		vector<string> words;
		extractWords(text, words);

		bool isAnd = true;
		vector<Tuple3>* a = NULL;
		vector<Tuple3>* b = NULL;
		vector<Tuple3>* result = NULL;
		unordered_map<string, pair<ui,ui> >::const_iterator it;
		unordered_map<string, pair<ui,ui> >::const_iterator end = globalWordsMap->end();

		for(string& w : words){
			if("or" == w){
				isAnd = false;
				continue;
			}else if("and" == w){
				isAnd = true;
				continue;
			}
			it = globalWordsMap->find(w);
			if(it == end){
				cout << "The word " << w << " was not indexed\n";
				continue;
			}

			if(a == NULL) {
				a = loadTuples(it->second.first);
			}else{
				b = loadTuples(it->second.first);
				result = new vector<Tuple3>();
				if(isAnd){
					AND(*a, *b, *result);
				}else{
					OR(*a, *b, *result);
				}
				delete a;
				delete b;
				b = NULL;
				a = result;
				isAnd = true;
			}
		}

		if(b != NULL){
			delete b;
		}

		if(a == NULL){
			return;
		}

		getDocuments(*a, docs);
		delete a;
}

void BooleanModel::getDocuments(vector<Tuple3>& tuples, unordered_map<ui, float>& docs){
	ui lastDoc;
	bool first = true;

	for(Tuple3& t : tuples){
		if(first || lastDoc != t.docId){
			first = false;
			lastDoc = t.docId;
			docs.insert(make_pair(lastDoc, 0.0f));
		}
	}
}

/**
 * Adds tuples from a or b considering the docId
 */
void BooleanModel::OR(vector<Tuple3>& a, vector<Tuple3> &b, vector<Tuple3>& result) {
	ui ia = 0, ib = 0;
	while(ia < a.size() || ib < b.size()){
		if(ia == a.size()){
			while(ib < b.size()){
				result.push_back(b[ib]);
				++ib;
			}
			break;
		}
		if(ib == b.size()){
			while(ia < a.size()){
				result.push_back(a[ia]);
				++ia;
			}
			break;
		}
		while(a[ia].docId < b[ib].docId && ia < a.size()){
			result.push_back(a[ia]);
			++ia;
		}
		while(b[ib].docId < a[ia].docId && ib < b.size()){
			result.push_back(b[ib]);
			++ib;
		}
		writeResultsSameDocId(ia, ib, result, b, a);
	}
}


/**
 * Writes the tuples from a and b in result while they have the same docId
 */
void BooleanModel::writeResultsSameDocId(ui& ia, ui& ib,vector<Tuple3>& result, vector<Tuple3> &b, vector<Tuple3>& a) {
	ui currDoc = a[ia].docId;
	while (a[ia].docId == currDoc || b[ib].docId == currDoc) {
		if (a[ia] == MyStream::END_MARK3 || a[ia].docId != currDoc) {
			//end of a
			while (!(b[ib] == MyStream::END_MARK3) && b[ib].docId == currDoc) {
				result.push_back(b[ib]);
				++ib;
			}
			break;
		}

		if (b[ib] == MyStream::END_MARK3 || b[ib].docId != currDoc) {
			//end of b
			while (!(a[ia] == MyStream::END_MARK3) && a[ia].docId == currDoc) {
				result.push_back(a[ia]);
				++ia;
			}
			break;
		}

		if (a[ia] < b[ib]) {
			result.push_back(a[ia]);
			++ia;
		} else if (b[ib] < a[ia]) {
			result.push_back(b[ib]);
			++ib;
		} else {
			result.push_back(b[ib]);
			++ia;
			++ib;
		}
	}
}

