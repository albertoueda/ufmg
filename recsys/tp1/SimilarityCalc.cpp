/*
 * SimilarityCalc.cpp
 *
 *  Created on: 04/10/2015
 *      Author: alberto
 */

#include "SimilarityCalc.h"

SimilarityCalc::SimilarityCalc(){}

SimilarityCalc::~SimilarityCalc(){}


void SimilarityCalc::compute_norm(int user)
{
	int ratings_sum = 0;
	for (int i = 0; i < ratings[user].size(); i++)
	{
		if (ratings[user][i].value == -1) continue;

		ratings_sum += pow(ratings[user][i].value, 2);
	}

	norms[user] = sqrt(ratings_sum);
//	cout << "  Norm of " << user << " is " << norms[user] << endl;
}

float SimilarityCalc::similarity(int a, int b)
{
	if (a == b || ratings[a].empty() || ratings[b].empty()
			   || norms[a] == 0 || norms[b] == 0) return 0;

	int i = 0, j = 0, item_a, item_b;
	float sum_of_products = 0;

	while (i < ratings[a].size() && j < ratings[b].size())
	{
		item_a = ratings[a][i].id;
		item_b = ratings[b][j].id;

		for (; item_b < item_a && j < ratings[b].size(); item_b = ratings[b][++j].id);
		for (; item_a < item_b && i < ratings[a].size(); item_a = ratings[a][++i].id);

		if (item_a == item_b && i < ratings[a].size() && j < ratings[b].size()) {
			float rating_product = ratings[a][i].value * ratings[b][j].value;
			sum_of_products += rating_product;
//			cout << "  Item " << item_a << " (" << a << " and " << b
//							<< ") rating product: " << rating_product << endl;

			if (ratings[a].size() < ratings[b].size()) // TODO try difference
				i++;
			else
				j++;
		}
	}

	return sum_of_products / (norms[a] * norms[b]);
}

bool compare_similarities (neighbor a, neighbor b)
{
	return (a.similarity > b.similarity);
}

void SimilarityCalc::compute_similarities(int ref_user)
{
	// Check if the similarities are already computed TODO test this
	if (all_similarities[ref_user].empty() == false)
		return;

	// Init
	all_similarities[ref_user].resize(max_user_id + 1);
	for (int i = 0; i <= max_user_id; i++) all_similarities[ref_user][i].similarity = 0;

	for (int other_user = 0; other_user <= max_user_id; other_user++)
	{
		float similarity_score = similarity(ref_user, other_user);
//		cout << "    User " << other_user << " x ref_user " <<
//						ref_user << ": " << similarity_score << endl;

		all_similarities[ref_user][other_user].id = other_user;
		all_similarities[ref_user][other_user].similarity = similarity_score;
	}

//	OBS: Not necessary anymore
//	sort(all_similarities[ref_user].begin(),
//					all_similarities[ref_user].end(), compare_similarities);
}
