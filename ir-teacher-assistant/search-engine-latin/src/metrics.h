/*
 * metrics.h
 *
 *  Created on: May 14, 2013
 *      
 */

#ifndef METRICS_H_
#define METRICS_H_

#include <vector>
#include <unordered_set>
#include <sstream>

#include "common.h"


float precisionAt(ui n, vector<pair<float, ui> >& result, unordered_set<ui>& answerKey);
vector<pair<float, float> >* recallVsPrecision(vector<pair<float, ui> >& result, unordered_set<ui>& answerKey);
vector<pair<float, float> >* recallVsPrecisionAvg(vector<vector<pair<float, float> >*> values);
float auc(vector<pair<float, float> >& recallVsPrecision);
void normalizeRecallVsPrecision(vector<vector<pair<float, float> >*>& results);

pair<float, float> meanStd(vector<float>& values);

#endif /* METRICS_H_ */
