#ifndef _LINKIDX_H_INCLUDED_
#define _LINKIDX_H_INCLUDED_

#include <config.h>

// System libraries

#include <stdlib.h>
#include <assert.h>
#include <fstream>

// Local libraries

#include <const.h>
#include <metaidx.h>
#include <storage.h>
#include <urlidx.h>

// Filenames

#define FILENAME_LINKS_DOWNLOAD	"_links_download.txt"
#define FILENAME_LINKS_LOG		"_links_log.txt"
#define FILENAME_LINKS_STAT		"_links_stat.txt"
#define FILENAME_RESOLVED_LINKS "_links_download_resolved.txt"
#define DEPTH_UNDEFINED			(200)

// Constants

#define LINK_MAX_OUTDEGREE	350
#define LINK_CAPTION_FORBIDDEN		"<FORBIDDEN>"
#define LINK_CAPTION_LAST_MODIFIED	"<LAST-MODIFIED>"

#define LINK_SCORE_MAX_ITERATIONS			(500)
#define LINK_ANALYSIS_CALC_PAGERANK			(1<<0)
#define LINK_ANALYSIS_CALC_WIRESCORE		(1<<1)
#define LINK_ANALYSIS_CALC_HITS				(1<<2)


// Enumeration for the emphasis tags
enum em_tag_t {
    TAG_UNDEF	= 0,
    TAG_H1		= 1,
    TAG_H2		= 2,
    TAG_H3		= 3,
    TAG_H4		= 4,
    TAG_H5		= 5,
    TAG_H6		= 6,
    TAG_B		= 7,
    TAG_FONT	= 8,
    TAG_STRONG	= 9
};

#define TAG_STR(x) (\
		(x == TAG_UNDEF)	? "TAG_UNDEF"	:\
		(x == TAG_H1)		? "TAG_H1"		:\
		(x == TAG_H2)		? "TAG_H2"		:\
		(x == TAG_H3)		? "TAG_H3"		:\
		(x == TAG_H4)		? "TAG_H4"		:\
		(x == TAG_H5)		? "TAG_H5"		:\
		(x == TAG_H6)		? "TAG_H6"		:\
		(x == TAG_B)		? "TAG_B"		:\
		(x == TAG_FONT)		? "TAG_FONT"	:\
		(x == TAG_STRONG)	? "TAG_STRONG"  :\
		"(invalid)" )



// Link index status

enum linkidx_status_t {
	LINKIDX_OK         = 0,
	LINKIDX_ERROR,
	LINKIDX_DUPLICATE
};

// Structures. This contains a storage_t inside

typedef struct {
	char dirname[MAX_STR_LEN-1];
	bool readonly;
	storage_t *storage;
} linkidx_t;

// Structure to store a out link used for ranking calculations
typedef struct{
	docid_t dest;
	char rel_pos;
	char tag;
	int anchor_length;
} out_link_t;

// Type for link_weight

typedef double linkweight_t;

// Functions

linkweight_t calculate_link_weight(out_link_t link);

linkidx_t *linkidx_open( const char *dirname, bool readonly );
void linkidx_close( linkidx_t *linkidx );
void linkidx_remove( const char *dirname );

linkidx_status_t linkidx_store( linkidx_t *linkidx, docid_t src, out_link_t dest[], uint degree );
linkidx_status_t linkidx_retrieve( linkidx_t *linkidx, docid_t src, out_link_t dest[], uint *degree );
linkidx_status_t linkidx_retrieve_using_buffer( linkidx_t *linkidx, char *buffer, docid_t src, out_link_t dest[], uint *degree );

void linkidx_prepare_sequential_read( linkidx_t *linkidx );

void linkidx_dump_links( linkidx_t *linkidx, docid_t src );
void linkidx_dump_links_checking( linkidx_t *linkidx, docid_t src, char *ok );
void linkidx_dump_links_with_url( linkidx_t *linkidx, urlidx_t *urlidx, docid_t src_docid );
void linkidx_show_links( linkidx_t *linkidx, docid_t src );
void linkidx_dump_status( linkidx_t *linkidx );
void linkidx_fix_depths( linkidx_t *linkidx, metaidx_t *metaidx, urlidx_t *urlidx );

linkidx_status_t linkidx_link_analysis(
		linkidx_t *linkidx, metaidx_t *metaidx,
		bool calc_pagerank, bool calc_wlrank, bool calc_hits,
		bool linearize );

// INLINED functions (used by sitelink)

//
// Name: linkidx_sequential_read
//
// Description:
//   Reads the links for the next document
//
// Input:
//   linkidx - the structure
//   buffer - a buffer
//
// Output:
//   dest[] - destinations of links
//   outdegree - out degree
//

inline void linkidx_sequential_read( linkidx_t *linkidx, out_link_t dest[], uint *out_degree ) {
	storage_record_t rec;
	storage_status_t rc;
	rc = storage_sequential_read( linkidx->storage, &(rec), (char *)(dest) );

	if( rc == STORAGE_NOT_FOUND ) {

		*out_degree = 0;

	} else {

		// Get out_degree
		*out_degree = rec.size/sizeof(out_link_t);

	}
}

// 
// Name: linkidx_sequential_skip
//
// Description:
//   Skips a number of records, this is useful to skip
//   several documents with zero out-degree
//
// Input:
//   linkidx - the structure
//   skip - the number of records to skip
//

inline void linkidx_sequential_skip( linkidx_t *linkidx, uint skip ) {
	storage_sequential_skip( linkidx->storage, skip );
}

// 
// Name: linkidx_calc_delta
//
// Description:
//   Returns the ratio between the larger and the smaller value
//   minus one.
//
// Input:
//   a, b - numbers to divide
//

inline double linkidx_calc_delta( double a, double b ) {
	if( a > b && b > 0 ) {
		return (a/b) - (double)1;
	} else if( a < b && a > 0 ) {
		return (b/a) - (double)1;
	} else {
		return 0;
	}
}

#endif
