/*
 * Query.cpp
 *
 *  Created on: Mar 31, 2013
 *      
 */

#include "Query.h"
#include "MyStream.h"
#include "DocInfo.h"
#include "socket/ServerSocket.h"
#include "socket/SocketException.h"
#include "BooleanModel.h"
#include "CosineModel.h"
#include "BM25Model.h"
#include "LinearMixture.h"
#include "LinearMixture2.h"
#include "PageRankModel.h"

#include <cassert>
#include <sstream>
#include <algorithm>
#include <functional>


Query::Query(): config(Config::Instance()),
rankingModels({make_pair(new BooleanModel(&globalWordsMap, &cache, &wordsOffset), "Boolean"),
	make_pair(new CosineModel(&globalWordsMap, &cache, &wordsOffset), "Cosine"),
	make_pair(new BM25Model(&globalWordsMap, &cache, &wordsOffset, 0.1, 0.48f), "BM25")
}),
pageSize(10)

{
	CosineModel* cosine = dynamic_cast<CosineModel*>(rankingModels[1].first);
	BM25Model* bm25 = dynamic_cast<BM25Model*>(rankingModels[2].first);
	LinearMixture* linear = new LinearMixture(&globalWordsMap, &cache, &wordsOffset, bm25, cosine);
	CosineModel* cosineanchor = new CosineModel(&anchorWordsMap, &anchorCache, &anchorWordsOffset, config->anchorPrefix);
	BM25Model* bm25anchor = new BM25Model(&anchorWordsMap, &anchorCache, &anchorWordsOffset, 0.2, 0.0f, config->anchorPrefix);
	PageRankModel* pagerank = new PageRankModel(&globalWordsMap, &cache, &wordsOffset);

	rankingModels.push_back(make_pair(linear, "Mix"));
	rankingModels.push_back(make_pair(cosineanchor, "Cosine-Anchor"));
	rankingModels.push_back(make_pair(bm25anchor, "BM25-Anchor"));
	rankingModels.push_back(make_pair(new PageRankModel(&globalWordsMap, &cache, &wordsOffset), "PageRank"));
	rankingModels.push_back(make_pair(new LinearMixture2(&anchorWordsMap, &anchorCache, &anchorWordsOffset, bm25anchor, cosineanchor, pagerank, linear ), "Mix-All"));

	fdocInfo.open(config->workDirPath +  config->docInfoName,  ifstream::in | ifstream::binary);
	if(fdocInfo.fail()){
			cerr << "Fail to open file: " << (config->workDirPath +  config->docInfoName) << endl;
			assert(fdocInfo.fail() == false);
	}

	readVocabulary("", globalWordsMap);
	readVocabulary(config->anchorPrefix, anchorWordsMap);

	readOffset("", wordsOffset, globalWordsMap.size());
	readOffset(config->anchorPrefix, anchorWordsOffset, anchorWordsMap.size());

	if(anchorWordsOffset.size() != anchorWordsMap.size()){
		cerr << "different sizes\n";
		cerr << anchorWordsOffset.size()  << " " << anchorWordsMap.size() << endl;
		cerr << wordsOffset.size()  << " " << globalWordsMap.size() << endl;
		exit(-1);
	}

	answerkey = new AnswerKey(&rankingModels);
}

Query::~Query() {
	if(fdocInfo.is_open()){
		fdocInfo.close();
	}
	delete answerkey;
}


void Query::readVocabulary(string prefix, unordered_map<string, pair<ui,ui> >& vocabulary) {
	ifstream fwords(config->workDirPath + prefix + config->vocabularyName);
	if (fwords.fail()) {
		cerr << "Fail to open file: "
				<< (config->workDirPath + prefix + config->vocabularyName) << endl;
		assert(fwords.fail() == false);
	}
	//Reads the vocabulary
	string w;
	ui id, frequency;
	fwords >> w >> id >> frequency;
	while (fwords) {
		vocabulary.insert(
				pair<const string, pair<ui, ui> >(w,
						pair<ui, ui>(id, frequency)));
		fwords >> w >> id >> frequency;
	}
	fwords.close();
}


void Query::readOffset(string prefix, vector<ul>& voffset, size_t numberOfWords) {
	ifstream fOffset(config->workDirPath + prefix + config->wordsOffsetName, ios::binary | ios::in);
	if (fOffset.fail()) {
		cerr << "Fail to open file: "
				<< (config->workDirPath + prefix + config->wordsOffsetName) << endl;
		assert(fOffset.fail() == false);
		exit(-1);
	}
	//Reads the words offsets that indicates the position in bytes of the tuples for each word
	ul offset;
	size_t size = sizeof(ul);
	voffset.reserve(numberOfWords);
	fOffset.read(reinterpret_cast<char*>(&offset), size);
	while (fOffset) {
		voffset.push_back(offset);
		fOffset.read(reinterpret_cast<char*>(&offset), size);
	}
	fOffset.close();
}

void Query::search(string& text, ServerSocket* socket) {
	ui numberOfDocs = 0;
	timer.start();

	vector<string> parts;
	split(text, ';', parts);
	ui page = atoi(parts[0].c_str());
	string queryString = parts[2];
	vector<string> smodels;
	split(parts[1], ',', smodels);
	auto models = new vector<MODEL_ENUM>();
	for(auto& s : smodels){
		models->push_back( static_cast<MODEL_ENUM>(atoi(s.c_str())));
	}

	unordered_set<ui> answers;
	 //answerkey.getanswers(queryString, answers);
	answerkey->getanswers(queryString, answers);

	vector<vector<pair<float, ui> > *> results;


	for(MODEL_ENUM model : *models){
		vector<pair<float, ui> > * result = rankingModels[static_cast<ui>(model)].first->search(queryString);
		if(model != MODEL_ENUM::BOOLEAN){
			ui k = min((page+1)*pageSize, static_cast<ui>(result->size()));
			partial_sort(result->begin(), result->begin() + k, result->end(), greater<pair<float, ui> >());
		}
		numberOfDocs = max(numberOfDocs, static_cast<ui>(result->size()));
		results.push_back(result);
	}

	if(socket == NULL){
		sumary(results[0], socket);
		return;
	}

	if(!answers.empty()){
		(*socket) << "<ul id='myTab' class='nav nav-tabs'>\n"
				     "<li class='active'><a href='#resultPages' data-toggle='tab'>Results</a></li>\n"
					 "<li><a href='#chartResult' data-toggle='tab'>Charts</a></li>\n"
					  "</ul>\n";

		(*socket) << "<div class='tab-content'>\n"
						 "<div class='tab-pane'  id=chartResult>";
		stringstream inputs;
		answerkey->getChartData(inputs, *models, queryString);
		(*socket) << inputs.str() << "</div>\n";
//		cout << inputs.str() << endl;
		(*socket) <<	"<div  class='tab-pane active' id='resultPages'>\n"
					     "<div id='statsId2' class='stats'></div>\n";
	}



	writeTableHeader(*models, socket);
	writeTableBody(results, answers, socket, page, numberOfDocs);
	writePagination(numberOfDocs, page, queryString, socket);


	stringstream ss;
	ss << "<div id='statsId' class='stats'>";
	ss << "Number of documents retrievied: " << numberOfDocs;
	if(numberOfDocs > pageSize){
		ss << ". Showing only " << pageSize;
	}
	ss << " (" << timer.stopAndGetSeconds() << " seconds)";
	ss << "</div>\n";
	if(!answers.empty()){
		ss << "</div></div>\n";//resul pages div,tab-content
	}
	(*socket) << ss.str();

	delete models;
	for(auto result : results){
		delete result;
	}
}


void Query::sumary(vector<pair<float, ui> >* result, ServerSocket* socket) {
	if(result == NULL || result->size() == 0){
		if(socket){
		  (*socket) << "<p>Your search did not match any documents.</p>\n";
		}else{
			cout << "Your search did not match any documents.\n";
		}

		return;
	}

	ui lastDoc;
	ui numberOfDocs = 0u;
	string html;
	DocInfo doc;


	for(auto& t : *result){
		if(numberOfDocs == 0 || lastDoc != t.second){
			lastDoc = t.second;
			if(numberOfDocs < pageSize){
				DocInfo::readDoc(fdocInfo, t.second, doc);
				if(socket){
					doc.toHtml(html);
					(*socket) << html;
				}
				else{
					cout << doc.id << " " << doc.url << endl;
				}
			}

			++numberOfDocs;
		}
	}


	float sec = ((timer.stop()*100)/1000000)/100.0f;
	stringstream ss;
	if(socket){
		ss << "<div id='statsId' class='stats'>";
	}
	ss << "Number of documents retrievied: " << numberOfDocs;
	if(numberOfDocs > pageSize){
		ss << ". Showing only " << pageSize;
	}
	ss << " (" << sec << " seconds)";
	if(socket){
		ss << "</div>";
	}
	ss << endl;
	if(socket){
		(*socket) << ss.str();
	}else{
		cout << ss.str();
	}

}

void Query::writeTableHeader(vector<MODEL_ENUM>& models, ServerSocket* socket) {
	stringstream ss;
	ss << "<table class='table table-bordered'>\n<thead><tr>\n<th max-width='5%'>#</th>\n";
	for(MODEL_ENUM model : models){
		ss << "<th>" << rankingModels[static_cast<ui>(model)].second << "</th>\n";
	}
	ss << "</tr>\n</thead>\n";
	(*socket) << ss.str();
}

void Query::writeTableBody(vector<vector<pair<float, ui> > *>& results, unordered_set<ui>& answers, ServerSocket* socket, ui page, ui maxDocs) {
	vector<ui> hits(results.size(), 0);
	ui startDoc = page*pageSize;
	ui endDoc = min(maxDocs, (page+1)*pageSize);
	ui nmodels = results.size();
	auto keyEnd = answers.end();
	DocInfo doc;
	stringstream* ss = new stringstream();
	(*ss) << "<tbody>\n";

	for(ui curDoc = 0; curDoc < endDoc; ++curDoc){
		if(curDoc >= startDoc){
			(*ss) << "<tr>\n<td>"<< (curDoc+1) << "</td>\n";
		}
		for(ui model = 0; model < nmodels; ++model){
			if(curDoc < results[model]->size()){
				bool isHit = false;
				if(answers.find(results[model]->at(curDoc).second) != keyEnd){
					++hits[model];
					isHit = true;
				}
				if(curDoc >= startDoc){
					(*ss) << "<td>\n";
					if(isHit){
						(*ss) << "<span class='badge badge-success'>"<< hits[model] <<"</span>";
					}
					DocInfo::readDoc(fdocInfo, results[model]->at(curDoc).second, doc);
					string html;
					doc.toHtml(html);
					(*ss) << html;
					(*ss) << "</td>\n";
				}
			}else if(curDoc >= startDoc){
				(*ss) << "<td>-</td>\n";
			}
		}

		if(curDoc >= startDoc){
			(*ss) << "</tr>\n";
			(*socket) << (*ss).str();
			delete ss;
			ss = new stringstream();
		}
	}

	(*ss) << "</tbody>\n</table>\n";

	(*socket) << (*ss).str();

	delete ss;
}

void Query::writePagination(ui maxDocs,ui page, string& text, ServerSocket* socket) {
	if(maxDocs/pageSize == 0){
		return;
	}
	stringstream ss;
	const ui pagesToShow = 10;
	ui startPage = page > pagesToShow/2 ? page - pagesToShow/2 : 0u;
	ui endPage = min(maxDocs/pageSize, startPage + pagesToShow -1);

	ss << "<div class='pagination'>\n<ul>\n";

	if(page == 0){
		ss << " <li class='disabled'><a href='#'>Prev</a></li>\n";
	}else{
		ss << " <li><a href='#' onclick='searchPages(" << (page-1) << ");'>Prev</a></li>\n";
	}

	for(ui i = startPage; i <= endPage; ++i){
		if(i == page){
			ss << " <li class='active'><a href='#'>"<< (i+1) << "</a></li>\n";
		}else{
			ss << " <li><a href='#'  onclick='searchPages(" << i << ");'>"<< (i+1) << "</a></li>\n";
		}
	}

	if(page == maxDocs/pageSize){
		ss << " <li class='disabled'><a href='#'>Next</a></li>\n";
	}else{
		ss << " <li><a href='#'  onclick='searchPages(" << page+1 << ");'>Next</a></li>\n";
	}

	ss << "</ul>\n</div>\n";

	(*socket) << ss.str();

}


/**
 * Reads the queries from the terminal or socket and answers then to the terminal or socket.
 */
void Query::executeQueries(){
	Config* config = Config::Instance();
	string queryString;
	Query query;
	cout << "Query engine is ready.\n";

	if(config->answerQueryInTerminal){
		while (true) {
			cin >> queryString;
			query.search(queryString, NULL);
		}

	}else{
		try {
			// Create the socket
			ServerSocket server(config->socketPort);
			while (true) {
				ServerSocket new_sock;
				server.accept(new_sock);
				try {

					new_sock >> queryString;
					cout << "QueryString: " << queryString << endl;
					query.search(queryString, &new_sock);
					server.cleanCon(new_sock);
				}
				catch (exception& e) {
					cerr << "Exception was caught:" << e.what() << endl;
				}catch(...){
					cerr << "Unknow Exception\n";
				}
			}

		} catch (SocketException& e) {
			cerr << "Exception was caught:" << e.description() << endl;
		}
	}
}


