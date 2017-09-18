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
// ifstream
#include <iomanip>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <ctime>
#include <math.h>

// std::sort (allowed by teacher assistant)
#include <algorithm>

using namespace std;

#define M 10000000  // max number of users
#define EXPECTED_NEIGHBORS 10
#define EXPECTED_TARGETS 80000
#define MIN_RATING 0
#define MAX_RATING 10
#define iter vector<rating>::iterator

// Could be id of user or a item
struct rating {
	int id;
	float value;

	rating(int p_id, float p_value) : id(p_id), value(p_value) {};
};

// Could be id of user or a item
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

extern vector<vector<rating > > ratings;
extern vector<vector<rating > > ratings_r;
extern vector<target> targets;
extern vector<float> norms;
extern vector<float> avg_rating;

extern vector<vector<neighbor> > all_similarities;

extern int max_user_id;//, total_users = 0;
extern float avg_global_rating;
extern clock_t exec_time_start;

#endif /* OLYMPUS_H_ */
