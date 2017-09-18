/*
 * ParserManager.cpp
 *
 *  Created on: Apr 5, 2013
 *      
 */

#include "ParserManager.h"
#include "Timer.h"
#include "ExternalMergeSort.h"
#include "UrlNormalization.h"
#include "PageRank.h"
#include "MyStream.h"
#include <cmath>
#include <algorithm>

ParserManager::ParserManager() : ndocs(0), htmlDocs(0), duplicatedHtmlDocs(0)
{
	config = Config::Instance();
	fdocs.open(config->workDirPath + config->docInfoName, ios_base::out | ios_base::binary);
	if(fdocs.fail()){
		cerr << "Fail to open the file:" << config->workDirPath + config->docInfoName << endl;
		exit(-1);
	}

	fBm25.open(config->workDirPath + config->bm25Name, ios_base::out | ios_base::binary);
	if(fBm25.fail()){
		cerr << "Fail to open the file:" << config->workDirPath + config->bm25Name << endl;
		exit(-1);
	}

	runs = new Runs(config->availableMemory, config->workDirPath, config->runsPosfix + "0", config->maxThreads);
	reader = new RICPNS::CollectionReader(config->compressedBasePath, config->indexBaseName);


}

vector<Tuple3>* loadUniquiTuples(ui offset, MyStream* tuplesStream) {
	vector<Tuple3>* tuples = new vector<Tuple3>();
	Tuple3 tuple;
	bool first = true;
	ui lastDoc;

	//Sets the position of the first tuple
	tuplesStream->seekg(offset);
	(*tuplesStream) >> tuple;
	while (!(tuple == MyStream::END_MARK3)) {
		if(first || tuple.docId != lastDoc){
			first = false;
			lastDoc = tuple.docId;
			tuples->push_back(tuple);
		}
		(*tuplesStream) >> tuple;
	}



	return tuples;
}

void ParserManager::writeCosineDocSizes(vector<float>& docSizes, string fname) {
	ofstream fVectorModel(fname, ios_base::out | ios_base::binary);
	if (fVectorModel.fail()) {
		cerr << "Fail to open the file:"
				<< config->workDirPath + config->cosineName << endl;
		exit(-1);
	}
	size_t floatSize = sizeof(float);
	for (float d : docSizes) {
		d = sqrt(d);
		fVectorModel.write(reinterpret_cast<char*>(&d), floatSize);
	}
	fVectorModel.close();
}

//

void ParserManager::writeAnchorTextData(void){

	Timer timer;
	timer.start();

	cout << "Start writing anchor text data\n";

	vector<DocURLInfo*> docs;
	docs.reserve(urlDocInfoMap.size());

	//ofstream fgraph("/tmp/graph.txt"); 

	for(auto& doc : urlDocInfoMap){
		//fgraph << doc.second->docId << " " << doc.second->numberOfOutLinks << " " << doc.second->listInLinks.size()<< " " << doc.first << endl;
		docs.push_back(doc.second);
	}

	//fgraph.close();
	//return;

	PageRank pageRank(0.7f, 100,1e-5); //best parameters

	pageRank.calcRank(docs);

	partial_sort(docs.begin(), docs.begin() + htmlDocs, docs.end(),
			 [](const DocURLInfo* a, const DocURLInfo* b){
					return a->docId < b->docId;
				}
	);

	vector< vector<pair<ui, ui> > > wordsDocFrequency(anchorTextWordsMap.size(), vector<pair<ui, ui> >());// wordId, docId, pos


	ofstream fbm25a(config->workDirPath + config->anchorPrefix +  config->bm25Name, ios_base::out | ios_base::binary);
	if(fbm25a.fail()){
		cerr << "Fail to open the file:" << config->workDirPath + config->anchorPrefix +  config->bm25Name << endl;
		exit(-1);
	}


	//Writes the vocabulary file containing the 3-tuple: word, wordId, wordGlobalFrenquecy
	ofstream f(config->workDirPath + config->anchorPrefix + config->vocabularyName);
	if(f.fail()){
		cerr << "Fail to open the file:" << config->workDirPath + config->anchorPrefix + config->vocabularyName << endl;
		exit(-1);
	}
	float N = htmlDocs;

	vector<float> idfs;
	idfs.reserve(anchorTextWordsMap.size());
	for(auto& w : anchorTextWordsMap){
		f << w.first << " " << w.second.first << " " << w.second.second << endl;
		idfs.push_back(log2(N/w.second.second));//idf
	}

	f.close();

	size_t uisize = sizeof(ui);

	vector<float> cosineWeights(htmlDocs, 0.0f);

	ofstream fpagerank(config->workDirPath + config->pageRankName, ios::binary|ios::out);
	if(fpagerank.fail()){
		cerr << "Fail to open the file:" << config->workDirPath + config->pageRankName << endl;
		exit(-1);
	}

	size_t floatsize = sizeof(float);

	for(auto doc : docs){
		if(doc->docId < htmlDocs){
			ui bm25size = 0u;
			float& s = cosineWeights[doc->docId];
			float idf = idfs[doc->docId];
			for(auto& w : doc->mapWords){
				float tfidf = idf*(1.0f + log2(w.second));
				s += tfidf*tfidf;
				wordsDocFrequency[w.first].push_back(make_pair(doc->docId, w.second));
				bm25size += w.second;
			}
			fbm25a.write(reinterpret_cast<char*>(&bm25size), uisize);
			fpagerank.write(reinterpret_cast<char*>(&doc->rank), floatsize);

			//cout << doc->docId << " page rank " << doc->rank << endl;
		}
		delete doc;
	}
	fbm25a.close();
	fpagerank.close();
	idfs.clear();

	writeCosineDocSizes(cosineWeights, config->workDirPath + config->anchorPrefix + config->cosineName);
	cosineWeights.clear();



	MyStream* tuplesStream = new MyStream(config->workDirPath + config->tuplesName, false, true, true, config->anchorPrefix);
	Tuple3 tuple;
	tuple.wdocPosition = 0;
	cout << "number of anchor words " <<  wordsDocFrequency.size() << endl;



	for(ui v = 0; v < wordsDocFrequency.size(); ++v){
		//tuple.wordId = v;
		for(auto& d : wordsDocFrequency[v]){
			tuple.docId = d.first;
			tuple.wdocFrequency = d.second;
			(*tuplesStream) << tuple;
		}
		if(v + 1 < wordsDocFrequency.size()){
			tuplesStream->endWordIndice();
		}
	}



	delete tuplesStream;


	cout << "End writing anchor text data. Time: " << timer.stopAndGetSeconds() << endl;
}

ParserManager::~ParserManager() {
	//Writes the vocabulary file containing the 3-tuple: word, wordId, wordGlobalFrenquecy
	ofstream f(config->workDirPath + config->vocabularyName);
	if(f.fail()){
		cerr << "Fail to open the file:" << config->workDirPath + config->vocabularyName << endl;
		exit(-1);
	}

	MyStream* tuplesStream = new MyStream(config->workDirPath + config->tuplesName, true, false, false);
	vector<float> docSizes(htmlDocs, 0.0f);

	vector<ul> wordsOffset;
	ifstream fOffset(config->workDirPath + config->wordsOffsetName, ios::binary|ios::in);
	if(fOffset.fail()){
		cerr << "Fail to open file: " << (config->workDirPath + config->wordsOffsetName) << endl;
		assert(fOffset.fail() == false);
		exit(-1);
	}

	//Reads the words offsets that indicates the position in bytes of the tuples for each word
	ul offset;
	size_t size = sizeof(ul);
	wordsOffset.reserve(globalWordsMap.size());
	fOffset.read(reinterpret_cast<char*>(&offset), size);
	while(fOffset){
		wordsOffset.push_back(offset);
		fOffset.read(reinterpret_cast<char*>(&offset), size);
	}
	fOffset.close();

	float N = static_cast<float>(htmlDocs);
	for(const pair<const string, pair<ui, ui> >& w : globalWordsMap){
		f << w.first << " " << w.second.first << " " << w.second.second << endl;

		float idf = log2(N/w.second.second);
		auto tuples = loadUniquiTuples(wordsOffset[w.second.first], tuplesStream);

		for(auto &t : *tuples){
			float tf = 1.0 +  log2(t.wdocFrequency);
			docSizes[t.docId] += tf * idf;
		}

		delete tuples;
	}

	writeCosineDocSizes(docSizes, config->workDirPath + config->cosineName);

	wordsOffset.clear();
	docSizes.clear();
	delete tuplesStream;
	f.close();
	fdocs.close();
	fBm25.close();
	ofstream fndocs(config->workDirPath + config->ndocumentsName);
	fndocs << htmlDocs << endl;
	fndocs.close();
	cout << "Number of HTML documents in the collection: " << htmlDocs << endl;
	cout << "Number of DUPLICATED HTML documents in the collection: " << duplicatedHtmlDocs << endl;


	writeAnchorTextData();
}


/**
 *  Coordinates the HTML parser, the runs creation and the final merge sort.
 */
void ParserManager::parseAndCreateRuns(void) {
	//cout << "Creating runs...\n";
	Timer timer;
	timer.start();
	ui numberOfRuns =  this->createRuns();
	cout << "Time to parse and create runs : " << timer.stopAndGetSeconds() << endl;
	timer.start();
	cout << "Number of runs created: " << numberOfRuns << endl;
	if (numberOfRuns > 1) {
		cout << "Sorting files...\n";
		ExternalMergeSort esort(numberOfRuns);
		esort.sort();
	}
	cout << "Time to sort: " << timer.stopAndGetSeconds() << endl;
}

DocURLInfo* ParserManager::getUrlDoc(const string& url){
	DocURLInfo* currentDocUrl = NULL;
	auto itUrl = urlDocInfoMap.find(url);
	if(itUrl == urlDocInfoMap.end()){
		currentDocUrl = new DocURLInfo();
		currentDocUrl->url = url;//retirar
		urlDocInfoMap.insert(pair<const string, DocURLInfo*>(url, currentDocUrl));
	}else{
		currentDocUrl = itUrl->second;
	}

	return currentDocUrl;
}


/**
 * Gets the words and positions extracted from the HTML and merges with the global words map.
 * It also saves the informations of the document, like url, title...
 */
void ParserManager::getParserData(ParserDocument& parser) {
	if(!parser.processedCorrect){
		return;
	}

	std::lock_guard<std::mutex> lock(mutexWordsMap);//One thread per time
	auto itUrl = urlDocInfoMap.find(parser.currentDoc.url);
	if(itUrl != urlDocInfoMap.end() && itUrl->second->docId != DocURLInfo::MAX_VALID_DOC_ID){
		++duplicatedHtmlDocs;
		return;//repeated html file in the base
	}


	parser.currentDoc.id = htmlDocs;
	fdocs << parser.currentDoc;//saves the information of the document
	fBm25.write(reinterpret_cast<char*>(&parser.currentDoc.nwords), sizeof(parser.currentDoc.nwords)) ;


	IntermediateTuple4 tuple;
	unordered_map<string, pair<ui,ui> >::iterator it;
	for(pair<const string, vector<ui>> & p : parser.currentWordsMap){
		it = globalWordsMap.find(p.first);
		ui wordId;
		ui wordFrenquency = p.second.size();
		if(it == globalWordsMap.end()){
			wordId = globalWordsMap.size();
			globalWordsMap.insert(pair<const string, pair<ui,ui> >(p.first, pair<ui,ui>(wordId, 1)));
		}else{
			wordId = (*it).second.first;
			(*it).second.second += 1;
		}

		for(ui position : p.second){
			tuple.wordId = wordId;
			tuple.docId = parser.currentDoc.id;
			tuple.wdocFrequency = wordFrenquency;
			tuple.wdocPosition = position;
			runs->insert(tuple);
		}
	}

	//cout << "******* " << parser.currentDoc.url << endl;
	DocURLInfo* currentDocUrl = getUrlDoc(parser.currentDoc.url);
	currentDocUrl->docId = parser.currentDoc.id;

	currentDocUrl->numberOfOutLinks += parser.anchorTextWordMap.size();

	for(auto& urlInfo : parser.anchorTextWordMap){
	//	cout << urlInfo.first << " * ";
		DocURLInfo* edge =  getUrlDoc(urlInfo.first);
		edge->listInLinks.push_back(currentDocUrl);
		for(auto& w : urlInfo.second){
			auto wordId = anchorTextWordsMap.find(w.first);
			if(wordId == anchorTextWordsMap.end()){
				edge->mapWords.insert(pair<const ui,ui>(anchorTextWordsMap.size(), w.second));
				anchorTextWordsMap.insert(pair<string, pair<ui,ui> >(w.first, pair<ui, ui>(anchorTextWordsMap.size(), 1))); // word id, idf
			}else{
				wordId->second.second += 1;//idf
				auto itLw = edge->mapWords.find(wordId->second.first);//document map
				if(itLw == edge->mapWords.end()){
					edge->mapWords.insert(pair<const ui,ui>(wordId->second.first, w.second)); // word id, document frequency
				}else{
					itLw->second += w.second;
				}
			}
			//cout << w.first << " " << w.second << " ";
		}
		//cout << endl;
	}


	++htmlDocs;
}

/**
 * Parses the html files and create the runs files.
 */
ui ParserManager::createRuns(void) {
	bool mustContinue = true;
	vector<std::thread> threads;

	for(ui i = 0; i < config->maxThreads; ++i){
		threads.push_back(std::thread([&,this](){//lambda function
				this->processDocs(mustContinue);
			})
		);
	}

	for(auto& thread : threads){
		thread.join();
	}
	runs->end();

	cout << "Number of documents in the collection: " << ndocs  << endl;
	delete reader;
	ui nfiles = runs->getNumberOfFiles();
	delete runs;
	return nfiles;
}

/**
 * Reads the document and extracts it words and positions.
 * @mustContinue this is used to avoid the threads from reading end of file.
 */
void ParserManager::processDocs(bool& mustContinue) {
	RICPNS::Document doc;
	doc.clear();
	ParserDocument parser;

	while(mustContinue){
			if(!mustContinue){
				break;
			}
			mutexDocsReader.lock();
			if( !mustContinue || !reader->getNextDocument(doc) || ndocs > 2600000){
				mustContinue = false;
				mutexDocsReader.unlock();
				break;
			}
			++ndocs;

			mutexDocsReader.unlock();
			if(ndocs%10000u == 0){
				cerr  << ndocs << endl;
			}
//			if(ndocs < 574122){
//				continue;
//			}
//			if(ndocs == 574124){
//				cout << doc.getURL() << endl << endl;
//
//				cout << doc.getText() << endl;
//
//			}

	//		cerr << ndocs << endl;


		//cout << doc.getURL() << endl;
		parser.parse(doc);
		//cout << "parseou\n";
		cout.flush();
		this->getParserData(parser);
		doc.clear();
	}
}



