/*
 * UserBased.cpp
 *
 *  Created on: 04/10/2015
 *      Author: alberto
 */

#include "UserBased.h"
#include "IO.h"

UserBased::UserBased()
{
}

UserBased::~UserBased(){}

bool compare_items (rating a, rating b)
{
	return (a.id < b.id);
}

void UserBased::process_ratings()
{
	for (int user = 0; user <= max_user_id; user++)
	{
		if (ratings[user].empty()) continue;
		sort(ratings[user].begin(), ratings[user].end(), compare_items);
		similarityCalc.compute_norm(user);
		avg_rating[user] = ratingManager.get_user_avg(user);
//		cout << "  User " << user << " with avg: " << avg_rating[user] << endl;
	}
}

void UserBased::select_neighbors(int user, int item, vector<neighbor> *neighbors, int total_neighbors, float threshold)
{
	int count = 0;
	for (int i = 0; i <= max_user_id; i++)
	{
//		Limiting the number of neighbors worsens the recommendation quality
//		if (count == total_neighbors) break;

		int other_user = all_similarities[user][i].id;

		float other_user_rating = ratingManager.find_rating(other_user, item);
		if (other_user_rating == -1)
			continue;

		if (all_similarities[user][i].similarity < threshold) break;

		neighbors->emplace_back(other_user, other_user_rating,
						all_similarities[user][i].similarity);
//		cout << "Adding neighbor " << other_user << " with rating " << other_user_rating <<
//				" and similarity: " << all_similarities[user][i].similarity << endl;
		count++;
	}
}

float UserBased::aggregate_ratings(int ref_user, int item, vector<neighbor> *neighbors)
{
	if (neighbors->empty()) return 0;

	float sum_of_ratings = 0;
	float sum_of_similarities = 0;
					;
	for (int i = 0; i < neighbors->size(); i++) {
		neighbor x = (*neighbors)[i];
		sum_of_ratings += x.similarity * (x.rating - avg_rating[x.id]);
		sum_of_similarities += x.similarity;
//		cout << " Aggregating " << x.similarity << " and " << x.rating
//						<< " resulting in " << x.similarity * x.rating << endl;
	}

	if (sum_of_similarities == 0) return 0;

	float final_rating = avg_rating[ref_user] + sum_of_ratings / sum_of_similarities;

	if (final_rating < MIN_RATING) return MIN_RATING;
	else if (final_rating > MAX_RATING) return MAX_RATING;

	return final_rating;
}
void UserBased::predict_user_based()
{
	exec_time_start = clock();
	process_ratings();

	for (int i = 0; i < targets.size(); i++)
	{
//			cout << "  Computing similarities..." << endl;
		similarityCalc.compute_similarities(targets[i].user);

//			cout << "  Selecting neighbors..." << endl;
		vector<neighbor> neighbors; neighbors.reserve(EXPECTED_NEIGHBORS);
		select_neighbors(targets[i].user, targets[i].item, &neighbors, 30, 0.0f);

//			cout << "  Aggregating ratings..." << endl;
		float predicted_rating = aggregate_ratings(targets[i].user, targets[i].item, &neighbors);
//			cout << "  Computed rating: " << rating << " for user/item: " << user << ":" << it->id << endl;

		targets[i].value = predicted_rating;

//		if ((i+1) % 5000 == 0) {
//			cout << "  OK: Target " << i + 1 << "/" << targets.size() << ". "; IO::print_exec_time();
//		}
	}
}
