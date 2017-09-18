/*
 * PageRank.cpp
 *
 *  Created on: Jun 10, 2013
 *      
 */

#include "PageRank.h"
#include "Timer.h"

PageRank::PageRank(float pd, ui pmaxNumberOfIterations, float pdelta) :
d(pd), maxNumberOfIterations(pmaxNumberOfIterations), delta(pdelta) {

}

PageRank::~PageRank() {

}

void PageRank::calcRank(vector<DocURLInfo*>& urlDocs) const {
	cout << "Starting pagerank\n";
	Timer timer;
	timer.start();
	for(DocURLInfo* doc : urlDocs){//remove dangling links
		if(doc->docId >= doc->MAX_VALID_DOC_ID){
			doc->rank = 0.0f;
			for(auto& edge : doc->listInLinks){//DESCOMENTAR
				--edge->numberOfOutLinks;
			}
		}else{
			doc->rank = 1.0f;
		}
	}

	for(ui i = 0; i < maxNumberOfIterations; ++i){
		float currentDelta = 0.0f;
		for(DocURLInfo* doc : urlDocs){
		//	cout << "***" << doc->url << endl;
			if(doc->docId >= doc->MAX_VALID_DOC_ID){
				continue;
			}
			float rank = 0.0f;
			for(auto& edge : doc->listInLinks){
				if(edge->docId < edge->MAX_VALID_DOC_ID){
				//cout << edge->url << endl;
					rank += edge->rank/edge->numberOfOutLinks;
				}
			}
			rank = 1.0f - d  + d*rank;
			currentDelta = max(currentDelta, abs(doc->rank - rank));
			doc->rank = rank;
		//	cout << "rank " << rank << endl << endl;
		}
		cout << "interation " << i << " delta " << currentDelta << endl;
		if(cmp(delta, currentDelta) >= 0){
			break;
		}
	}

	partial_sort(urlDocs.begin(), urlDocs.begin() + 20, urlDocs.end(),
				 [](const DocURLInfo* a, const DocURLInfo* b){
						return a->rank > b->rank;
					}
		);

//	for(ui i = 0; i < 20; ++i){
//		//if(urlDocs[i]->docId < urlDocs[i]->MAX_VALID_DOC_ID)
//		cout << i << " " << urlDocs[i]->docId << " " <<  urlDocs[i]->rank << " " << urlDocs[i]->listInLinks.size() << " " << urlDocs[i]->numberOfOutLinks << " "<< urlDocs[i]->url << endl;
//	}

	cout << "End pagerank. Time: " << timer.stopAndGetSeconds() << endl;;
}


