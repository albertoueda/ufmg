/*
 * BooleanModel.h
 *
 *  Created on: Apr 30, 2013
 *      
 */

#ifndef BOOLEANMODEL_H_
#define BOOLEANMODEL_H_

#include "BasicModel.h"

class BooleanModel : public BasicModel {
public:
	BooleanModel(unordered_map<string, pair<ui,ui> >* pglobalWordsMap,
			     unordered_map<ui, vector<Tuple3> >* pcache,
			     vector<ul>* pwordsOffset, string prefixName="");
	virtual ~BooleanModel();
	virtual void getDocsWeights(string& text, unordered_map<ui, float>& docs);


	/**
	 * Writes the tuples from a and b in result while they have the same docId
	 */
	void writeResultsSameDocId(ui& ia, ui& ib,vector<Tuple3>& result, vector<Tuple3> &b, vector<Tuple3>& a);
	/**
	 * Intercept tuples from a and b considering the docId
	 */
	void AND(vector<Tuple3>& a, vector<Tuple3> &b, vector<Tuple3>& result);
	/**
	 * Adds tuples from a or b considering the docId
	 */
	void OR(vector<Tuple3>& a, vector<Tuple3> &b, vector<Tuple3>& result);
	/**
	 * Processes the result to show the user. If the socket is null the user is written in
	 * the standard output (console).
	 */

	void getDocuments(vector<Tuple3>& tuples, unordered_map<ui, float>& docs);
};

#endif /* BOOLEANMODEL_H_ */
