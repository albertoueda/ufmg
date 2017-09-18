/*
 * CollectionAppender class implementation
 * @author: ueda@dcc.ufmg.br
 */

#include <zlib.h>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstring>
#include <chrono>

// TODO
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "CollectionAppender.h"
#include "envDefault.h"


using namespace std;
using namespace RICPNS;
using namespace chrono;

high_resolution_clock::time_point start;

CollectionAppender::CollectionAppender(const std::string & inputDirectory,
								   const std::string & inputIndexFileName,
								   const std::string & outputDirectory,
								   const std::string & outputIndexFileName,
								   const std::string & outputPrefixFileName)
	: inputDirectory_(inputDirectory),
		inputIndexFileName_(inputDirectory + '/' + inputIndexFileName), // must be at input directory
		outputDirectory_(outputDirectory),
		outputIndexFileName_(outputDirectory + '/' + outputIndexFileName),
		outputPrefixFileName_(outputPrefixFileName),
		inputIndexFilePtr_(NULL),
		inputFile(NULL),
		outputIndexFilePtr_(NULL),
		outputContentFilePtr_(NULL),
		outputContentIndex_(0),
		inputContentFileName_(""),
		outputCurrentOffset_(0),
		filePos_(0)
{

	initialize();
}

CollectionAppender::~CollectionAppender() {
	if(inputIndexFilePtr_ != NULL) {
		fclose(inputIndexFilePtr_);
	}
	if(inputFile != NULL) {
		fclose(inputFile);
	}
	if(outputIndexFilePtr_ != NULL) {
		fclose(outputIndexFilePtr_);
	}
	if(outputContentFilePtr_ != NULL) {
		fclose(outputContentFilePtr_);
	}
}

void CollectionAppender::initialize()
{
	readInputIndex();

	outputIndexFilePtr_ = fopen( outputIndexFileName_.c_str(), "w");
	assert(outputIndexFilePtr_ != NULL);

	stringstream outputPrefix;
	outputPrefix << outputDirectory_ << '/' << outputPrefixFileName_ << outputContentIndex_;
//	cout << outputPrefix << .str().c_str()
	outputContentFilePtr_ = fopen( outputPrefix.str().c_str(), "w");
	assert(outputContentFilePtr_ != NULL);

	if (DEBUG) {
		clog << "Appender Input Index Filename [" << inputIndexFileName_ << "]" << endl;
		clog << "Appender Output Index Filename [" << outputIndexFileName_ << "]" << endl;
		clog << "Appender Output Content Filename [" << outputPrefix.str() << "]" << endl;
		clog << "Appender Initialization OK!" << endl;
	}

	start = high_resolution_clock::now();
}

void CollectionAppender::readInputIndex() 
{
  	ifstream myFile(inputIndexFileName_);
	if (myFile.is_open()) {
		string line;
		string url;

		while (getline(myFile, line)) {
			std::stringstream  linestream(line);			
			linestream >> url;	
			urls[url] = true;
		}
		
		clog << "Read previous " << urls.size() << " URLs" << endl;
		myFile.close();
	} 
}

void CollectionAppender::append(unsigned first, unsigned last)
{
	TUCharPtr docCompressed;
	size_t compressedFileSize = 0;
	Document doc;
	ContentFile content;
	bool file_found = true;

// "../input/content-101-urls.html"
	for (int i = first; i <= last; i++) 
	{
		stringstream input_filename;
		input_filename << inputDirectory_ << '/' << "students_pages_" << i << ".txt";

		file_found = getNextContentFile(content, input_filename.str());
		if (!file_found) {
			if (DEBUG) { clog << "Input file [" << input_filename.str() << "] not found. Skipping..." << endl; }
			continue;
		}

		while(getNextDocument(content, doc))
		{
			if(valid(doc)) {
				compressDocument(doc, docCompressed, compressedFileSize);
				// cout << "url:[" << doc.getURL() << "], contentSize:" << doc.getLength() << endl;
				// cout << "text:[" << doc.getText() << "]" << endl;
				appendDocument(doc, docCompressed, compressedFileSize);
				doc.clear();
				free(docCompressed);
			}
		}
	}

}

void remove_spaces(string& s) 
{
 	s.erase(remove_if(s.begin(), s.end(), ::isspace), s.end());
}


bool CollectionAppender::valid(const Document & doc)
{
	// URL checks
	string url = doc.getURL();
	remove_spaces(url);
	
	if(url.empty() || url.length() > 200) 
		return false;

	// http or www
	string urlPrefix = url.substr(0, 3);
	if (urlPrefix != "www" && urlPrefix != "htt") {
		// clog << "URL " << url << " not valid by prefix." << endl;
		return false;
	}

	// remove final '/'
	if (url.back() == '/'){
		url.pop_back();
	}

	// uniqueness check
	if (urls[url] == true)  {
		// clog << "Skipping " << url << "..." << endl;
		return false;
	}

	urls[url] = true;
	doc.setURL(url);

	return true;
}

bool CollectionAppender::getNextContentFile(ContentFile& contentFile, const std::string& filename) 
{
	FILE *inputFile = fopen(string(filename).c_str(), "r"); // r ?
	if (inputFile == NULL)
		return false;

	if(DEBUG) { clog << "Appender: Input file [" << filename << "] openned." << endl; }

	// obtain file size
	fseek (inputFile , 0 , SEEK_END);
	long size = ftell (inputFile);
	rewind (inputFile);
	filePos_ = 0; // TODO all file?

	unsigned char* text = NULL;
	text = new char[size + 1]; //+1
	int nchars = fread(text, sizeof(char), size, inputFile);
	text[size] = '\n';// '\0'; TODO

	contentFile.setText(string(text));
	contentFile.setLength(size);

	delete[] text;

	return true;
}

// TODO extra |||'s
bool CollectionAppender::getNextDocument(ContentFile& content, Document& doc)
{
	string urlBeginDelim ("|||");
	string urlEndDelim ("|");
	string text = content.getText();

	size_t urlBegin = text.find(urlBeginDelim, filePos_); 
	if (urlBegin == string::npos)
		return false;

	size_t urlEnd = text.find(urlEndDelim, urlBegin + urlBeginDelim.size()); //, 30
	if (urlEnd == string::npos)
		return false;

	size_t urlLength = urlEnd - urlBegin + 1 - urlBeginDelim.size() - urlEndDelim.size();
	string url = text.substr(urlBegin + urlBeginDelim.size(), urlLength);
	doc.setURL(url);

	size_t htmlEnd;
	size_t newUrlBegin = text.find(urlBeginDelim, urlEnd + urlEndDelim.size());

	// the html was the last of the content file
	if (newUrlBegin == string::npos) {
		htmlEnd = content.getLength();
		filePos_ = string::npos;
	}
	else {
		htmlEnd = newUrlBegin - 1;
		filePos_ = newUrlBegin;
		// cout << "filePos_:" << filePos_ << endl;
	}

	size_t htmlBegin = urlEnd + urlEndDelim.size();
	size_t htmlLength = htmlEnd - htmlBegin + 1;
	doc.setText(text.substr(htmlBegin, htmlLength));
	doc.setLength(htmlLength);

	return true;
}

int CollectionAppender::compressDocument(const Document & document,
										 TUCharPtr & to,
									     size_t & size) {

	size_t inDocLength = 0, outDocLength = 0;
	unsigned char * in = NULL;
	unsigned char * out = NULL;

	inDocLength = document.getText().length();
	in = new unsigned char[inDocLength];
	assert(in != NULL);

//	clog << "[" << document.getURL() << "][" << document.getLength() << "]" << endl;
//	clog << "[" << strlen(document.getText().c_str()) << "][" << document.getText().length() << "]" << endl;

	unsigned char * retIn = (unsigned char*)memcpy(in, document.getText().c_str(), inDocLength);
	assert(retIn == in);
	outDocLength = (inDocLength * 1.001) + 12; // According to zlib documentation

	// ALBERTO attention
	out = new unsigned char[outDocLength];
	int ret = compress2(out, &outDocLength, in, inDocLength, COMPRESSION_LEVEL);

	if(DEBUG) {
		switch(ret) {
			case Z_OK:
				//clog << "Z_OK" << endl;
				break;
			case Z_MEM_ERROR:
				clog << "Z_MEM_ERROR" << endl;
				break;
			case Z_BUF_ERROR:
				clog << "Z_BUF_ERROR" << endl;
				break;
			case Z_DATA_ERROR:
				clog << "Z_DATA_ERROR" << endl;
				break;
			default:
				clog << "Hein?" << endl;
				break;
		}
	}

/*
	unsigned char * testout = new unsigned char[inDocLength+1];
	uLong testoutLength = (uLong) inDocLength;
	uncompress(testout, &testoutLength, out, outDocLength);
	testout[testoutLength] = '\0';
	clog << "[" << testout << "]" << endl;
	delete[] testout;

*/
	to = out;
	size = outDocLength;

	delete[] in;

	return EXIT_SUCCESS;
}


void CollectionAppender::appendDocument(const Document & doc,
										TUCharPtr dPtr,
										const size_t & compressedDocSize)
{
//	int size = doc.getLength();
	size_t newOutputOffset = outputCurrentOffset_ + compressedDocSize;

	// If output content file reaches MAX_FILE_SIZE
	if(newOutputOffset >= MAX_FILE_SIZE)
	{
		fclose(outputContentFilePtr_);
		outputContentFilePtr_ = NULL;
		outputContentIndex_++;

		stringstream newFilename;
		newFilename << outputDirectory_ << '/' << outputPrefixFileName_ << outputContentIndex_;
		outputContentFilePtr_ = fopen(newFilename.str().c_str(), "w");
		assert(outputContentFilePtr_ != NULL);

		outputCurrentOffset_ = 0;
		newOutputOffset = compressedDocSize;

		high_resolution_clock::time_point now = high_resolution_clock::now();
		unsigned duration = duration_cast<seconds>( now - start ).count();

		if(DEBUG) { clog << "\tOutput file[" << newFilename.str() << "] openned (total of " 
			<< urls.size() << " urls in " << duration << " sec)" << endl; }
	}

	// Write a new line in file index
	stringstream contentFilename;
	contentFilename << outputPrefixFileName_ << outputContentIndex_;
	fprintf(outputIndexFilePtr_, "%s %s %lu %lu %lu\n",
			doc.getURL().c_str(),
			contentFilename.str().c_str(),
			outputCurrentOffset_,
			newOutputOffset, //-1 , size
			doc.getText().length()
			);

	outputCurrentOffset_ = newOutputOffset;

	// cout << doc.getText() << endl;
	// Write the content of the document
	size_t nchars = fwrite(dPtr, sizeof(unsigned char), compressedDocSize, outputContentFilePtr_);
	assert(nchars == compressedDocSize);
}

