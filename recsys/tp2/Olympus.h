/*
 * Olympus.h
 *
 *  Created on: 04/10/2015
 *      Author: alberto
 */

#ifndef OLYMPUS_H_
#define OLYMPUS_H_

#include <cstdlib>
#include <iostream>
#include <cctype>
// ifstream
#include <iomanip>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <ctime>
#include <math.h>
#include <algorithm> // std::sort, std::remove_if
#include <unordered_map>
#include <unordered_set>

#include "rapidjson/document.h"

// Greatest user ID: 38729
// Greatest item ID: 4967094

using namespace rapidjson;
using namespace std;

#define DEBUG 0

#define M 40000  // max number of users
#define N 5000000  // max number of users

#define EXPECTED_NEIGHBORS 10000000
#define EXPECTED_TARGETS 80000
#define MIN_RATING 0
#define MAX_RATING 10
#define NO_INFO_RATING_PREDICTION 7.25
#define iter vector<rating>::iterator

// Could be id of user or a item, dependending on the use
// (e.g. ratings_r uses item_id as vector index and user_id as id)
struct rating {
	int id;
	float value;

	rating(int p_id, float p_value) : id(p_id), value(p_value) {};
};

struct target {
	int user;
	int item;
	float value;

	target(int p_user, int p_item, float p_value) : user(p_user), item(p_item), value(p_value) {};
};

struct neighbor {
	int id;
	float rating;
	float similarity;

	neighbor() {};
	neighbor(int p_id, float p_rating, float p_similarity) : id(p_id), rating(p_rating), similarity(p_similarity) {};
};


struct content {
	string title;
	string plot;
	string country;
	string director;
	string genre;
	double imdbRating;

	string all_words;

	// word -> tf-idf in content
	unordered_map<int, int> words;

	content() {};
	content(string p_title, string p_plot, string p_country, string p_director, double p_imdbRating) :
		title(p_title), plot(p_plot), country(p_country), director(p_director), imdbRating(p_imdbRating) {};

	void compile() {
		all_words = title + " " + genre + " " + director;// + " " + plot;// + " " + genre; country
		std::transform(all_words.begin(), all_words.end(), all_words.begin(), ::tolower);
		all_words.erase(std::remove_if(all_words.begin(), all_words.end(),
						[](char c) { return !std::isalnum(c) && !std::isspace(c); }), all_words.end());
	}
};

inline std::ostream& operator<<(std::ostream &strm, const content &c) {
	return strm << c.title << "___" << c.director << "___" << c.country;
}


struct word_info {
	string word;
	int id;
	int count;
	float idf;
	unordered_map<int, int> tf; // DocId -> Term Frequency
	unordered_map<int, float> tf_users; // UserId -> TF-IDF

	word_info() {};
	word_info (string p_word, int p_id, int p_count) : word(p_word), id(p_id), count(p_count) {};
};

inline std::ostream& operator<<(std::ostream &strm, const word_info &info) {
	return strm << info.id << ":" << info.word << ":" << info.count << ":" << info.idf;
}

extern vector<vector<rating > > ratings;
extern vector<vector<rating > > ratings_r;
extern vector<target> targets;
extern vector<vector<neighbor> > all_similarities;
extern vector<float> user_norms;
extern vector<float> item_norms;
extern vector<float> avg_rating;

extern vector<content> contents;

extern unordered_map<string, int> words; // string with word -> word_id
extern vector<word_info> all_ids; // word_id -> word_info
extern vector<unordered_map<int, float> > user_words; // user -> map < word_id, user_weighted_word_tf_idf>
extern unordered_set<string> stopwords;

extern int max_user_id;
extern float avg_global_rating;
extern int total_docs;

extern clock_t exec_time_start;

#endif /* OLYMPUS_H_ */
