/*
 * IO.cpp
 *
 *  Created on: 04/10/2015
 *      Author: alberto
 */

#include "IO.h"

IO::IO()
{
}

IO::~IO()
{
}

void IO::load_ratings(const string filename)
{
	ifstream inputFile(filename.c_str());
    string tmp; getline(inputFile, tmp);  // Ignore first line

    int user, item, rating, sum_of_ratings = 0, total_entries = 0;

    for(string line ; getline(inputFile, line); )
    {
    	int colon_pos = line.find(":");
    	int comma_pos = line.find(",");
    	int another_comma_pos = line.find(",", comma_pos + 1);

    	int user = atoi(line.substr(1, colon_pos - 1).c_str());
    	int item = atoi(line.substr(colon_pos + 2, comma_pos - (colon_pos + 2)).c_str());
    	int rating = atoi(line.substr(comma_pos + 1, another_comma_pos - (colon_pos + 1)).c_str());

    	if (user > max_user_id) max_user_id = user;
    	sum_of_ratings += rating;
    	total_entries++;

        // cout << user << " and " << item << " and " << rating << endl;
       	ratings[user].emplace_back(item, rating);
       	ratings_r[item].emplace_back(user, rating);
    }

    inputFile.close();

    avg_global_rating = 1.0f * sum_of_ratings / total_entries;
//    avg_global_rating = 7.0f;
}

void IO::load_targets(const string filename)
{
	ifstream inputFile(filename.c_str());
    string tmp; getline(inputFile, tmp);  // Ignore first line

    int user, item;

    for(string line ; getline(inputFile, line); )
    {
    	int colon_pos = line.find(":");

    	int user = atoi(line.substr(1, colon_pos - 1).c_str());
    	int item = atoi(line.substr(colon_pos + 2).c_str());

//        cout << user << " and " << item << endl;
       	targets.emplace_back(user, item, -1);
    }

    inputFile.close();
}


void IO::print_exec_time() {
	double exec_time_current = (clock() - exec_time_start) / (double) CLOCKS_PER_SEC;
	cout << "Time elapsed: " << setiosflags(ios::fixed) << setprecision(1) << exec_time_current << "s" << endl;
}

void IO::print_ratings(int n, bool enable_reversed)
{
	cout << "Ratings:" << endl;
	for (int i = 0, k = 0; i < ratings.size() && k < n; i++)
	{
		if (ratings[i].empty()) continue;
		k++;

		cout <<  setw(6) << i << ": ";
		for (int j = 0; j < ratings[i].size(); j++)
		{
			rating r = ratings[i][j];
			cout << setw(6) << r.id << ":" << r.value;
		}
		cout << endl;
	}

	if (enable_reversed) {
		cout << "Ratings_R:" << endl;
		for (int i = 0, k = 0; i < ratings_r.size() && k < n; i++)
		{
			if (ratings_r[i].empty()) continue;
			k++;

			cout <<  setw(9) << i << ": ";
			for (int j = 0; j < ratings_r[i].size(); j++)
			{
				rating r = ratings_r[i][j];
				cout << setw(9) << r.id << ":" << r.value;
			}
			cout << endl;
		}
	}
}

void IO::print_ratings()
{
	print_ratings(100, false);
}

// Users and items with all final ratings equal zero:
// u0009222 u0000139  u0000271  u0000335
// 165 u0002769:  85 u0011814: 83 u0027182:
// 3 i2820852, //3 i2515034, //3 i2395427,
void IO::print_similarities(int n, int m)
{
	cout << "Similarities:" << endl;
	for (int user = 0, count_users = 0; user <= max_user_id && count_users < n; user++)
	{
		if (all_similarities[user].empty()) continue;
		count_users++;

		cout <<  setw(6) << user << ": ";
		for (int i = 0, count_results = 0; i < all_similarities[user].size() && count_results < m; i++)
		{
			if (user == all_similarities[user][i].id) continue;

			float similarity_score = all_similarities[user][i].similarity;
			if (similarity_score == 0) continue;

			count_results++;
			cout << setw(6) << all_similarities[user][i].id << ":"
							<< setiosflags(ios::fixed) << setprecision(5) << similarity_score;
		}
		cout << endl;
	}
}

void IO::write_predictions(const string filename)
{
	FILE *outputFile;
	outputFile = fopen(filename.c_str(), "w+");
	fprintf(outputFile, "UserId:ItemId,Prediction\n");

	for (int i = 0; i < targets.size(); i++) {
		target prediction = targets[i];
		fprintf(outputFile, "u%07d:i%07d,%f\n", prediction.user, prediction.item, prediction.value);
	}

	fclose(outputFile);
}

void IO::print_predictions()
{
	cout << "UserId:ItemId,Prediction" << endl;
	for (int i = 0; i < targets.size(); i++) {
		target prediction = targets[i];
		printf("u%07d:i%07d,%f\n", prediction.user, prediction.item, prediction.value);
	}
}
