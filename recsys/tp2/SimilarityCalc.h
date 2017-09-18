/*
 * SimilarityCalc.h
 *
 *  Created on: 04/10/2015
 *      Author: alberto
 */

#ifndef SIMILARITYCALC_H_
#define SIMILARITYCALC_H_

#include "Olympus.h"

class SimilarityCalc
{
public:
	SimilarityCalc();
	virtual ~SimilarityCalc();

	void compute_user_norm(int);
	void compute_item_norm(int);
	float similarity(int, int);
};


#endif /* SIMILARITYCALC_H_ */
