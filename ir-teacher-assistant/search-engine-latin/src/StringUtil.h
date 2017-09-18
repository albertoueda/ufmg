/*
 * StringUtil.h
 *
 *  Created on: Mar 18, 2013
 *      
 */

#ifndef STRINGUTIL_H_
#define STRINGUTIL_H_

#include <locale>
#include <string>
#include <vector>
#include <istream>
#include <fstream>
#include <algorithm>

#include "htmlcxx/html/CharsetConverter.h"

using namespace std;

/**
 * Functor that compares two strings ignoring the case
 */
struct EqualNocase : public std::binary_function<char, char, bool> {
	bool operator()(char x, char y) const {
		return tolower(static_cast<unsigned char>(x)) == tolower(static_cast<unsigned char>(y));
	}
};

void split(const string& input, char separator, vector<string>& tokens);

void replaceAll(string& str, const string& from, const string& to);
/**
 * Deals with the text from the HTML file
 */
class StringUtil {
public:

	/**
	 * Singleton
	 */
	static StringUtil* Instance(){
		if(!stringUtil){
			stringUtil = new StringUtil;
		}
		return stringUtil;
	}

	/**
	 * Extracts the words of the text and save them in the vector ret
	 */
	void extractWords(string& text, vector<string>& ret);

	/**
	 * Removes the html tags from s
	 */
	string& sanitezeHTML(string& s);

	~StringUtil(){
		if(stringUtil){
			delete stringUtil;
			stringUtil = 0;
		}
	}
	/**
	 * Reads all the content of the file located in path to the string str
	 */
	void readText(string& path, string &str){
		ifstream inputfile(path);
		str = string((istreambuf_iterator<char>(inputfile)),istreambuf_iterator<char>());
		inputfile.close();
	}

	/**
	 * Compares if the string a is equal b ignoring the case
	 */
	bool isEqual(const string& a, const string& b )const{
		return equal(a.begin(), a.end(), b.begin(), EqualNocase());
	}

	/**
	 * Converts the text encoding to ISO-8859-1 if the original encoding is UTF-8.
	 */
	void convertToIso88591(string& text, string &out);

	/**
	 * Substitutes accents or symbols by their most similar ascii character
	 */
	void normalize(char& c);

private:
	StringUtil();
	/**
	 * Detects end or begin of a word.
	 */
	bool isSeparator(char c){
		return !isalnum(c);
	}


	/**
	 * Finds the beginning of the next word in str
	 */
	char* nextWord(char* str){
		char *c = str;
		while(*c){
			//normalize(*c);
			if(isalnum(*c)){
				break;
			}
			++c;
		}
		return c;
	}

	/**
	 * Finds the end of the actual word
	 */
	char* nextSepartor(char* str){
		char *c = str;
		while(*c  && !isSeparator(*c)){/*normalize(*c); */++c;}
		return c;
	}

	static StringUtil* stringUtil;
	locale loc;
    htmlcxx::CharsetConverter UTF8_TO_ISO88591;
};

#endif /* STRINGUTIL_H_ */
