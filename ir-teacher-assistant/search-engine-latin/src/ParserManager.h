/*
 * ParserManager.h
 *
 *  Created on: Apr 5, 2013
 *      
 */

#ifndef PARSERMANAGER_H_
#define PARSERMANAGER_H_

#include "common.h"
#include "Tuple.h"
#include "Runs.h"
#include "ParserDocument.h"
#include "ricplib/CollectionReader.h"
#include "DocURLInfo.h"

#include <unordered_map>
#include <string>
#include <fstream>
#include <mutex>

/**
 * Coordinates the HTML parser, the runs creation and the final merge sort.
 */
class ParserManager{
public:
	ParserManager();
	~ParserManager();

	/**
	 * Coordinates the HTML parser, the runs creation and the final merge sort.
	 */
	void parseAndCreateRuns(void);


private:
	unordered_map<string, pair<ui,ui> > globalWordsMap; //word, <id, frequency in the collection>
	unordered_map<string, pair<ui,ui> > anchorTextWordsMap; //word, <id, frequency in the collection>
	unordered_map<string, DocURLInfo*> urlDocInfoMap;//url, urlDocInfo

	Runs* runs;//Used to write the tuples
	ui ndocs;//Number of documents processed
	ui htmlDocs;//Number of HTML documents processed
	ui duplicatedHtmlDocs;
	ofstream fdocs;//Stores the informations of each document processed, like url, title...
	ofstream fBm25;//stores the number of words of the documents
	std::mutex mutexWordsMap;//Used to avoid running conditions during the tuples creation
	std::mutex mutexDocsReader;//Used to avoid running conditions during the reading of the documents

	Config* config;//Stores the configuration of the application
	RICPNS::CollectionReader* reader;///Reads the compressed documents

	/**
	 * Gets the words and positions extracted from the HTML and merges with the global words map.
	 * It also saves the informations of the document, like url, title...
	 */
	void getParserData(ParserDocument& parser);
	/**
	 * Parses the html files and create the runs files.
	 */
	ui createRuns(void);
	/**
	 * Reads the document and extracts it words and positions.
	 * @mustContinue this is used to avoid the threads from reading end of file.
	 *
	 */
	void processDocs(bool& mustContinue);

	DocURLInfo* getUrlDoc(const string& url);
	void writeCosineDocSizes(vector<float>& docSizes, string fname);
	void writeAnchorTextData(void);
};

#endif /* PARSERMANAGER_H_ */
