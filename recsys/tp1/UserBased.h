/*
 * UserBased.h
 *
 *  Created on: 04/10/2015
 *      Author: alberto
 */

#ifndef USERBASED_H_
#define USERBASED_H_

#include "Olympus.h"
#include "RatingManager.h"
#include "SimilarityCalc.h"

class UserBased
{
	public:
		UserBased();
		virtual ~UserBased();

		void select_neighbors(int, int, vector<neighbor>*, int, float);
		float aggregate_ratings(int, int, vector<neighbor>*);
		void predict_user_based();
		void process_ratings();

	private:
		SimilarityCalc similarityCalc;
		RatingManager ratingManager;
};

#endif /* USERBASED_H_ */
