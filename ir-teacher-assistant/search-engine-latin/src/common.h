/*
 * common.h
 *
 *  Created on: Mar 18, 2013
 *      
 *
 * This file has common structures and methods for all application
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <string>
#include <cstring>
#include <algorithm>
#include <thread>
#include <cassert>
#include <getopt.h>
#include <iostream>
#include <sstream>
#include <cstdio>


using namespace std;

typedef unsigned int ui;
typedef unsigned long ul;


const ui KBYTE = 1024;
const ui MBYTE = KBYTE * 1024;
const ui GBYTE = MBYTE * 1024;


template<typename Container> string sequenceToString(const Container &c)
{
	stringstream ss;
	for(typename Container::const_iterator it = c.begin(); it != c.end(); ++it){
		ss << *it << " ";
	}

	return ss.str();
}

/**
 * Splits the vector in nthreads+1 parts and sorts this parts in pararellal.
 * The merge step is done in sequential
 *
 * @nthreads the numer of threads that can be used.
 * @buffer vector of type T containing data.
 * @size number of items to sort.
 */
template <typename T> void pararellalSort(ui nthreads, vector<T>& buffer, ui size=0){
	ui block = size/(nthreads+1);
	std::vector<std::thread> threads;
	for(ui i = 0; i < nthreads; ++i){
		threads.push_back(std::thread([=, &buffer](){
			ui shift = i*block;
			sort(buffer.begin() + shift, buffer.begin()+ shift +block);
		}));
	}
	sort(buffer.begin() + (nthreads)*block, buffer.begin()+size);

	for(auto& thread : threads){
		thread.join();
	}

	//This part is done in sequential
	for(ui i = 0; i < nthreads; ++i){
		ui shift = i*block;
		inplace_merge(buffer.begin(), buffer.begin()+ shift, buffer.begin()+ shift +block);
	}
	inplace_merge(buffer.begin(),  buffer.begin() + nthreads*block, buffer.begin()+size);

	//assert(is_sorted(buffer.begin(), buffer.begin() + size));
}

const float EPS = 1e-15;
inline int cmp(float x, float y = 0.0, float tol = EPS)
{
    return ( x <= y + tol ) ? ( x + tol < y ) ? -1 : 0 : 1;
}

inline string exec(string &cmd){
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "ERROR";
    char buffer[128];
    stringstream result;
    while(!feof(pipe)) {
    	if(fgets(buffer, 128, pipe) != NULL){
    		result.write(buffer, strlen(buffer));
    	}
    }
    pclose(pipe);
    return result.str();
}

/**
 * Stores the parameters of the application
 */
class Config{
public:
	/**
	 * Singleton class.
	 */
	static Config* Instance(void){
		if(config == NULL){
			config = new Config();
		}
		return config;
	}

	~Config(){
		if(config != NULL){
			delete config;
			config = NULL;
		}
	}

	//The definitions of the variables are in HELP variable defined bellow.
	string runsPosfix;
	string docInfoName;
	string tuplesName;
	string vocabularyName;
	string wordsOffsetName;

	string cosineName;
	string bm25Name;
	string pageRankName;
	string ndocumentsName;

	string indexBaseName;
	string compressedBasePath;
	string workDirPath;
	string anchorPrefix;

	ui availableMemory;
	bool mustCreateIndex;
	bool mustexecuteQueries;
	bool answerQueryInTerminal;
	ui maxThreads;
	ui socketPort;
	ui maxFiles2Sort;


	void parseArgs(int argc, char** argv){
		int c;
		while((c=getopt(argc,argv, OPTIONS.c_str()))!=-1) {
				switch(c) {
					case 'h':
						cout << HELP << endl;
						exit(0);
						break;
					case 'n':
						indexBaseName = string(optarg);
						break;
					case 'c':
						compressedBasePath = string(optarg) + "/";
						break;
					case 'w':
						workDirPath = string(optarg) + "/";
						break;
					case 'm':
						availableMemory = atoi(optarg)*MBYTE;
						break;
					case 'i':
						mustCreateIndex = true;
						break;
					case 'q':
						mustexecuteQueries = true;
						break;
					case 'a':
						answerQueryInTerminal = true;
						break;
					case 'p':
						socketPort = atoi(optarg);
						break;
					case 't':
						maxThreads = atoi(optarg);
						break;
					case 'f':
						maxFiles2Sort = atoi(optarg);
						break;
					default:
						cerr << "Invalid option " << static_cast<char>(c) << endl;
						exit(-1);
				}
		}

		if(mustCreateIndex && indexBaseName.empty()){
			cerr << "-n Name of the file that indexes the compressed documents is empty.\n";
			exit(-1);
		}
		if(mustCreateIndex && compressedBasePath.empty()){
			cerr << "-c Path of the directory where the compressed documents are located is empty.\n";
			exit(-1);
		}

		if(maxThreads < 1){
			cerr << "You need at least one thread.\n";
			exit(-1);
		}

	}


private:
	const string OPTIONS;
	const string HELP;

	Config() :  runsPosfix(".runs"), docInfoName("docs.bin"), tuplesName("tuple3.bin"),
	vocabularyName("words.txt"), wordsOffsetName("wordsOffeset.bin"), cosineName("cosine.bin"), 
	bm25Name("bm25.bin"), pageRankName("pagerank.bin"), ndocumentsName("ndocs.txt"), workDirPath(""), // tmp
	anchorPrefix("anchor_"), availableMemory(256*MBYTE), 
	mustCreateIndex(false), mustexecuteQueries(false), answerQueryInTerminal(false), maxThreads(4),socketPort(3000),
	maxFiles2Sort(8), OPTIONS("hn:c:w:m:iqap:t:f:"),
	HELP(  "h:[no value] help.\n"
		   "\n"
		   "n:[str] Name of the file that indexes the compressed documents (required)\n"
		   "c:[str] Path of the directory where the compressed documents are located (required)\n"
		   "\n"
		   "w:[str] Working directory where the files will be written/saved (optional, default /tmp/)\n"
		   "m:[int] available memory in Mega Bytes (optional, default 256)\n"
		   "i:[no value] must create index (optional, default false)\n"
		   "q:[no value] must execute queries (optional, default false)\n"
		   "a:[no value] answer queries in terminal? If no is by socket (optional, default false)\n"
		   "p:[int] socket port. (optional, default 3000)\n"
		   "t:[int] maximal number of threads. (optional, default 4)\n"
		   "f:[int] maximal number of files to use in the merge sort. (optional, default 8)\n")
	{}
	static Config* config;
};

#endif /* COMMON_H_ */
