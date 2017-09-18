/*
 * ContentBased.h
 *
 *  Created on: 26/10/2015
 *      Author: alberto
 */

#ifndef CONTENTBASED_H_
#define CONTENTBASED_H_

#include "Olympus.h"
#include "SimilarityCalc.h"
#include "RatingManager.h"

class ContentBased
{
	public:
		ContentBased();
		virtual ~ContentBased();
		void predict_by_imdb_rating();
		void predict();

	private:
		void preprocess_user(int);
		void preprocess_item(int);
		void compute_targets_similarities();

		SimilarityCalc similarityCalc;
		RatingManager ratingManager;
};

#endif /* CONTENTBASED_H_ */
