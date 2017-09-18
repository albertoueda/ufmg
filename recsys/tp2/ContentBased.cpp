/*
 * ContentBased.cpp
 *
 *  Created on: 26/10/2015
 *      Author: alberto
 */

#include "ContentBased.h"
#include "IO.h"
#include <sstream>

unordered_map<string, int> words; // string with word -> word_id
vector<word_info> all_ids; // word_id -> word_info
vector< unordered_map<int, float> > user_words; // user -> map < word_id, user_weighted_word_tf_idf>
unordered_set<string> stopwords;

ContentBased::ContentBased()
{}

ContentBased::~ContentBased()
{}

void init()
{
	all_ids.resize(200000);
	user_words.resize(M);
}

void split(const string &s, char delim, vector<string> &elems) {
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
}

/**
 *  From each content c of the dataset, process all words of c gathering
 *  the relevant information for the future computation of words' tfs and idfs.
 */
void model_items()
{
	int new_word_id = 1; // First word id = 1. It will not use the index 0

	for (int item = 0; item < contents.size(); item++)
	{
		if (contents[item].title.empty())
			continue;

		vector<string> item_words;
		split(contents[item].all_words, ' ', item_words);

		for (int k = 0; k < item_words.size(); k++)
		{
			string word = item_words[k];

			if (stopwords.count(word) > 0)
				continue;

			auto words_iterator = words.find(word);

			if (words_iterator == words.end()) {
				// word not found in vocabulary
				word_info new_word_info(word, new_word_id, 1);
				new_word_info.tf.insert({item, 1});

				words.insert({word, new_word_id});
				all_ids[new_word_id] = new_word_info;
				contents[item].words.insert({new_word_id, 1});

				new_word_id++;
			}
			else {
				// vocabulary contains the word
				int found_id = words_iterator->second;
				all_ids[found_id].count++;

				auto tf_iterator = all_ids[found_id].tf.find(item);

				if (tf_iterator == all_ids[found_id].tf.end()) {
					// 1st ocurrence of word in this doc
					all_ids[found_id].tf.insert({item, 1});
					contents[item].words.insert({found_id, 1});
				} else {
					// the word has been found earlier in this doc
					tf_iterator->second++;
					contents[item].words.at(found_id)++;
				}
			}
		}
	}
}

/*
 * Update some attributes of the words *after* they are mapped, such as the IDFs of the terms.
 */
void process_words()
{
	for (int i = 0; i < all_ids.size(); i++) {
		if (all_ids[i].id == 0) continue;
		all_ids[i].idf = 1 + log10f(total_docs / (all_ids[i].tf.size() * 1.0));
	}
}

/*
 * Model each user according to his ratings (weights) and respective tf/idf of item terms.
 */
void model_users()
{
	for (int item = 0; item < contents.size(); item++)
	{
		if (DEBUG && (item-1) % 1000000 == 0) {
			cout << "    OK: Content " << item - 1 << "/" << contents.size() << ". ";
			IO::print_exec_time();
		}

		if (contents[item].title.empty() || ratings_r[item].empty()) continue;

		for (int i = 0; i < ratings_r[item].size(); i++)
		{
			int user_id = ratings_r[item][i].id;
			int user_rating = ratings_r[item][i].value;

//			cout << "    Using user [" << user_id << "] with tf-idfs [" << user_rating << "x] :" << endl;
			for (auto x = contents[item].words.begin(); x != contents[item].words.end(); x++)
			{
				int word_id = x->first;
				word_info *user_word_info = &all_ids[word_id];

				float item_tf = 1 + log10f(user_word_info->tf.at(item));
				float item_idf = user_word_info->idf;

				float weighted_word_tf_idf = user_rating * (item_tf * item_idf);

//				if (user_word_info->word == "the") cout << "      User [" << user_id << "], word [" << user_word_info->word
//								<< "] --> tf[" << item_tf << "] x idf[" << item_idf << "] = "
//								<< weighted_word_tf_idf << endl;

				// Check if the word info already contains the user id
				auto tf_users_iterator = user_word_info->tf_users.find(user_id);

				if (tf_users_iterator == user_word_info->tf_users.end()) {
					user_word_info->tf_users.insert({user_id, weighted_word_tf_idf});
				} else {
					tf_users_iterator->second += weighted_word_tf_idf;
				}
			}

		}
	}
}

/*
 * Process some attributes of the users *after* they are modeled, such as the user-oriented vector of terms.
 */
void process_users()
{
	for (int word_id = 0; word_id < all_ids.size(); word_id++)
	{
		if (DEBUG && (word_id-1) % 100000 == 0) {
			cout << "    OK: word " << word_id - 1 << "/" << all_ids.size() << ". ";
			IO::print_exec_time();
		}

		if (all_ids[word_id].id == 0) continue;

		for (auto tf_user = all_ids[word_id].tf_users.begin(); tf_user != all_ids[word_id].tf_users.end(); tf_user++) {
			int user_id = tf_user->first;
			float weighted_word_tf_idf = tf_user->second;
//			cout << "    Compiled user: " << user_id <<
//							" , word = [" << all_ids[word_id].id << ":" << all_ids[word_id].word
//							<< "], tf-idf = " << weighted_word_tf_idf << endl;

			user_words[user_id].insert({word_id, weighted_word_tf_idf});
		}
	}
}

void ContentBased::preprocess_user(int user_id)
{
	if (user_norms[user_id] > 0 || user_words[user_id].empty())
		return;

	similarityCalc.compute_user_norm(user_id);
	avg_rating[user_id] = ratingManager.get_user_avg(user_id);
}

void ContentBased::preprocess_item(int item_id)
{
	if (item_norms[item_id] > 0 || contents[item_id].title.empty())
		return;

	similarityCalc.compute_item_norm(item_id);
}

void ContentBased::compute_targets_similarities()
{
	for (int i = 0; i < targets.size(); i++)
	{
		preprocess_user(targets[i].user);
		preprocess_item(targets[i].item);

		float similarity = similarityCalc.similarity(targets[i].user, targets[i].item);
//		cout << "    Computed similarity: [" << similarity << "] for user/item: " << targets[i].user << "/" << targets[i].item << endl;

		float user_avg = avg_rating[targets[i].user];

		if (user_avg != 0 && similarity >= 1) {
			targets[i].value = user_avg + similarity;
			if (targets[i].value > MAX_RATING) targets[i].value = MAX_RATING;
		}
		else if (user_avg != 0 && similarity < 1) {
			targets[i].value = user_avg - similarity;
			if (targets[i].value < 0) targets[i].value = 0;
		}

		if (DEBUG && (i+1) % 5000 == 0) {
			cout << "  OK: Target " << i + 1 << "/" << targets.size() << ". ";
			IO::print_exec_time();
		}
	}
}

/**
 * Use imdb rating only if the similarity metric was not good enough (e.g. similarity equals zero).
 */
void ContentBased::predict_by_imdb_rating()
{
	for (int i = 0; i < targets.size(); i++)
	{
		if (targets[i].value > 0 || contents[i].imdbRating == 0) continue;

		targets[i].value = contents[i].imdbRating;
	}
}

void ContentBased::predict()
{
	init();

	if (DEBUG) cout << "  Mapping words..." << endl;
	model_items();
	process_words();

	if (DEBUG) cout << "    Total of dintinct words: " << words.size() << endl;
	if (DEBUG) cout << "    Total of documents: " << total_docs << endl;

	if (DEBUG) cout << "  Mapping users..." << endl;
	model_users();

	if (DEBUG) cout << "  Compiling users..." << endl;
	process_users();

	exec_time_start = clock();
	if (DEBUG) cout << "  Computing similarities..." << endl;
	compute_targets_similarities();
}
