/* g++ acr.cpp -o acr -O3 -std=c++11 && time ./acr ../data2/model02mc/graph.inlinks.example ../data2/acr/results-head-10.tsv
 * grep ",0$" output-rcr.csv -c
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <string>
#include <list>
#include <cmath>
#include <inttypes.h>
#include <sstream>
#include <chrono>
#include <algorithm>
#include <set>
#include <unordered_set>
#include <unordered_map>
using namespace std;

const unsigned M = 30; //4294967295; // 4GB
const unsigned N = 1000; //4294967295; // 4GB
const float MAX_DCG_10 = 6.55497;

int n_candidates = 0;
std::chrono::time_point<std::chrono::steady_clock> start;
vector< set<string >> relevant(N); // researcher -> relevant papers
unordered_set<string> candidates(N);
unordered_map <string, unordered_set <string> > inlinks(N);
unordered_map <string, unordered_set <string> > outlinks(N);
unordered_map <string, unordered_set <string> > bros(N);
unordered_map <string, int > acr(N); // TODO default zero
unordered_map <string, float > rcr(N);
unordered_map <string, float > avg_neighbor_acr(N);

struct compare_suggestions {
    bool operator() (const string &a, const string &b) const {
    	if (rcr.count(a) == 0)
    		return false;
    	else if (rcr.count(b) == 0)
    		return true;

    	return rcr.at(a) > rcr.at(b);
    }
};

vector< set <string, compare_suggestions >> user_bros(M); // researcher -> neighborhood papers // TODO ordered!

using get_time = chrono::steady_clock;
void print_time( std::chrono::time_point<std::chrono::steady_clock> start )
{
    cout << "  " << chrono::duration_cast<chrono::seconds>(get_time::now() - start).count() <<" seconds" << endl;
}

void read()
{
	start = get_time::now();

	cout << "  Reading relevant papers..." << endl;
    int user_id = 1;
	for (int i = 1; i <= 15; i++) {
		ifstream f2("20100825-SchPaperRecData/JuniorR/y" + to_string(i) + "/y" + to_string(i) + "-rlv.txt");
		string s; set<string> empty;
		relevant[user_id] = empty;
		while (f2 >> s) {
			relevant[user_id].insert(s);
			// cout << "Added [" << s << "] to user [" << user_id << "]" << endl;
		}
		f2.close();
		user_id++;
	}
	for (int i = 1; i <= 13; i++) {
		ifstream f2("20100825-SchPaperRecData/SeniorR/m" + to_string(i) + "/m" + to_string(i) + "-rlv.txt");
		string s; set<string> empty;
		relevant[user_id] = empty;
		while (f2 >> s) {
			relevant[user_id].insert(s);
			// cout << "Now added [" << s << "] to user [" << user_id << "]" << endl;
		}
		f2.close();
		user_id++;
	}
	print_time(start);

	cout << "  Reading candidate papers..." << endl;
	ifstream f("creuza/candidates.txt");
	string candidate_paper;
	while (f >> candidate_paper) {
		candidates.insert(candidate_paper); // TODO pq nao estoura?
		unordered_set<string> empty;
		bros.insert({candidate_paper, empty});
		n_candidates++;
		// cout << " [" << s << "]";
	}
	f.close();
	print_time(start);

	cout << "  Reading graph..." << endl;
	ifstream fg("creuza/acl.graph.csv");//acl.graph.csv acl.20080325.net.csv") acl.graph.only.1140;
	string s, t;
	while(getline(fg, s, ',')) {
		getline(fg, t);

		// acr.insert({t, 1}); // 	TODO
//		cout << "  Considering line " << s << " --> " << t << endl;
		// basic inlinks
		auto inlinks_it = inlinks.find(t);
		if (inlinks_it == inlinks.end()) {
			unordered_set<string> first_inlink({s});
			inlinks.insert({t, first_inlink});
			acr.insert({t, 1});
//			if (t == "P06-1140")
//				cout << "Inserting NEW inlink of [" << t << "]: " << s << endl;
		} else {
			inlinks_it->second.insert(s);
			acr.at(t)++;
//			if (t == "P06-1140")
//			cout << "  (" << acr.at(t) << ") Inserting other inlink of ["
//					<< t << "]: " << s << endl;
		}

		// extra inlinks // TODO could be an improvement
//		inlinks_it = inlinks.find(s);
//		if (inlinks_it == inlinks.end()) {
//			unordered_set<string> extra_inlink({t});
//			inlinks.insert({s, extra_inlink});
//			acr.insert({s, 0});
////			if (s == "P06-1140")
////				cout << "Inserting NEW EXTRA inlink of [" << s << "]: " << t << endl;
//		} else {
//			inlinks_it->second.insert(t);
////			if (s == "P06-1140")
////				cout << "Inserting other EXTRA inlink of [" << s << "]: " << t << endl;
//		}

		// outlinks
		auto outlinks_it = outlinks.find(s);
		if (outlinks_it == outlinks.end()) {
			unordered_set<string> first_outlink({t});
			outlinks.insert({s, first_outlink});
			// if (t == "J96-2004")
//			cout << "Inserting NEW outlink of [" << s << "]: " << t << endl;
		} else {
			outlinks_it->second.insert(t);
			//if (t == "J96-2004")
//				cout << "Inserting other outlink of ["
//					<< s << "]: " << t << endl;
		}
	}
	fg.close();
	print_time(start);
}

void calculate_rcr()
{
	start = get_time::now();
	int k = 0, no_inlinks = 0, no_outlinks = 0, no_acr = 0;

	for (auto candidates_it = candidates.begin(); candidates_it != candidates.end(); candidates_it++, k++)
	{
		if (k % 100 == 0) {
			cout << "  Calculating RCR of relevant paper #" << k << " [" << *candidates_it << "]...";
			print_time(start);
		}

//		if (k == 10) break;

		string target_paper = *candidates_it; // relevant[user_id][paper_index];
		if (inlinks.count(target_paper) == 0) {
//			cout << "    Paper " << target_paper << " not found in inlinks." << endl;
			no_inlinks++;
			continue;
		}

		// unsigned n_parents = inlinks.at(target_paper).size();
		float sum = 0;
		unsigned count_bros = 0;

		for (auto inlinks_it = inlinks.at(target_paper).begin();
						inlinks_it != inlinks.at(target_paper).end(); inlinks_it++)
		{
			string parent_node = *inlinks_it;
//			cout << "    Considering parent: " << parent_node << endl;

			if (outlinks.count(parent_node) == 0) {
//				cout << "      Paper " << parent_node << " not found in outlinks." << endl;
				no_outlinks++;
				continue;
			}

//			unordered_set <string> new_bros;
			for (auto outlinks_it = outlinks.at(parent_node).begin();
							outlinks_it != outlinks.at(parent_node).end(); outlinks_it++)
			{
				string bro = *outlinks_it;
				if (target_paper != bro && acr.count(bro) > 0) {
//					cout << "      Considering bro: " << bro << " (acr = " << acr.at(bro) << ")" << endl;

					// Add to paper bros only candidates
					if (candidates.count(bro) > 0)
						bros.at(target_paper).insert(bro);

					sum += acr.at(bro);
					count_bros++;
				}
			}
		}

		if (count_bros == 0)
		  avg_neighbor_acr.insert({target_paper, 0});
		else
		  avg_neighbor_acr.insert({target_paper, sum / (count_bros * 1.0)});
	}

	for (auto candidates_it = candidates.begin(); candidates_it != candidates.end(); candidates_it++, k++)
	{
		float candidate_rcr;
        if (acr.count(*candidates_it) == 0 || acr.at(*candidates_it) == 0) {
//        	cout << "  No acr found for paper: " << *candidates_it << endl;
        	if (avg_neighbor_acr.count(*candidates_it) == 0 || avg_neighbor_acr.at(*candidates_it) == 0)
        		candidate_rcr = 1; // TODO think about it
        	else
        		candidate_rcr = 1 / avg_neighbor_acr.at(*candidates_it) * 1.0;
            no_acr++;
        }
        else if (avg_neighbor_acr.count(*candidates_it) == 0 || avg_neighbor_acr.at(*candidates_it) == 0)
            candidate_rcr = 1;
        else {
            candidate_rcr = acr.at(*candidates_it) / avg_neighbor_acr.at(*candidates_it) * 1.0;
//            cout << "rcr of [" << *candidates_it << "] = " << acr.at(*candidates_it) << " / "
//            	 << avg_neighbor_acr.at(*candidates_it) << endl;
        }

        rcr.insert({*candidates_it, candidate_rcr});
//		cout << *candidates_it << "," << candidate_rcr << endl;
	}

	cout << "\nTotal of candidates: " << n_candidates << endl;
	cout << "Number of candidates without ACR: " << no_acr << endl;
	cout << "Number of candidates without inlinks: " << no_inlinks << endl;
	cout << "Number of parents without outlinks (may be duplicated): " << no_outlinks << endl;

}

void map_user_bros()
{
	start = get_time::now();
	cout << "Writing output..." << endl;
	ofstream fout("creuza/output.csv", ofstream::out);
	float sum_ndcgs = 0, sum_p = 0, sum_r = 0;

	for (int user_id = 0; user_id < relevant.size(); user_id++) {
		if (relevant[user_id].size() == 0) continue;

		// Add sorted recommendations
		set<string, compare_suggestions> recommendations;
		for (auto it = relevant[user_id].begin(); it != relevant[user_id].end(); it++) {
			string user_paper = *it;
			recommendations.insert(bros.at(user_paper).begin(), bros.at(user_paper).end());
		}
		user_bros[user_id] = recommendations;

		// Print results
//		fout << "User #" << user_id << ", #input: " << relevant[user_id].size();
//		fout << ", recommendations: " << user_bros[user_id].size() << endl;
		int k = 0, max = 20, hits = 0;
		float dcg = 0;

		for (auto bro = user_bros[user_id].begin(); bro != user_bros[user_id].end(); bro++) {
//			fout << *bro << "," << rcr.at(*bro);
			if (relevant[user_id].count(*bro) > 0) {
//				fout << " [*]";
				hits++;
				dcg += 1 / log(k + 2); // first k = 0
			}
//		    fout << endl;
			if(++k == max) {
				break;
			}
		}

		float ndcg = dcg/(MAX_DCG_10*1.0);
		float prec =  hits/(10*1.0);
		float rec = hits/(relevant[user_id].size()*1.0);
		sum_ndcgs += ndcg;
		sum_p += prec;
		sum_r += rec;

//		fout << "  User #" << user_id << ", P@10 = [" << prec << "]"
//				<< ", R@10 = [" << rec << "]"
//				<< ", nDCG@10 = [" << ndcg << "]" << endl;

		fout << user_id
			 << "," << prec
			 << "," << rec
			 << "," << ndcg << endl;
	}

	fout << "\nAverage nDCGs: " <<  sum_ndcgs / (28) << endl;
	fout << "Average Precision: " <<  sum_p / (28) << endl;
	fout << "Average Recall: " <<  sum_r / (28) << endl;
	fout.close();
}

int main(int argc, char ** argv)
{
    cout << "Initializing..." << endl;

    cout << "Reading files..." << endl;
    read();

	cout << "Calculating RCRs..." << endl;
	calculate_rcr();

	cout << "Mapping user neighboorhood..." << endl;
	map_user_bros();

	cout << "\nProgram finished." << endl;

	return 0;
}
