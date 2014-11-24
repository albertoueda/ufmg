#ifndef STEM_EN_H
#define STEM_EN_H

#include <config.h>
#include <stdlib.h>  /* for malloc, free */
#include <string.h>  /* for memcmp, memmove */

struct stemmer {
char * b;       /* buffer for word to be stemmed */
int k;          /* offset to the end of the string */
int j;          /* a general offset into the string */
};

using namespace std;

class EnglishStemmer {

	private:
		struct stemmer* z;
		int cons(int);
		int m();
		int vowelinstem();
		int doublec(int);
		int cvc(int);
		int ends(char*);
		void setto(char*);
		void r(char*);
		void step1ab();
		void step1c();
		void step2();
		void step3();
		void step4();
		void step5();
	public:
		int do_stem(char*,int);
		EnglishStemmer();
		~EnglishStemmer();
};
#endif // STEM_EN_H
