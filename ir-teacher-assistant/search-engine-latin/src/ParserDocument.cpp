/*
 * ParserDocument.cpp
 *
 *  Created on: Mar 18, 2013
 *      
 */

#include "ParserDocument.h"
#include "htmlcxx/html/utils.h"

#include <iostream>
#include <map>
#include <algorithm>

ParserDocument::ParserDocument() : skipTag(false), isTitle(false), isLink(false),
tags2skip({"script", "style", "button", "input", "textarea", "applet", "comment", "embed", "select"})
{
	stringUtil = StringUtil::Instance();
}

ParserDocument::~ParserDocument() {

}


bool isPossibleHtmlFile(const string& url) {

	//cout << "isPossibleHtmlFile\n";
	//cout << url << endl;
	if(url.size() < 6){
		return false;
	}
	if(url.find('@') != string::npos){
		return false; //email
	}

	if(url.find("file") == 0){
		return false;
	}

	if(url.find("ftp") == 0){
			return false;
	}
	if(url[url.size() - 1] == '/'){
		return true;
	}

	size_t p = url.rfind('.');
	//cout << "compare\n";
	if(p != string::npos && p < url.length() && url.length() - p < 6){
		string prefix = url.substr(p+1, url.length() - p);
//		printf("%s\n", prefix.c_str());
		transform(prefix.begin(), prefix.end(), prefix.begin(), ::tolower);
		static unordered_set<string> fileTypes = {"jpeg", "jpg", "png", "gif", "doc", "docx", "xls", "xlsx", "ppt", "flv", "swf", "exe", "pdf", "avi", "mp3", "mp4", "zip", "gz", "tar", "bz2", "rar", "wmv", "wma"};

		if(fileTypes.find(prefix) != fileTypes.end()){
		//	cout << "**\n";
			return false;
		}
	}

//	cout << "**e\n";

	return true;
}

/**
 * Extracts the words and positions from the document. It also extracts saves the URL, title
 * and the description of the document. If the description field is not present it uses
 * the first line of text that has more than 80 characters.
 */
void ParserDocument::parse(RICPNS::Document& document) {
	processedCorrect = false;
	skipTag = false;
	isTitle = false;
	isLink = false;
	currentDoc = DocInfo( document.getURL(), 0);
	absoluteUrl = htmlcxx::Uri(document.getURL());
	string empty("");
	//cerr << "converting link \n";
	currentDoc.url = htmlcxx::HTML::convert_link(empty, absoluteUrl);
	transform(currentDoc.url.begin(), currentDoc.url.end(), currentDoc.url.begin(), ::tolower);

	//cerr << "end converting link\n";
	//cout << "^^ " << currentDoc.url << "\t" << serverUrl << "\t" << absoluteUrl << endl;
	currentWordsMap.clear();
	anchorTextWordMap.clear();
	string html;
	//Removes the TCP header
	extractHtml(document, html);
	if(html.empty()){
		//Content-Type is not html
		return;
	}
	stringUtil->convertToIso88591(html, html);

	//cerr << "starting parsing\n";
	htmlcxx::HTML::ParserSax::parse(html);

	//cerr << "end parsing\n";
}


void ParserDocument::foundTag(htmlcxx::HTML::Node& node, bool isEnd) {
	if(isEnd){
		skipTag = false;
		isTitle = false;
		isLink = false;
	}else{
		string tagname = node.tagName();
		transform(tagname.begin(), tagname.end(), tagname.begin(), ::tolower);
		if(tags2skip.find(tagname) != tags2skip.end()){
			skipTag = true;
		}else if("title" == tagname){
			isTitle = true;
		}
		else if( "a" == tagname){
		//	cerr << "found a\n";
		//	cerr << node.text() << endl;
			node.parseAttributes();
		//	cerr << "end parseAttributes\n";
			const map<string, string>& atts = node.attributes();
			auto it = atts.find("href");
			//cout << "found HREF\n";
			if(it != atts.end()){
				if(!it->second.empty() && it->second[0] != '#'){
				//	cerr << "relative " << it->second << endl;
					string relative = it->second;
					if(relative.size() > 2048){
						relative = relative.substr(0, 2047);
					}
					anchorUrl =  htmlcxx::HTML::convert_link(relative, absoluteUrl);

					if(anchorUrl != currentDoc.url && isPossibleHtmlFile(anchorUrl)){
						transform(anchorUrl.begin(), anchorUrl.end(), anchorUrl.begin(), ::tolower);
						isLink = true;
					}
					//cout << currentDoc.url << " | " << it->second << " | " << anchorUrl << endl;
					//cout << "**\n";
					//cout.flush();
					//cout << "is html " << isLink << endl;
				}
			}

			//cerr << "end a\n";
			//cerr.flush();
		}
		else if("meta" == tagname){
			//http://www.w3schools.com/tags/tag_meta.asp
			node.parseAttributes();
			 const map<string, string>& atts = node.attributes();
			 map<string, string>::const_iterator it = atts.find("name");
			 if(it != atts.end()){
				 if(it->second == "author" || it->second == "description" || it->second == "keywords"){
					 bool isDescription = it->second == "description";
					 it = atts.find("content");
					 if(it != atts.end()){
						 if(isDescription){
							 currentDoc.description.assign(it->second.begin(), it->second.end());
							 stringUtil->sanitezeHTML(currentDoc.description);
						 }
						 processWords(const_cast<string&>(it->second));
					 }
				 }
			 }
		}
	}

	//cout << "tag name: " << node.tagName() << skipTag << endl;
//	cout << "foundTag: " << node.text() << endl;
}

/**
 * Extracts the words presents in the text
 */
void ParserDocument::processWords(string& text) {
	//cout << "processWords\n";
	vector < string > words;
	if (isTitle) {
		currentDoc.title.assign(text.begin(), text.end());
		stringUtil->sanitezeHTML(currentDoc.title);
	}else if(currentDoc.description.empty() && text.size() > 80){
		currentDoc.description.assign(text.begin(), text.end());
	}

	htmlcxx::HTML::decode_entities2(text);
	stringUtil->extractWords(text, words);
	unordered_map<string, vector<ui> >::iterator it;
	for (const string& w : words) {
		//cout << "##:" << w << endl;
		it = currentWordsMap.find(w);
		if (it == currentWordsMap.end()) {
			currentWordsMap.insert(
					pair<const string, vector<ui> >(w,
							vector < ui > (1, currentDoc.nwords)));
		} else {
			(*it).second.push_back(currentDoc.nwords);
		}
		++currentDoc.nwords;
		if(isLink){
			//cout << "in link\n";
			auto mapWords = anchorTextWordMap.find(anchorUrl);
			if(mapWords == anchorTextWordMap.end()){
				anchorTextWordMap.insert(
					pair<const string, unordered_map<string, ui>>(anchorUrl,
							{pair<const string, ui>(w, 1)}
					)
				);
			}else{
				auto itW = mapWords->second.find(w);
				if(itW == mapWords->second.end()){
					mapWords->second.insert(pair<const string, ui>(w, 1));
				}else{
					++(itW->second);
				}
			}
			//cout << "end link\n";
			//cout.flush();
		}
	}
	//cout << "** s\n";
	//cout.flush();

}

void ParserDocument::foundText(htmlcxx::HTML::Node& node) {
	if(skipTag){
		return;
	}

	 string& text = const_cast<string&>(node.text());
	processWords(text);
	//cout << "foundText: " << text << endl;
}

void ParserDocument::foundComment(htmlcxx::HTML::Node& node) {

}

void ParserDocument::endParsing() {
	processedCorrect = true;
}
/**
 * Removes the TPC header
 */
void ParserDocument::extractHtml(RICPNS::Document& document, string& html) {
	html = document.getText();
	size_t begin = html.find('<');//if there isn't < the it is a binary document or at least it isn't HTML
	html = begin == html.npos ? "" : html.substr(begin, html.size());
}

