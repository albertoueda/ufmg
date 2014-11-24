// Class IrudikoGenericReader

#include "irudikogenericreader.h" // class's header file

#define TOGGLETAGS_SPECIAL_CHARS 11
#define TOGGLETAGS_USELESS_TAGS 5
#define TOGGLETAGS_BLANK_CHAR_TYPE1 1
#define TOGGLETAGS_BLANK_CHAR_TYPE2 2

// class constructor
IrudikoGenericReader::IrudikoGenericReader()
{
	executeStem = false;
	fileType = HTML;
}

IrudikoGenericReader::IrudikoGenericReader(bool stemPhase, IrudikoFileType _filetype) {
	executeStem = stemPhase;
	fileType = _filetype;
}

IrudikoGenericReader::IrudikoGenericReader(IrudikoFileType _filetype) {
        executeStem = false;
	fileType = _filetype;
}

IrudikoGenericReader::IrudikoGenericReader(bool stemPhase) {
        fileType = HTML;
	executeStem = stemPhase;
}

void IrudikoGenericReader::setType(IrudikoFileType _filetype) {
        fileType = _filetype;
}

// class destructor
IrudikoGenericReader::~IrudikoGenericReader()
{
	// insert your code here
}

// main procedure
int IrudikoGenericReader::process(const char * filename, vector<string>& response)
{
        wordlist.clear();

	infile.open(filename, ios::binary);
	if (!infile.is_open()) return -1;
	preprocess();
	removeStopwords();
	if (executeStem) stem();
	
	response=wordlist;
	return 0;
}

int IrudikoGenericReader::process_and_sketch(char * buf, unsigned int bufsize, unsigned long * sketch, int sketch_dim) {
        sketch_gen.assign(3, SH_MOD_SELECTION, 0);
        wordlist.clear();

	preprocess_alt(buf, bufsize);
	
	removeStopwords();
	
	if (executeStem) stem();
							  
	sketch_gen.do_sketch(wordlist,sketch,sketch_dim,NULL);

	return 0;
}

int IrudikoGenericReader::process_and_sketch(char * buf, unsigned int bufsize, unsigned long * sketch, int sketch_dim, int _sh_selmode, int _sh_param) {
        sketch_gen.assign(3, _sh_selmode, _sh_param);
        wordlist.clear();

	preprocess_alt(buf, bufsize);
	
	removeStopwords();
	
	if (executeStem) stem();
	
	sketch_gen.do_sketch(wordlist,sketch,sketch_dim,NULL);

	return 0;
}

int IrudikoGenericReader::process_and_sketch(char * buf, unsigned int bufsize, unsigned long * sketch, int sketch_dim, int _sh_selmode, int _sh_param, char* exclfile) {
        sketch_gen.assign(3, _sh_selmode, _sh_param);
        wordlist.clear();

	preprocess_alt(buf, bufsize);
	
	removeStopwords();
	
	if (executeStem) stem();
	
	sketch_gen.do_sketch(wordlist,sketch,sketch_dim,exclfile);

	return 0;
}

// Pre-processing function
int IrudikoGenericReader::preprocess()
{
	// Get length of file
	infile.seekg(0, ios::end);
	int in_length = infile.tellg();
	infile.seekg(0, ios::beg);
	
	// Initializations
	char* content = new char[in_length+1];
	
	infile.read(content,in_length);
	
	int in_length_2 = toggleZeroChars(content,in_length);

	if ((fileType == XML) || (fileType == HTML)) 
	  in_length_2 = toggleTags(content,in_length_2);
	
	infile.close();
	in_length_2 = toggleNonAlphaChars(content,in_length_2);

	splitString(content,in_length_2,wordlist);
	
	delete[] content;

	return 0;
}

// Alternative pre-processing function
int IrudikoGenericReader::preprocess_alt(char* content, unsigned int in_length)
{
	// Initializations
        int in_length_2 = toggleZeroChars(content,in_length);

	if ((fileType == XML) || (fileType == HTML)) 
	  in_length_2 = toggleTags(content,in_length_2);

	in_length_2 = toggleNonAlphaChars(content,in_length_2);
		
	splitString(content,in_length_2,wordlist);

	return 0;
}

// No description
int IrudikoGenericReader::removeStopwords()
{
	list<string> stopwordList;
	
	if (getStopwordList(stopwordList) == 0) {
		vector<string> vectCopy;
		vector<string>::iterator docItr; // document iterator
		list<string>::iterator swlItr;   // stopword_list iterator
		for (docItr=wordlist.begin();docItr!=wordlist.end();docItr++) {
			string curElement = (string)*docItr;
			if ((!binary_search(stopwordList.begin(), stopwordList.end(), curElement)) && (curElement.size()>2)){
				vectCopy.push_back(curElement);
			}
		}
		wordlist=vectCopy;
		return 0;
	}
	
	return 0;
}

int IrudikoGenericReader::getStopwordList(list<string>& stopwordList)
{
	char bufArray[32];
	ifstream in_swfile;
	string swFname = getStopwordFileName();
        if (swFname == "") return -1;
	in_swfile.open(getStopwordFileName());
	
	if (!in_swfile.is_open()) return -1;
	
	while (!in_swfile.eof()) {
		in_swfile.getline(bufArray, 31);
		string s(bufArray);
		
		stopwordList.push_back(s);
	}
	
	stopwordList.sort();
	
	in_swfile.close();
	
	return 0;
}

int IrudikoGenericReader::toggleTags(char* content, int content_len) {
  
  char* all_start = content;

  char* igr_deleteall_tags[TOGGLETAGS_USELESS_TAGS] = {  "object", "script", "head", "style", "noscript" };
    
  // Phase 1 (the core): every HTML/XML tag got to be marked
  while(true) {

        char* left = strchr((all_start), (unsigned char)'<');
	char* right = 0;
	if (left==NULL) {
	  break;
	} else {
	  right = strchr((left),(unsigned char)'>');
	  if (right==NULL) break;
	}
	
	if (left < right) {
	        const char* _dtag = NULL;

	        char* tag = new char[right-left];
		strncpy(tag,left+1,right-left-1); tag[right-left-1]='\0';
		
		all_start = left;
		//Not used anymore, as it is made useless by Phase 1 now: strtolow(tag);
		
		int is_deletable = 0;
		
		if (strncmp(tag,"!--",3)==0) {
		  is_deletable = 2;
		}
		
		for (int i=0;(is_deletable == 0) && (i < TOGGLETAGS_USELESS_TAGS); ++i) {
		   if (strncmp(tag,igr_deleteall_tags[i],strlen(igr_deleteall_tags[i]))==0) {
		     _dtag = igr_deleteall_tags[i];
		     is_deletable = 1;
		   }
		}
		
		delete[] tag;

		if (is_deletable == 1) {
		   unsigned int tag_k = 1;
		   char* d_right = right;
		   int _dtag_len = strlen(_dtag);
		   
		   char* _dtag_close = new char[_dtag_len+1];
		   _dtag_close[0]='/'; 
		   
		   memcpy((char*)(_dtag_close+1),(const char*)_dtag,_dtag_len);
		   _dtag_close[_dtag_len+1]='\0';
		   
		   while (tag_k>0) {
		     char* d_left = strchr((d_right+1),(unsigned char)'<');

		     if (d_left==NULL) {
		       break;
		     } else {
			d_right = strchr(d_left,(unsigned char)'>');
			if (d_right==NULL) break;
		     }
		     
		     if (d_left < d_right) {
		       
		       char* d_tag = new char[d_right-d_left];
		       strncpy(d_tag,d_left+1,d_right-d_left-1);
		       d_tag[d_right-d_left-1]='\0';

		       //Not used anymore: strtolow(d_tag);
		       
		       if (strncmp(d_tag,_dtag,_dtag_len)==0) {
			 ++tag_k;
		       } else if (strncmp(d_tag,_dtag_close,_dtag_len+1)==0) {
			 --tag_k;
		       }
			
		       if (tag_k == 0) {
			 right = d_right;
			 break;
		       }

		       delete[] d_tag;
		     }

		   }
		   delete[] _dtag_close;
		} else if (is_deletable == 2) {
			char* d_right = strstr((left+1),"-->");
			
			if (d_right!=NULL) {
				right = d_right+2;
			}
		}
	        
		memset(left,TOGGLETAGS_BLANK_CHAR_TYPE1,right-left+1);
 	}   
  }
  
  // Phase 2: replacement of HTML special sequences with their associated characters
  const char * special_src[TOGGLETAGS_SPECIAL_CHARS] = { "&lt;", "&gt;", "&amp;", "&nbsp;", "&eacute;", "&egrave;", "&agrave;",
			     "&igrave;", "&ograve;", "&ugrave;", "&quot;" };
  const char special_dst[TOGGLETAGS_SPECIAL_CHARS] = { '<', '>', '&', ' ', 'é', 'è', 'à', 'ì', 'ò', 'ù', '\"'};
  
  for (int i = 0; i < TOGGLETAGS_SPECIAL_CHARS; ++i) {
    char* substr_ptr = strstr(content, special_src[i]);
    if (substr_ptr != NULL) {
      *substr_ptr = special_dst[i];
      memset((substr_ptr+1),TOGGLETAGS_BLANK_CHAR_TYPE2,strlen(special_src[i])-1);
    }
  }

  // Phase 3: the final cleanup
  int j = 0;
  int addAWhiteSpace = 1;

  for (int i = 0; i < content_len; i++) {

    if ((content[i]!=TOGGLETAGS_BLANK_CHAR_TYPE1) && (content[i]!=TOGGLETAGS_BLANK_CHAR_TYPE2)) {
      if (j < i) content[j]=content[i];
      ++j;
      if (addAWhiteSpace == 0) addAWhiteSpace = 1;
    } else if ((content[i]==TOGGLETAGS_BLANK_CHAR_TYPE1) && (addAWhiteSpace)) {
      content[j]=' ';
      ++j;
      addAWhiteSpace=0;
    }
  }

  content[j]='\0';

  return j;
}

int IrudikoGenericReader::toggleNonAlphaChars(char* content, int len) {
  int phase1_j = 0;
  int lastCharIsInvalid = 0;
  for (int i = 0; i < len; i++) {
    int k = (int)content[i];
    if (((k >= 65) && (k <= 90)) || ((k >= 97) && (k <= 122)) || (k > 127)) {
      if (i > phase1_j) content[phase1_j] = std::tolower(content[i]);
      ++phase1_j;
      lastCharIsInvalid = 0;
    } else {
      if (lastCharIsInvalid == 0) {
	content[phase1_j] = ' ';
	phase1_j++;
	lastCharIsInvalid = 1;
      }
    }
  }
  content[phase1_j]='\0';
  return phase1_j;

}

int IrudikoGenericReader::toggleZeroChars(char* buf, int len) {
        
        int j = 0;
	int newLine = 0;
	char*startIdx;
	//int htmlVal = 0;
	if ( (fileType == HTML) || (fileType == XML) ) {
	    startIdx = (strchr(buf, '<')==NULL)?buf:strchr(buf,'<');
	} else {
	    startIdx = buf;
	}
	
	int nLen = buf+len-startIdx;
	
	for (int i=0; i < len; i++) {
	  if (i <= nLen) {
	    buf[i]=startIdx[i];
	  } else {
	    buf[i]='\0';
	  }
	}
	
	if ( (fileType == HTML) || (fileType == XML) ) {
	    char*endIdx = strrchr(buf, '>'); nLen=(endIdx==NULL)?strlen(buf):(endIdx-buf);
	} else {
	    nLen = strlen(buf);
	}
        
	for (int i=nLen+1; i < len; i++) {
	  buf[i]='\0';
	}
	
	for (int i=0; i <= nLen; i++) {
                
		if ((buf[i] < 0) || ((buf[i] > 31) && (buf[i] != 127))) {
			if (newLine==1) newLine=0;
			if (i > j) buf[j] = buf[i];
			j++;
			/*
			if ((htmlVal < 6)&&(htmlTag[htmlVal]==buf[i])) {
				htmlVal ++;
			} else if (htmlVal < 6) {
				htmlVal = 0;
			} else if ((htmlVal == 6) && (j2==0)){
				j2 = i;
			}
			*/
		} else if ((newLine==0)&&((buf[i] == 10) || (buf[i] == 13))) {
			buf[j]=' '; j++; newLine=1;
		}
	}
	
	buf[j]='\0';

	return j;
}

int IrudikoGenericReader::splitString(char* input, int isize, vector<string>& results) {
    if(isize == 0) return 0;
    // splitting it now
    int numFound = 0;
    char* pch = strtok(input, " ");
    
    while (pch != NULL) {
      string s(pch, strlen(pch));
      ++numFound;
      results.push_back(s);

      pch = strtok(NULL, " ");
    }
    
    return numFound;
}

string replaceAll(string s, string f, string r) {
  unsigned int found = s.find(f);
  while(found != string::npos) {
    s.replace(found, f.length(), r);
    found = s.find(f);
  }
  return s;
}
