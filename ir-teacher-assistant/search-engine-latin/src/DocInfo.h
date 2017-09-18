/*
 * DocInfo.h
 *
 *  Created on: Mar 18, 2013
 *      
 */

#ifndef DOCINFO_H_
#define DOCINFO_H_

#include "common.h"
#include <string>
#include <cstring>
#include <ostream>
#include <istream>


/**
 * Stores the information of the HTML pages.
 */
class DocInfo {
private:
	static const ui TITLE_LENGTH = 128u;//Max length in bytes of the title
	static const ui URL_LENGTH = 1024u;//Max length in bytes of the url
	static const ui DESCRIPTION_LENGTH = 128u;//Max length in bytes of the description of the html page
	static const ui ID_LENGTH = sizeof(ui);//Length in bytes of the field id
	static const ui NWORDS_LENGTH = sizeof(ui);//Length in bytes of field

public:
	DocInfo():  id(0), nwords(0){}
	DocInfo(string purl, ui pid): url(purl), id(pid), nwords(0){}

	//Total length in bytes of each document info. This is used to fast retrieval
	static const ui TOTAL_LENGTH = TITLE_LENGTH + URL_LENGTH + DESCRIPTION_LENGTH;
	string title; //Title of the page
	string url;//URL of the page
	string description;//Description of the page
	ui id;//Id Of the page
	ui nwords;//Number of words in the page

	/**
	 * Converts the document info in HTML.
	 * @html stores the html representation.
	 */
	void toHtml(string& html);

	/**
	 * Writes the docInfo to out.
	 */
	friend ostream& operator<< (ostream &out, DocInfo &docInfo);
	/**
	 * Reads from in the data of docInfo.
	 */
	friend istream& operator>> (istream &in, DocInfo &docInfo);

	/**
	 * Retrieves the docInfo from in. First it sets the position
	 * where the docInf is located.
	 */
	static void readDoc(istream &in, ui id, DocInfo &docInfo);


};

#endif /* DOCINFO_H_ */
