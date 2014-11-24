// Class IrudikoSketchGenerator

#include "irudikosketchgenerator.h" // class's header file

class ShinglingCompare {
	private:
		int max;
	public:
	ShinglingCompare(int k) {
		max = k;
	};
	bool operator() (shingling a, shingling b) {
		for (int k = 0; k < max; k++) {
    		if (a.subseq[k] < b.subseq[k]) {
    			return true;
			} else if (a.subseq[k] > b.subseq[k]) {
				return false;
			}
		}
		return false;
	};
	static bool are_equal(shingling a, shingling b, int k_max) {
		for (int k = 0; k < k_max; k++) {
    		if (a.subseq[k] != b.subseq[k]) {
    			return false;
			}
		}
		return true;
	}
};

// procedure assign
void IrudikoSketchGenerator::assign(int _shingling_dim, int _sh_selmode, int _sh_param) {
  shingling_dim = _shingling_dim;
  sh_selmode = _sh_selmode;
  sh_param = _sh_param;
}

// class constructors
IrudikoSketchGenerator::IrudikoSketchGenerator(int _shingling_dim, int _sh_selmode, int _sh_param = 16) {
	shingling_dim = _shingling_dim;
	sh_selmode = _sh_selmode;
	sh_param = _sh_param;
}

IrudikoSketchGenerator::IrudikoSketchGenerator() {
	IrudikoSketchGenerator(DEF_SHINGLING_W,SH_MIN_SELECTION,100);
}

IrudikoSketchGenerator::IrudikoSketchGenerator(int _shingling_dim) {
	IrudikoSketchGenerator(_shingling_dim,SH_MIN_SELECTION,100);
}

// class destructor
IrudikoSketchGenerator::~IrudikoSketchGenerator()
{
	// insert your code here
}

int IrudikoSketchGenerator::do_sketch(vector<string> wordlist, unsigned long* dest_sketch, int def_perm_num=DEF_PERMUTATION_NUM,char* exclfile=NULL) {
	vector<shingling> l_sh;
	list<unsigned long> ulist;
	list<shingling> excllist;
	sketch.clear();
	/* Build the w-shingling */
	if (wordlist.size()>0) l_sh = shingle(wordlist);
	/* Optional exclusion of bags-of-words */
	if(exclfile!=NULL) {
	  ifstream exfile;
	  exfile.open(exclfile, ios::in);
	  if (exfile.is_open()) {
	    char mybuf [256];
	    while(exfile.getline(mybuf,255)) { 
	      //rows are in the format 
	      //<(w,w,w),f>
	      shingling sh; 
	      sh.subseq=new string[shingling_dim];
	      char* sptr1 = mybuf+2;
	      char* sptr2 = mybuf+2;
	      char* sptrend = strchr(sptr1,')');
	      unsigned int h = 0;
	      while(1) {
		sptr2 = strchr(sptr1,','); 
		if (sptr2 < sptrend) {
		  *sptr2 = 0; 
		  string s(sptr1,sptr2-sptr1);
		  sh.subseq[h++]=s;
		  sptr1 = sptr2+1;
		} else {
		  *sptrend = 0;
		  string s(sptr1,sptrend-sptr1);
		  sh.subseq[h]=s;
		  sptr1=sptrend+2;
		  sptr2=strchr(sptr1,'>');
		  *sptr2 = 0;
		  sh.frequency = atoi(sptr1);
		  break;
		}
	      }
	      excllist.push_back(sh);
	    }
	  }
	  exfile.close();
	}
	sort(l_sh.begin(),l_sh.end(),ShinglingCompare(shingling_dim));
	ShinglingCompare sc(shingling_dim);
	for(list<shingling>::iterator citr=excllist.begin(); 
	    citr != excllist.end(); citr++) {
	  shingling s2 = *citr;
	    
	  int bs_high = l_sh.size()-1;
	  int bs_low = 0;
	  while(bs_low<=bs_high) {
	    int bs_mid = (bs_low+bs_high) / 2;
	    shingling s1 = *(l_sh.begin()+bs_mid);
	    if(ShinglingCompare::are_equal(s1,s2,shingling_dim)) { // found at bs_mid
	      //cout << "found;";
	      s1.frequency -= s2.frequency;
	      if(s1.frequency <= 0) 
		l_sh.erase(l_sh.begin()+bs_mid);
	      else 
		*(l_sh.begin()+bs_mid) = s1;
	      
	      break;
	    } else {
	      if (!sc.operator()(s1,s2))
		bs_high = bs_mid - 1;
	      else 
		bs_low = bs_mid + 1;
	      
	    }
	  }
	}
	/* Convert the bag-of-words in a compatible set */
	if (wordlist.size()>0) ulist = get_ul_list(l_sh);
	//cout << ulist.size() << ":" << endl;
	/* Apply a way to redimensionate the list above */
	switch (sh_selmode) {
		case (SH_MIN_SELECTION): { minShingles(ulist,sh_param); break; }
		case (SH_MOD_SELECTION): { if (sh_param==0) sh_param = chooseModParam(ulist.size()); modShingles(ulist,sh_param); break; }
	}
	
	//cout << sh_selmode << "," << sh_param << "::" << ulist.size() << endl;
	/* Compute the singular permutations */
	for(int i=0; i < def_perm_num; i++) {
		unsigned long min_hashval = 0;
		bool hashval_init = false;
		for(list<unsigned long>::iterator itr=ulist.begin();itr!=ulist.end();itr++) {
			unsigned long l = (unsigned long)*itr;
			l = hash_permute(l,i+1);
			if ((!hashval_init) || (l < min_hashval)) {
				min_hashval = l;
				if (!hashval_init) hashval_init=true;
			}
		}
		sketch.push_back(min_hashval);
	}
	
	for (int i = 0; i < def_perm_num; i++) {
		dest_sketch[i]=sketch[i];
	}
	
	for (vector<shingling>::iterator itr=l_sh.begin();itr!=l_sh.end();itr++) {
	        delete[] (((shingling)*itr).subseq);
	}

	return 0;
}

vector<shingling> IrudikoSketchGenerator::shingle(vector<string> wordlist) {
	vector<shingling> res, res2;
	
	int i = 0, remElems = wordlist.size()+1-shingling_dim;
	
	if (remElems < 1) remElems = 1;

	for (vector<string>::iterator itr=wordlist.begin();
	     remElems!=0;itr++,i++,remElems--) {
		// W-Shingling Allocation
		shingling shingle;
		shingle.subseq = new string[shingling_dim];
		shingle.frequency = 1;
		
		vector<string>::iterator itrCopy = itr;
		
		for (int j = 0; j < shingling_dim; j++) {
		        if (itrCopy == wordlist.end()) {
			  shingle.subseq[j] = "";
			} else {
			  shingle.subseq[j] = (string)*itrCopy;
			  itrCopy++;
			}
		}
		// add it
		res.push_back(shingle);
	}
	
	bool setNewShingling = true;
	
	shingling sh_cur, sh_copy;

	sort(res.begin(),res.end(),ShinglingCompare(shingling_dim));
	
	for (vector<shingling>::iterator itr=res.begin();itr!=res.end();itr++) {
		if (setNewShingling) {
		  sh_cur = (shingling)*itr;
		  sh_copy.frequency=sh_cur.frequency;
		  sh_copy.subseq=new string[shingling_dim];
		  for (int j = 0; j < shingling_dim; j++) {
		    string s(sh_cur.subseq[j]);
		    sh_copy.subseq[j] = s;
		  }

		}
		  
		vector<shingling>::iterator itr2 = itr;
		itr2++;
		
		if (itr2!=res.end()) {
			shingling sh_next = (shingling)*itr2;
			bool b = ShinglingCompare::are_equal(sh_copy,sh_next,shingling_dim);
			if (b) {
				sh_copy.frequency+=sh_next.frequency;
				setNewShingling = false;
			} else {
				setNewShingling = true;
			}
			
		}
		
		if (setNewShingling) {
			res2.push_back(sh_copy);
		} else {

		}
	}
	
	for (vector<shingling>::iterator itr=res.begin();itr!=res.end();itr++) {
		delete[] (((shingling)*itr).subseq);
	}

	return res2;
}

unsigned long IrudikoSketchGenerator::hash_permute(unsigned long x, int idx) {
	unsigned long a, b; // coefficients
	unsigned long hash = 0;
	
	/* I would like to have a fixed sequence of random numbers, in order to
	  ensure to always have the same permutation coefficient sequence */
	long seed = ((idx+1)*22369);
	rnd_init(524287,seed);
	a = 1+rnd_return(1073676287);
	b = rnd_return(1073676287);
	
  	hash = (a*x+b) % 1073676287; // 49979687 is a prime number
   	
   	return (hash /*& 0x7FFFFFFF*/);
}

double IrudikoSketchGenerator::resemblance(vector<unsigned long> sketch2) {
	vector<unsigned long>::iterator itr1, itr2;
	itr1=sketch.begin(); itr2=sketch2.begin();
	
	int resembl_est = 0;
	
	for (int i=0;i < dpn; i++) {
		unsigned long a = (unsigned long)*itr1;
		unsigned long b = (unsigned long)*itr2;
		
		if (a==b) resembl_est++;
		itr1++;itr2++;
	}
	
	return ((double)resembl_est)/dpn;
}

double IrudikoSketchGenerator::resemblance(unsigned long* u1, unsigned long* u2, int _sketch_len) {
  int resembl_est=0;
  for (int i = 0; i < _sketch_len; i++) {
    if (u1[i]==u2[i]) ++resembl_est;
  }
  return ((double)resembl_est)/_sketch_len;
}

void IrudikoSketchGenerator::rnd_init(long s1, long s2) {
	rnd_s1 = s1; rnd_s2 = s2;
}

long IrudikoSketchGenerator::rnd_return(long max)
{
	static double factor = 1.0/2147483563.0;
	register long k,z;
	k= rnd_s1 /53668;
	rnd_s1 =40014*(rnd_s1%53668)-k*12211;
	if (rnd_s1 < 0) rnd_s1 += 2147483563;
	k=rnd_s2/52774;
	rnd_s2=40692*(rnd_s2%52774)-k*3791;
	if (rnd_s2 < 0) rnd_s2 += 2147483399;

	/*
	z = abs(s1 ^ s2);
	*/
	z= (rnd_s1 - 2147483563) + rnd_s2;
	if (z < 1) z += 2147483562;
	
	double unif_x = (((double)(z))*factor);
	return (long)(max*unif_x);
}

unsigned long IrudikoSketchGenerator::convert_shingle(shingling sh) {
	int temp_size = 37;
	char* temp_array="abcdefghijklmnopqrstuvwxyz,0123456789";
	char num_array[255];
	unsigned long x = 5381;
	string s("");

	ostringstream oss;
	
	oss << sh.frequency;
	
	for (int i = 0; i < shingling_dim; i++) {
		oss << sh.subseq[i];        
		if ((i+1) < shingling_dim) oss << ",";
	}
	
	s = oss.str();

	for (int i = 0; i < temp_size; i++) {
		num_array[(int)temp_array[i]]=i+1;
	}
	
	// DJB HASH (Author: Daniel J. Bernstein)
	for(unsigned int i = 0; i < s.length(); i++)
   	{
	  x=((x<<5)+x)+num_array[(int)s[i]];
	}
   	
	return x;
}

int IrudikoSketchGenerator::minShingles(list<unsigned long>& sh_l, unsigned int w) {
	list<unsigned long> temp_list;
	
	for(list<unsigned long>::iterator itr=sh_l.begin();itr!=sh_l.end();itr++) {
	        unsigned long ul = hash_permute((unsigned long)*itr, 18);
		list<unsigned long> tl; tl.push_back(ul);
		temp_list.merge(tl);
	}
	
	if (w < sh_l.size()) {
		temp_list.resize(w);
	}
	sh_l = temp_list;

	return 0;
}

int IrudikoSketchGenerator::modShingles(list<unsigned long>& sh_l, unsigned int m) {
	list<unsigned long> temp_list;
	list<unsigned long>::iterator itr = sh_l.begin();
	
	for(;itr!=sh_l.end();itr++) {
	  unsigned long ul = hash_permute((unsigned long)*itr, 18);
	  if ((ul%m)==0) {
	    //cout << "..." << ul ;
	    temp_list.push_back(ul);
	  }
	}
	
	sh_l = temp_list;
	return 0;
}

list<unsigned long> IrudikoSketchGenerator::get_ul_list(vector<shingling>& l_sh) {
	list<unsigned long> res_list;
	
	for (vector<shingling>::iterator itr=l_sh.begin();itr!=l_sh.end();itr++) {
		shingling cur_sh = (shingling)*itr;
		
		unsigned int sh_freq = cur_sh.frequency;
		
		for (unsigned int j=1;j<=sh_freq;j++) {
			cur_sh.frequency = j;
			unsigned long l = convert_shingle(cur_sh);
			res_list.push_back(l);
		}
	}
	
	return res_list;
}

int IrudikoSketchGenerator::chooseModParam(int dim) {
	int i = ((int)(log(dim / 100)/log(2.0)));
	int k = 1;
	
	for (int j=i;j>0;--j) {
		k*=2;
	}
	
	return k;
}
