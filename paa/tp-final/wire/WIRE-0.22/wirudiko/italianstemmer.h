#ifndef STEM_IT_H
#define STEM_IT_H

#include <config.h>

class ItalianStemmer {
	private:
		static int removeItalianAccent(char*,int);
	public:
		static int do_stem(char*,int);
};
#endif // STEM_IT_H
