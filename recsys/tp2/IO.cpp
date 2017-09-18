/*
 * IO.cpp
 *
 *  Created on: 04/10/2015
 *      Author: alberto
 */

#include "IO.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/error/error.h" // rapidjson::ParseResult
#include "rapidjson/error/en.h"

using namespace rapidjson;

IO::IO() {}

IO::~IO() {}

void IO::load_stopwords(const string filename) {

	ifstream inputFile(filename.c_str());
    string tmp; getline(inputFile, tmp);  // Ignore header line

    for(string stopword; getline(inputFile, stopword); )
        stopwords.insert(stopword);

    inputFile.close();
}

void parse_content(int item, string *item_content)
{
    Document d;
    d.Parse(item_content->c_str());

    if (d.HasParseError()) {
//        	cout << "  Error for content (" << total_entries << "): " << item_content << endl;
//        	cout <<  "  " << GetParseError_En(d.GetParseError()) ;
//        	cout << " (offset: " << d.GetErrorOffset() << ")" << endl << endl;
    	return;
    }

    content new_content;

    if (d.HasMember("Title"))
    	new_content.title = d["Title"].GetString();
    if (d.HasMember("Plot"))
    	new_content.plot = d["Plot"].GetString();
    if (d.HasMember("Country"))
    	new_content.country = d["Country"].GetString();
    if (d.HasMember("Director"))
    	new_content.director = d["Director"].GetString();
    if (d.HasMember("imdbRating"))
    	new_content.imdbRating = atof(d["imdbRating"].GetString());
    if (d.HasMember("Genre"))
    	new_content.genre = d["Genre"].GetString();

    new_content.compile();

    contents[item] = new_content;
//	cout << "  contents[" << item << "] = {" << contents[item].all_words << "}" << endl;
}

void IO::load_content(const string filename) {

	ifstream inputFile(filename.c_str());
    string tmp; getline(inputFile, tmp);  // Ignore header line
    int item, total_entries = 0; string item_content;

    for(string line ; getline(inputFile, line); )
    {
    	total_entries++;
    	int comma_pos = line.find(",");

    	int item = atoi(line.substr(1, comma_pos - 1).c_str());
    	item_content = line.substr(comma_pos + 1);
    	parse_content(item, &item_content);
    }

    total_docs = total_entries;
    inputFile.close();
}

void IO::load_ratings(const string filename)
{
	ifstream inputFile(filename.c_str());
    string tmp; getline(inputFile, tmp);  // Ignore header line

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
}

void IO::load_targets(const string filename)
{
	ifstream inputFile(filename.c_str());
    string tmp; getline(inputFile, tmp);  // Ignore header line

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
	cout << "Time elapsed: " << setiosflags(ios::fixed) << setprecision(2) << exec_time_current << "s" << endl;
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


void IO::print_vocabulary(int step)
{
	cout << "Vocabulary:" << endl;
	for (int i = 0; i < all_ids.size(); i+=1) {
		if (all_ids[i].id == 0) continue;
		cout << " |" << all_ids[i] << endl;

		if (i > 200) i += step;
	}
	cout << endl;
}

void IO::print_vocabulary()
{
	print_vocabulary(0);
}

bool word_info_compare(word_info a, word_info b)
{
	return a.count > b.count;
}

// CAUTION: This will end with the word ids correspondence. To be used only for tests.
void IO::print_sorted_vocabulary(int step)
{
	sort(all_ids.begin(), all_ids.end(), word_info_compare);
	print_vocabulary(step);
}

