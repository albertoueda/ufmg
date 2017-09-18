/*
 * StringUtil.cpp
 *
 *  Created on: Mar 18, 2013
 *      
 */

#include "StringUtil.h"
#include "htmlcxx/html/utils.h"
#include <iostream>
#include <regex>

StringUtil* StringUtil::stringUtil = 0;

StringUtil::StringUtil():  UTF8_TO_ISO88591("UTF-8", "ISO_8859-1//IGNORE//TRANSLIT") {

}

/**
 * Extracts the words of the text and save them in the vector ret
 */
void StringUtil::extractWords(string& text, vector<string>& ret){
	for(char& c : text){
		normalize(c);//remove accents and symbols
	}
	char* begin = nextWord(const_cast<char *>(text.c_str()));
	char *end = 0;
	while(*begin){
		end = nextSepartor(begin);
		char *last = end;
		--last;
		if(isdigit(*begin) && isdigit(*last)){
			ret.push_back(to_string(last-begin+1));
		}else if(end - begin > 1){ // has more than on letter
			if(end - begin > 4){
				if(*last != 's'){//remove s from end of the words greater then four
					++last;
				}
			}else{
				++last;
			}

			ret.push_back(string(begin, last));
		}
		begin = nextWord(end);
	}
}

/**
 * Converts the text encoding to ISO-8859-1 if the original encoding is UTF-8.
 */
void StringUtil::convertToIso88591(string& text, string &out){
	if(htmlcxx::HTML::detect_utf8(text.c_str(), text.size())){
		out = UTF8_TO_ISO88591.convert(text);

		if(out.empty()){ // error during conversion
			out.assign(text.begin(), text.end());
		}
	}else{
		out.assign(text.begin(), text.end());
	}
}

/**
 * Removes the html tags from s
 */
string& StringUtil::sanitezeHTML(string& s){
	bool isCode = false;
	for(size_t i = 0; i < s.size(); ++i){
		if(s[i] == '<'){
			isCode = true;
		}
		if(isCode){
			s[i] = ' ';
		}
		if(s[i] == '>'){
			isCode = false;
		}
	}

	return s;
}

/**
 * Substitutes accents or symbols by their most similar ascii character
 */
void StringUtil::normalize(char& c){

	if (! //Not convertible char or number
			((c >= (char) 0x30 && c <= (char) 0x39) || //0-9
			 (c >= (char) 0x41 && c <= (char) 0x5A) || // A-Z
			 (c >= (char) 0x61 && c <= (char) 0x7A) || // a-z
			 (c >= (char) 0xC0 && c <= (char) 0xD6) || //À-Ö
			 (c >= (char) 0xD8 && c <= (char) 0xF6) || //Ø-ö
			 (c >= (char) 0xF8 && c <= (char) 0xFF) //ø-ÿ
			)) {//Convert to space .....
		c = ' ';
	}
	else {// Make conversion if applicable!
		if (c >= (char) 0xC0 && c <= (char) 0xC6) c = 'a'; // À-Æ
		if (c == (char) 0xC7) c = 'c'; //Ç
		if (c >= (char) 0xC8 && c <= (char) 0xCB) c = 'e'; //È-Ë
		else if (c >= (char) 0xCC && c <= (char) 0xCF) c = 'i'; //Ì-Ï
		else if (c == (char) 0xD0) c = 'd'; //Ð
		else if (c == (char) 0xD1) c = 'n'; //Ñ
		else if (c >= (char) 0xD2 && c <= (char) 0xD6) c = 'o'; //Ó-Ö
		else if (c == (char) 0xD8) c = 'o'; //Ø
		else if (c >= (char) 0xD9 && c <= (char) 0xDC) c = 'u'; //Ù-Ü
		else if (c == (char) 0xDD) c = 'y'; //Ý
		else if (c == (char) 0xDE) c = 'p'; //Þ
		else if (c == (char) 0xDF) c = 'b'; //ß
		// Same chars just lower case ....
		else if (c >= (char) 0xE0 && c <= (char) 0xE6) c = 'a'; // À-Æ
		else if (c == (char) 0xE7) c = 'c'; //Ç
		else if (c >= (char) 0xE8 && c <= (char) 0xEB) c = 'e'; //È-Ë
		else if (c >= (char) 0xEC && c <= (char) 0xEF) c = 'i'; //Ì-Ï
		else if (c == (char) 0xF0) c = 'd'; //Ð
		else if (c == (char) 0xF1) c = 'n'; //Ñ
		else if (c >= (char) 0xF2 && c <= (char) 0xF6) c = 'o'; //Ó-Ö
		else if (c == (char) 0xF8) c = 'o'; //Ø
		else if (c >= (char) 0xF9 && c <= (char) 0xFC) c = 'u'; //Ù-Ü
		else if (c == (char) 0xFD) c = 'y'; //Ý
		else if (c == (char) 0xFE) c = 'p'; //Þ
		else if (c == (char) 0xFF) c = 'y'; //ÿ
	}
	c = (char)tolower(c);
}

void split(const string& input, char separator, vector<string>& tokens){
	char* c = const_cast<char*>(input.c_str());
	char* end = c;
	while(*end){
		while(*end && *end != separator){
			++end;
		}
		if(end != c){
			tokens.push_back(string(c, end));
			c = end;
		}
		if(*end){
			++end;
			c = end;
		}
	}
}

void replaceAll(string& str, const string& from, const string& to) {
    if(from.empty())
        return;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}
