/*
 * RatingManager.h
 *
 *  Created on: 04/10/2015
 *      Author: alberto
 */

#ifndef RATINGMANAGER_H_
#define RATINGMANAGER_H_

#include "Olympus.h"

class RatingManager
{
public:
	RatingManager();
	virtual ~RatingManager();

	float get_user_avg(int);
	float bin_search_rating(int, int, int, int);
	float find_rating(int, int);
	void predict_by_user_average();
	void predict_with_no_info();

};

#endif /* RATINGMANAGER_H_ */
