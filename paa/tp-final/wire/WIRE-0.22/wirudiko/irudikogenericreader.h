// IrudikoGenericReader.h (class header for IrudikoGenericReader)

#ifndef IRUDIKOGENERICREADER_H
#define IRUDIKOGENERICREADER_H

#include <config.h>

#ifndef IRUDIKO_NEEDED_CLASSES
#define IRUDIKO_NEEDED_CLASSES
#include <string.h>
#include <vector>
#include <iostream>
#include <list>
#endif

#include <fstream>
#include <cctype>
#include <algorithm>
#include <time.h>
#include <assert.h>

#include "irudikosketchgenerator.h"

using namespace std;

/* 
 * Generic reader class of Irudiko
 */
string replaceAll(string, string, string);

typedef enum IrudikoFileType { HTML = 0, XML = 1, TEXT = 2 } IrudikoFileType;

class IrudikoGenericReader
{
	public:
		/* VARIABLES */
		/* CONSTRUCTORS/DESTRUCTORS/FUNCTIONS */
		// class constructor
		// class destructor
		virtual ~IrudikoGenericReader();
		// main procedures		
		int process(const char * filename, vector<string>& response);
		int process_and_sketch(char * buf, unsigned int bufsize, unsigned long* sketch, int sketch_dim);
		int process_and_sketch(char * buf, unsigned int bufsize, unsigned long * sketch, int sketch_dim, int _sh_selmode, int _sh_param);
		int process_and_sketch(char * buf, unsigned int bufsize, unsigned long * sketch, int sketch_dim, int _sh_selmode, int _sh_param, char* exclfile);
		// setType
		void setType(IrudikoFileType);
	protected:	
		IrudikoGenericReader();
		IrudikoGenericReader(bool);
		IrudikoGenericReader(bool,IrudikoFileType);
		IrudikoGenericReader(IrudikoFileType);
		bool executeStem;
		IrudikoFileType fileType;
		
		IrudikoSketchGenerator sketch_gen;
		vector<string> wordlist;
		/* FUNCTIONS */
		virtual const char* getStopwordFileName() { return NULL; };
		int getStopwordList(list<string>&);
        // main preprocessing procedure
		int preprocess();
		int preprocess_alt(char* content, unsigned int in_length);
		// stemmer
		virtual int stem() { return -1; };
		// stopwords remover
		int removeStopwords();
	private:
		ifstream infile;
		// No description
		int toggleTags(char*,int);
                int toggleZeroChars (char*,int);
		int toggleNonAlphaChars (char*,int);
		int splitString (char*,int,vector<string>&);
};

#endif // IRUDIKOGENERICREADER_H
