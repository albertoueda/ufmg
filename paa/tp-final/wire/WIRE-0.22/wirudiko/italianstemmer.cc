/*  Italian stemmer tring to remove inflectional suffixes */
#include "italianstemmer.h"

int ItalianStemmer::do_stem (char * word, int len) {
	
	if (len > 4) {
		removeItalianAccent(word,len);
		if (word[len]=='e') {  /*  ending with -ie or -he  */
			if (word[len-1]=='i' || word[len-1]=='h') {
				word[len-1]='\0';
         		return (len-2);
        	}
      		word[len]='\0';  /*  ending with -e  */
      		return(len-1);
		}
   		if (word[len]=='i') {  /*  ending with -hi or -ii */
      		if ((word[len-1]=='h') || (word[len-1]=='i')) {
				word[len-1]='\0';
         		return (len-2);
        	}
      		word[len]='\0';  /*  ending with -i  */
      		return(len-1);
    	}
   		if (word[len]=='a') {  /*  ending with -ia  */
      		if (word[len-1]=='i') {
				word[len-1]='\0';
         		return (len-2);
        	}
      		word[len]='\0';  /*  ending with -a  */
      		return(len-1);
    	}
   		if (word[len]=='o') {  /*  ending with -io  */
      		if (word[len-1]=='i') {
         		word[len-1]='\0';
         		return (len-2);
        	}
      		word[len]='\0';  /*  ending with -o  */
      		return(len-1);
    	}
	} /* end if (len > 4) */ 
	return len;
}


int ItalianStemmer::removeItalianAccent(char * word, int len) {
	int i;
	
   	for(i=len; i>=0; i--) {
		if ((word[i]=='à') || (word[i]=='á') || (word[i]=='â') || (word[i]=='ä')) {
         	word[i] = 'a';
        }
      	if ((word[i]=='ò') || (word[i]=='ó') || (word[i]=='ô') || (word[i]=='ö')) {
         	word[i] = 'o';
        }
      	if ((word[i]=='è') || (word[i]=='é') || (word[i]=='ê') || (word[i]=='ë')) {
         	word[i] = 'e';
        }
      	if ((word[i]=='ù') || (word[i]=='ú') || (word[i]=='û') || (word[i]=='ü')) {
         	word[i] = 'u';
        }
      	if ((word[i]=='ì') || (word[i]=='í') || (word[i]=='î') || (word[i]=='ï')) {
         	word[i] = 'i';
        }
    }
   	return(0);
}
