
#if !defined(_CONST_H_INCLUDED_)
#define _CONST_H_INCLUDED_

#include <config.h>

// Maximum length of a string and a document
#define MAX_STR_LEN		1024       // 1Kb
#define MAX_DOC_LEN		524288    // 500Kb 
//#define MAX_DOC_LEN		1048576    // 1000Kb Increased due to charset conversion  

// Note that in some environments, just asking for
// "char buffer[MAX_DOC_LEN]" may silently fail
// use malloc() and assert() instead

// IT'S NOT ENOUGH TO MODIFY THE FOLLOWING TYPEDEFS HERE
// YOU HAVE TO MODIFY THEM ALSO IN SOME PARTS OF THE CODE
// (ie: printf "%l", atol())

#define	docid_t		unsigned long
#define siteid_t	unsigned long

// This has to be twice as big than docid_t if we are expecting
// to use all the addressable space of docid_t

#define doc_hash_t  unsigned long

// Loglevel constants
#define LOGLEVEL_QUIET	0
#define LOGLEVEL_NORMAL	1
#define LOGLEVEL_VERBOSE	2

// ANSI colors (for reporting)

#define RED "[31m"
#define GRE "[32m"
#define BRO "[33m"
#define BLU "[34m"
#define NOR "[0m"

// Collection typical dir

#define	COLLECTION_TEXT		"text"
#define	COLLECTION_URL		"url"
#define COLLECTION_METADATA	"metadata"
#define COLLECTION_LINK		"link"
#define COLLECTION_SITELINK	"sitelink"
#define COLLECTION_HARVEST	"harvest"
#define COLLECTION_ANALYSIS	"analysis"
#define COLLECTION_INDEX	"index"
#define COLLECTION_SPLIT	"split"
#define COLLECTION_LOG		"log"

#endif
