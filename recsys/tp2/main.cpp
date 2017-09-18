//============================================================================
// Name        : main.cpp
// Author      : Alberto Hideki Ueda
// Author ID   : 2015722801
// Description : RecSys Programming Assignment #2
//				 Content-based Movie Recommendation
// 				 Universidade Federal de Minas Gerais
//============================================================================

#include "IO.h"
#include "RatingManager.h"
#include "ContentBased.h"

vector<vector<rating > > ratings;
vector<vector<rating > > ratings_r;
vector<target> targets;
vector<float> user_norms;
vector<float> item_norms;
vector<float> avg_rating;
vector<content> contents;

int max_user_id = 0;//, total_users = 0;
float avg_global_rating = 0.0f;
int total_docs = 0;
clock_t exec_time_start;

void init_data()
{
	ratings.resize(M);
	ratings_r.resize(N);
	targets.reserve(EXPECTED_TARGETS);
	user_norms.resize(M);
	item_norms.resize(N);
	avg_rating.resize(M);
	contents.resize(N);
}

void predict()
{
	ContentBased contentBased;
	contentBased.predict();
	contentBased.predict_by_imdb_rating();

	RatingManager ratingsManager;
	ratingsManager.predict_by_user_average();
	ratingsManager.predict_with_no_info();
}

int main(int argc, char** argv)
{
	if (argc < 4) {
		cout << "Please inform the path of the three CSVs (content, ratings and targets) to initialize the system." << endl;
		return 1;
	}

	exec_time_start = clock();
	if (DEBUG) cout << "Initializing system..." << endl;
	init_data();

	if (DEBUG) cout << "Loading content..." << endl;
	IO::load_content(argv[1]);

	if (DEBUG) cout << "Loading stopwords..." << endl;
	IO::load_stopwords("data/stopwords.txt");

	if (DEBUG) cout << "Loading ratings..." << endl;
	IO::load_ratings(argv[2]);

	if (DEBUG) cout << "Loading targets..." << endl;
	IO::load_targets(argv[3]);

	if (DEBUG) cout << "Predicting ratings..." << endl;
	predict();

	if (DEBUG) cout << "Writing predictions..." << endl;

	if (DEBUG)
		IO::write_predictions("data/parrudos/submission.csv");
	else
		IO::print_predictions();

//	if (DEBUG) IO::print_sorted_vocabulary(1000);
	if (DEBUG) cout << "The program has finished." << endl;
}
