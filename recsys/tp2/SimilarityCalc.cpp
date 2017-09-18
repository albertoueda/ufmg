/*
 * SimilarityCalc.cpp
 *
 *  Created on: 04/10/2015
 *      Author: alberto
 */

#include "SimilarityCalc.h"

SimilarityCalc::SimilarityCalc(){}

SimilarityCalc::~SimilarityCalc(){}


void SimilarityCalc::compute_user_norm(int user)
{
	if (user_words[user].empty()) return;

	float sum = 0;
	for (auto user_word = user_words[user].begin(); user_word != user_words[user].end(); user_word++)
	{
//		cout << "      User [" << user << "] , adding tf-idf: " << user_word->second
//						<< " of word [" << all_ids[user_word->first].word << "]" << endl;
		sum += pow(user_word->second, 2);
	}

	user_norms[user] = sqrt(sum);
//	cout << "    Norm of user [" << user << "] is " << user_norms[user] << endl;
}

void SimilarityCalc::compute_item_norm(int item)
{
	if (contents[item].title.empty()) return;

	float sum = 0;

	for (auto content_word = contents[item].words.begin(); content_word != contents[item].words.end(); content_word++)
	{
		float word_tf_idf = (1 + log10f(content_word->second)) * all_ids[content_word->first].idf;

//		cout << "      Item [" << item << "], adding tf-idf: [" << word_tf_idf << "] = "
//						<< 1 + log10f(content_word->second) << " x " << all_ids[content_word->first].idf
//						<< " , word [" << all_ids[content_word->first].word << "]" << endl;
		sum += pow(word_tf_idf, 2);
	}

	item_norms[item] = sqrt(sum);
//	cout << "    Norm of item [" << item << "] is " << item_norms[item] << endl;
}

float SimilarityCalc::similarity(int user, int item)
{
	if (user_words[user].empty() || contents[item].title.empty()
			   || user_norms[user] == 0 || item_norms[item] == 0) return 0;

	float sum_of_products = 0;

	for (auto content_word = contents[item].words.begin(); content_word != contents[item].words.end(); content_word++)
	{
		auto user_word = user_words[user].find(content_word->first);

		if (user_word == user_words[user].end())
			continue;

		float user_tf_idf = user_word->second;
		float item_tf_idf = (1 + log10f(content_word->second)) * all_ids[content_word->first].idf;

		float rating_product = user_tf_idf * item_tf_idf;
		sum_of_products += rating_product;
//		cout << "      Word [" << all_ids[content_word->first].word << "] (user:" << user << " and item:" << item
//						<< ") --> Rating product: " << rating_product << endl;
	}

	return sum_of_products / (user_norms[user] * item_norms[item]);
}
