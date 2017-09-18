/*
 * DocInfo.cpp
 *
 *  Created on: Mar 29, 2013
 *      
 */

#include "DocInfo.h"
#include <iostream>
#include <sstream>
#include <cstdio>


/**
 * Converts the document info in HTML.
 * @html stores the html representation.
 */

string crop(string& s){
	return s.length() > 50 ? s.substr(0, 50) + "..." : s;
}

void DocInfo::toHtml(string& html) {
	stringstream ss;
	ss << "<!-- docId=" << this->id << "-->\n";
		ss << "\t <a href='" <<  this->url << "'>" << (this->title.empty()? this->url : this->title) << "</a></br>\n";

		ss << "\t <cite>" << crop(this->url) << "</cite></br>\n";
		ss << "\t<div class='desc'>" << this->description  << "</div>\n";
	ss << "\n";
	html = ss.str();
}


/**
 * Retrieves the docInfo from in. First it sets the position
 * where the docInf is located.
 */
void DocInfo::readDoc(istream &in, ui id, DocInfo &docInfo){
	long bytePosition = id*TOTAL_LENGTH;//Position where the doc is located
	long current = in.tellg();
	if(current == -1l){
		in.clear();//End of file
		current = in.tellg();
	}

	in.seekg(bytePosition - current, ios::cur);//Sets the position where the doc is located
	in >> docInfo;
	docInfo.id =  id;
}

/**
 * This function is used to limit the title and descriptions fields. The
 * objective is to split these fields in the last word that fits in max
 * char and put three points in the end.
 *
 * @buffer where the data will be stored
 * @s the string containing the original data
 * @maxChar the limit of the new string
 */
void writeString(char *buffer, string& s, ui maxChar){
	strncpy(buffer, s.c_str(), maxChar);
	char *c = buffer + maxChar - 1;
	if(*c){
		*c = '\0';
		--c;
		*c = ' ';
		--c;
		*c = ' ';
		--c;
		*c= ' ';
		--c;
		for(ui i =0; i < maxChar -4 && isalnum(*c); ++i ){*c = ' '; --c;}
		*c= '.';
		++c;
		*c= '.';
		++c;
		*c= '.';
	}

}


/**
 * Writes the docInfo to out. All the data are written in binary.
 */
ostream& operator<< (ostream &out, DocInfo &docInfo){
	static char BUFFEROUT[DocInfo::TOTAL_LENGTH];
	char* c = BUFFEROUT;
	writeString(c, docInfo.title,  DocInfo::TITLE_LENGTH);
	c += DocInfo::TITLE_LENGTH;
	strncpy(c, docInfo.url.c_str(), DocInfo::URL_LENGTH);
	c += DocInfo::URL_LENGTH;
	writeString(c, docInfo.description,  DocInfo::DESCRIPTION_LENGTH);
	c += DocInfo::DESCRIPTION_LENGTH;
	out.write(BUFFEROUT, DocInfo::TOTAL_LENGTH);
	return out;
}

/**
 * Reads from in the data of docInfo.
 */
istream& operator>> (istream &in, DocInfo &docInfo){
	static char BUFFERIN[DocInfo::TOTAL_LENGTH];

	in.read(BUFFERIN, DocInfo::TOTAL_LENGTH);
	char *c = BUFFERIN;
	docInfo.title.assign(c);
	c += DocInfo::TITLE_LENGTH;
	docInfo.url.assign(c);
	c += DocInfo::URL_LENGTH;
	docInfo.description.assign(c);

	return in;
}


