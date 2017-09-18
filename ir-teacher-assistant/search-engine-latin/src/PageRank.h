/*
 * PageRank.h
 *
 *  Created on: Jun 10, 2013
 *      
 */

#ifndef PAGERANK_H_
#define PAGERANK_H_

#include <vector>
#include "common.h"
#include "DocURLInfo.h"

class PageRank {
public:
	PageRank(float pd, ui pmaxNumberOfIterations, float pdelta);
	virtual ~PageRank();

	void calcRank(vector<DocURLInfo*>& urlDocs) const;

private:
	float d;//Damping factor
	ui maxNumberOfIterations;//to stop
	float delta;//stop criterion
};

#endif /* PAGERANK_H_ */
