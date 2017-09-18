/*
 * Query.h
 *
 *  Created on: Mar 31, 2013
 *      
 */

#ifndef QUERY_H_
#define QUERY_H_

#include "MyStream.h"
#include "common.h"
#include "StringUtil.h"
#include "Timer.h"
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include "socket/ServerSocket.h"
#include "Tuple.h"
#include "BasicModel.h"
#include  "AnswerKey.h"
/**
 * This class processes boolean queries and returns the results via terminal or socket (used by PHP).
 */


class Query {
public:
	Query();
	~Query();
	void search(string& text, ServerSocket* socket);

	/**
	 * Reads the queries from the terminal or socket and answers then to the terminal or socket.
	 */
	static void executeQueries();

private:
	ifstream fdocInfo;//Files that stores the informations of each document, like title, url
	unordered_map<string, pair<ui,ui> > globalWordsMap; //word, <id, frequency in the collection>
	unordered_map<ui, vector<Tuple3> > cache; //wordId, Tuples; If a word is searched, its tuples are stored in cache
	vector<ul> wordsOffset;//offsets that indicates the position in bytes of the tuples for each word

	unordered_map<string, pair<ui,ui> > anchorWordsMap; //word, <id, frequency in the collection>
	unordered_map<ui, vector<Tuple3> > anchorCache; //wordId, Tuples; If a word is searched, its tuples are stored in cache
	vector<ul> anchorWordsOffset;//offsets that indicates the position in bytes of the tuples for each word

	Timer timer;//Used to measure the execution time
	Config* config;//stores the paremeters of the application
	vector<pair<BasicModel*, string> > rankingModels;
	const ui pageSize;
	AnswerKey* answerkey;

	void sumary(vector<pair<float, ui> >* result, ServerSocket* socket);

	void writeTableHeader(vector<MODEL_ENUM>& models, ServerSocket* socket);
	void writeTableBody(vector<vector<pair<float, ui> >*>& results, unordered_set<ui>& answers, ServerSocket* socket, ui page, ui maxDocs);
	void writePagination(ui maxDocs, ui page, string& text, ServerSocket* socket);

	void readVocabulary(string prefix, unordered_map<string, pair<ui,ui> >& vocabulary);
	void readOffset(string prefix, vector<ul>& voffset, size_t numberOfWords);
};

#endif /* QUERY_H_ */
