/*
 * ParserDocument.h
 *
 *  Created on: Mar 18, 2013
 *      
 */

#ifndef PARSERDOCUMENT_H_
#define PARSERDOCUMENT_H_

#include "ricplib/Document.h"
#include "htmlcxx/html/ParserSax.h"
#include "htmlcxx/html/Node.h"
#include "htmlcxx/html/Uri.h"
#include "common.h"
#include "DocInfo.h"
#include "Tuple.h"
#include "StringUtil.h"

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <ostream>

/**
 * Parses the HTMLs files, handles the encoding and stores the words that
 * occur in document and it's positions. This class extends the ParseSax from htmlcxx library.
 * The Sax was choose instead of the DOM version because it is more efficient. It doesn't
 * constructs the HTML tree. It handles the HTML by demand.
 */
class ParserDocument : protected htmlcxx::HTML::ParserSax {
public:
	ParserDocument();
	virtual ~ParserDocument();

	/**
	 * Extracts the words and positions from the document. It also extracts saves the URL, title
	 * and the description of the document. If the description field is not present it uses
	 * the first line of text that has more than 80 characters.
	 */
	void parse(RICPNS::Document &document);

	unordered_map<string, vector<ui> > currentWordsMap;//word, list of positions
	unordered_map<string, unordered_map<string, ui> > anchorTextWordMap;//url <word, frequency>
	DocInfo currentDoc;//Stores the informations of the document
	bool processedCorrect;//Indicates if the file is HTML

protected:
	virtual void foundTag(htmlcxx::HTML::Node& node, bool isEnd);
	virtual void foundText(htmlcxx::HTML::Node& node);
	virtual void foundComment(htmlcxx::HTML::Node& node);
	virtual void endParsing();

private:
	StringUtil* stringUtil;
	bool skipTag;//Must skip the tag text
	bool isTitle;//The text of this tags stores the title
	bool isLink;
	htmlcxx::Uri absoluteUrl;
	string anchorUrl;
	string anchorText;
	unordered_set<string> tags2skip;//names of the tags to skip text

	/**
	 * Extracts the words presents in the text
	 */
	void processWords(string& text);
	/**
	 * Removes the TPC header
	 */
	void extractHtml(RICPNS::Document &document, string &html);
};

#endif /* PARSERDOCUMENT_H_ */
