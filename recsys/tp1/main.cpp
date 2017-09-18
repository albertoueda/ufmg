//============================================================================
// Name        : main.cpp
// Author      : Alberto Hideki Ueda
// Author ID   : 2015722801
// Description : RecSys Programming Assignment #1
//				 Collaborative Movie Recommendation
// 				 Universidade Federal de Minas Gerais
//============================================================================

#include "IO.h"
#include "UserBased.h"
#include "RatingManager.h"

vector<vector<rating > > ratings;
vector<vector<rating > > ratings_r;
vector<target> targets;
vector<float> norms;
vector<float> avg_rating;

vector<vector<neighbor> > all_similarities;

int max_user_id = 0;//, total_users = 0;
float avg_global_rating = 0.0f;
clock_t exec_time_start;

void init_data()
{
	ratings.resize(M); // reserve?
	ratings_r.resize(M); // actually is not M
	targets.reserve(EXPECTED_TARGETS);
	all_similarities.resize(M);
	norms.resize(M);
	avg_rating.resize(M);
}

void predict()
{
	UserBased userBasedRec;
	userBasedRec.predict_user_based();

	RatingManager ratingsManager;
	ratingsManager.predict_by_averages();
}

int main(int argc, char** argv)
{
	if (argc < 3) {
		cout << "Please inform the path of two CSVs (ratings and targets) to initialize the system." << endl;
		return 1;
	}

//	cout << "Initializing system..." << endl;
	exec_time_start = clock();
	init_data();

//	cout << "Loading ratings..." << endl;
	IO::load_ratings(argv[1]);
//	IO::print_ratings(100, false);

//	cout << "Loading targets..." << endl;
	IO::load_targets(argv[2]);

//	cout << "Predicting ratings..." << endl;
	predict();

//	cout << "Writing predictions..." << endl;
//	IO::write_predictions("data/parrudos/submission.csv");
	IO::print_predictions();

//	cout << "Program finished." << endl;
}
