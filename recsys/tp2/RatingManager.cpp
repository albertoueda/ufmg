/*
 * RatingManager.cpp
 *
 *  Created on: 04/10/2015
 *      Author: alberto
 */

#include "RatingManager.h"

RatingManager::RatingManager(){}
RatingManager::~RatingManager(){}

float RatingManager::get_user_avg(int user)
{
	if (ratings[user].empty()) return 0;

	int total_points = 0, total_items = 0;

	for (int j = 0; j < ratings[user].size(); j++) {
		total_points += ratings[user][j].value;
		total_items++;
	}

	if (total_items == 0) return 0;

	return 1.0f * total_points / total_items;
}

float RatingManager::bin_search_rating(int user, int item, int begin, int end)
{
	if (begin < end)
	{
		int middle = (begin + end) / 2;

		if (ratings[user][middle].id == item)
			return ratings[user][middle].value;

		if (item < ratings[user][middle].id)
			return bin_search_rating(user, item, begin, middle);
		else
			return bin_search_rating(user, item, middle + 1, end);
	}

	if (ratings[user][begin].id == item)
		return ratings[user][begin].value;

	return -1;
}

float RatingManager::find_rating(int user, int item)
{
	if (ratings[user].empty())
		return -1;
	// cout << "  Searching for: " << user << " and " << item << ". Visited items: ";
	return bin_search_rating(user, item, 0, ratings[user].size() - 1);
}

void RatingManager::predict_by_user_average()
{
	for (int i = 0; i < targets.size(); i++)
	{
		if (targets[i].value > 0) continue;

		float user_avg = avg_rating[targets[i].user];

		if (user_avg != 0)
			targets[i].value = user_avg;
	}
}

void RatingManager::predict_with_no_info()
{
	for (int i = 0; i < targets.size(); i++)
	{
		if (targets[i].value <= 0)
			targets[i].value = NO_INFO_RATING_PREDICTION;
	}
}

