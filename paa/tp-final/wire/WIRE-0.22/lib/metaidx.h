#ifndef _METAIDX_H_INCLUDED_
#define _METAIDX_H_INCLUDED_

#include <config.h>

// System libraries

#include <features.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>


#include <math.h>
#include <errno.h>
#include <queue>
#include <string>
#include <map>

using namespace std;

// Local libraries

#include "const.h"
#include "http_codes.h"
#include "xmlconf.h"
#include "die.h"
#include "utils.h"
#include "urlidx.h"
#include "http_charset.h"

// File names

#define METAIDX_FILENAME_SITE	"metaidx.site"
#define METAIDX_FILENAME_DOC	"metaidx.doc"

#define LINK_NORMALIZATION	((pagerank_t)1)

// Meta index status

enum metaidx_status_t {
	METAIDX_OK         = 0,
	METAIDX_ERROR,
	METAIDX_EOF
};

// Status of a document (doc.status)

enum doc_status_t {
	STATUS_DOC_ALL			= 0,
	STATUS_DOC_NEW			= 1,
	STATUS_DOC_ASSIGNED		= 2,
	STATUS_DOC_GATHERED		= 3,
	STATUS_DOC_INDEXED		= 4,
	STATUS_DOC_EXCLUSION	= 5,
	STATUS_DOC_IGNORED		= 99  // Must be ignored
};

#define DOC_STATUS_STR(x) (\
		x==0 ? "STATUS_DOC_ALL" :\
		x==1 ? "STATUS_DOC_NEW" :\
		x==2 ? "STATUS_DOC_ASSIGNED" :\
		x==3 ? "STATUS_DOC_GATHERED" :\
		x==4 ? "STATUS_DOC_INDEXED" :\
		x==5 ? "STATUS_DOC_EXCLUSION" :\
		x==99 ? "STATUS_DOC_IGNORED" : "(invalid)" )

// Status of a site (site.status)

enum site_status_t {
	STATUS_SITE_NEW			= 10,
	STATUS_SITE_VISITED		= 11
};
#define SITE_STATUS_STR(x) (\
		x==STATUS_SITE_NEW		? "STATUS_SITE_NEW" :\
		x==STATUS_SITE_VISITED  ? "STATUS_SITE_VISITED" : "(invalid)" )

// Type of a document (doc.mime_type)

enum mime_type_t {
	MIME_UNKNOWN			= 0,
	MIME_REDIRECT			= 1,
	MIME_ROBOTS_TXT			= 2,
	MIME_ROBOTS_RDF			= 3,
	MIME_TEXT_HTML			= 10,
	MIME_TEXT_PLAIN			= 11,
	MIME_APPLICATION_FLASH	= 12,
	MIME_APPLICATION		= 13,
	MIME_AUDIO				= 14,
	MIME_IMAGE				= 15,
	MIME_VIDEO				= 16,
	MIME_TEXT_WAP			= 17,
	MIME_TEXT_RTF			= 18,
	MIME_TEXT_XML			= 19,
	MIME_TEXT_TEX			= 20,
	MIME_TEXT_CHDR			= 21
};
#define MIME_TYPE_STR(x) (\
		x==MIME_UNKNOWN				? "MIME_UNKNOWN" :\
		x==MIME_REDIRECT			? "MIME_REDIRECT" :\
		x==MIME_ROBOTS_TXT			? "MIME_ROBOTS_TXT" :\
		x==MIME_TEXT_HTML			? "MIME_TEXT_HTML" :\
		x==MIME_TEXT_PLAIN			? "MIME_TEXT_PLAIN" :\
		x==MIME_APPLICATION_FLASH	? "MIME_APPLICATION_FLASH" :\
		x==MIME_APPLICATION			? "MIME_APPLICATION" :\
		x==MIME_AUDIO				? "MIME_AUDIO" :\
		x==MIME_IMAGE				? "MIME_IMAGE" :\
		x==MIME_VIDEO				? "MIME_VIDEO" :\
		x==MIME_TEXT_WAP			? "MIME_TEXT_WAP" :\
		x==MIME_TEXT_RTF			? "MIME_TEXT_RTF" :\
		x==MIME_TEXT_XML			? "MIME_TEXT_XML" :\
		x==MIME_TEXT_TEX			? "MIME_TEXT_TEX" :\
		x==MIME_TEXT_CHDR			? "MIME_TEXT_CHDR" :\
	   	"(invalid)" )

// Type for depth

typedef unsigned short int depth_t;

// Types for link ranking

typedef double siterank_t;
typedef double pagerank_t;
typedef double wlrank_t;
typedef double hubrank_t;
typedef double authrank_t;

// Type for freshness

typedef double freshness_t;

// Type for priority

typedef double priority_t;

// Type for a metaindex
// The document count is stored in doc[0].docid
// The site count is stored in site[0].siteid

typedef struct {
	FILE *file_site;
	FILE *file_doc;
	char dirname[MAX_STR_LEN];
	docid_t count_doc;
	siteid_t count_site;
	bool readonly;
} metaidx_t;

// Type for the metadata about a document

enum doc_field_t {
	DOC_FIELD_UNDEF = 0,
	DOC_FIELD_DEPTH,
	DOC_FIELD_IN_DEGREE,
	DOC_FIELD_OUT_DEGREE,
	DOC_FIELD_HTTP_STATUS
};

enum component_t {
	COMPONENT_UNDEF = 0,
	COMPONENT_MAIN_NORM,
	COMPONENT_MAIN_MAIN,
	COMPONENT_MAIN_IN,
	COMPONENT_MAIN_OUT,
	COMPONENT_IN,
	COMPONENT_OUT,
	COMPONENT_TIN,
	COMPONENT_TOUT,
	COMPONENT_TUNNEL,
	COMPONENT_ISLAND
};

#define COMPONENT_STR(x) (\
	(x==COMPONENT_UNDEF)		? "COMPONENT_UNDEF" : \
	(x==COMPONENT_MAIN_NORM)	? "COMPONENT_MAIN_NORM" : \
	(x==COMPONENT_MAIN_MAIN)	? "COMPONENT_MAIN_MAIN" : \
	(x==COMPONENT_MAIN_IN)		? "COMPONENT_MAIN_IN" : \
	(x==COMPONENT_MAIN_OUT)		? "COMPONENT_MAIN_OUT" : \
	(x==COMPONENT_IN)			? "COMPONENT_IN" : \
	(x==COMPONENT_OUT)			? "COMPONENT_OUT" : \
	(x==COMPONENT_TIN)			? "COMPONENT_TIN" : \
	(x==COMPONENT_TOUT)			? "COMPONENT_TOUT" : \
	(x==COMPONENT_TUNNEL)		? "COMPONENT_TUNNEL" : \
	(x==COMPONENT_ISLAND)		? "COMPONENT_ISLAND" : "(invalid)" )

typedef struct {
	// Main parameters
	docid_t			docid;
	siteid_t		siteid;
	doc_status_t   	status;
	mime_type_t		mime_type;

	// Harvester parameters
	int				http_status;
	off64_t			raw_content_length;
	float          	effective_speed;
	uint			number_visits;
	uint			number_visits_changed;
	time_t         	time_unchanged;
	time_t			first_visit;
	time_t			last_visit;
	time_t			last_modified;

	// Gatherer parameters
	off64_t			content_length;
	doc_hash_t		hash_value;
	docid_t			duplicate_of;

	// Seeder parameters
	depth_t			depth;
	bool			is_dynamic;

	// Manager parameters
	uint			in_degree;
	uint			out_degree;

	pagerank_t		pagerank;
	wlrank_t        wlrank;
	hubrank_t       hubrank;
	authrank_t      authrank;

	freshness_t     freshness;

	priority_t		current_score;
	priority_t		future_score;
} doc_old_t;

typedef struct {
	// Main parameters
	docid_t			docid;
	siteid_t		siteid;
	doc_status_t   	status;
	mime_type_t		mime_type;

	// Harvester parameters
	int				http_status;

	float          	effective_speed;
	float			latency;

	uint			number_visits;
	uint			number_visits_changed;

	time_t         	time_unchanged;
	time_t			first_visit;
	time_t			last_visit;
	time_t			last_modified;

	// Gatherer parameters
	off64_t			raw_content_length;
	off64_t			content_length;
	doc_hash_t		hash_value;
	docid_t			duplicate_of;

	// Seeder parameters
	depth_t			depth;
	bool			is_dynamic;

	// Manager parameters
	uint			in_degree;
	uint			out_degree;

	pagerank_t		pagerank;
	wlrank_t        wlrank;
	hubrank_t       hubrank;
	authrank_t      authrank;

	freshness_t     freshness;

	priority_t		current_score;
	priority_t		future_score;

	float			latency_connect;	// used for measuring times

    charset_t       charset; //4 Bytes
	char			reserved[8]; //Originally of size 12
} doc_t;


// Type for the metadata about a site

typedef struct {
	siteid_t		siteid;
	uint			count_doc;
	uint			count_error;
	time_t			last_visit;
	time_t          last_resolved;
	struct in_addr  addr;
	off64_t           raw_content_length;
	int             harvest_id;

	bool			has_exclusions;

	site_status_t	status;

	uint			count_doc_ok;
	uint			count_doc_static;
	uint			count_doc_dynamic;
	siterank_t		siterank;
	time_t			age_oldest_page;
	time_t			age_newest_page;
	time_t			age_average_page;
	siteid_t		in_degree;
	docid_t			reserved1;
	depth_t			max_depth;

	// Site graph structure
	component_t		component;
	docid_t			internal_links; // Internal links can be at most
									// the total number of documents, if
									// a website is completely internal links
	siteid_t		out_degree; 	

	uint			count_doc_gathered;
	uint			count_doc_new;
	uint			count_doc_assigned;
	uint			count_doc_ignored;

	pagerank_t		sum_pagerank;
	hubrank_t		sum_hubrank;
	authrank_t		sum_authrank;

	char			reserved2[2];
} site_old_t;

typedef struct {
	siteid_t		siteid;
	site_status_t	status;
	int             harvest_id;

	uint			count_doc;
	uint			count_error;

	uint			count_doc_ok;
	uint			count_doc_static;
	uint			count_doc_dynamic;
	uint			count_doc_gathered;
	uint			count_doc_new;
	uint			count_doc_assigned;
	uint			count_doc_ignored;

	time_t			age_oldest_page;
	time_t			age_newest_page;
	time_t			age_average_page;
	off64_t           raw_content_length;

	struct in_addr  addr;

	time_t			last_visit;
	time_t          last_resolved;
	time_t			last_checked_robots_txt;
	time_t			last_checked_robots_rdf;

	docid_t			docid_robots_txt;
	docid_t			docid_robots_rdf;

	bool			has_valid_robots_txt;
	bool			has_valid_robots_rdf;

	// Site graph structure
	siteid_t		in_degree;
	siteid_t		out_degree; 	
	depth_t			max_depth;
	component_t		component;
	siterank_t		siterank;
	docid_t			internal_links; // Internal links can be at most
									// the total number of documents, if
									// a website is completely internal links


	// Link rank summaries
	pagerank_t		sum_pagerank;
	wlrank_t		sum_wlrank;
	hubrank_t		sum_hubrank;
	authrank_t		sum_authrank;

	// Bytes
	off64_t			bytes_in;
	off64_t			bytes_out;

	// Time
	float			resolver_latency;

	char			reserved[480];
} site_t;

// Functions

// Open, close and remove metaidx
metaidx_t *metaidx_open( const char *dirname, bool readonly );
metaidx_status_t metaidx_close( metaidx_t *m );
void metaidx_remove( const char *dirname );

// Blank records
void metaidx_doc_default( doc_t *doc );
void metaidx_site_default( site_t *site );

// Retrieve and store
metaidx_status_t metaidx_site_retrieve( metaidx_t *m, site_t *site );
metaidx_status_t metaidx_site_store( metaidx_t *m, site_t *site );
metaidx_status_t metaidx_doc_retrieve( metaidx_t *m, doc_t *doc );
metaidx_status_t metaidx_doc_store( metaidx_t *m, doc_t *doc );

// Dump

void metaidx_dump_doc_status( doc_t *doc );
void metaidx_dump_doc_short_status( doc_t *doc );
void metaidx_dump_site_status( site_t *site );
void metaidx_dump_status( metaidx_t *metaidx );

void metaidx_dump_doc_header( FILE *out );
void metaidx_dump_doc( doc_t *doc, FILE *out );

void metaidx_dump_sitelist( metaidx_t *metaidx, urlidx_t *urlidx, FILE *out );
void metaidx_dump_site_header( FILE *out );
void metaidx_dump_site( site_t *site, char *sitename, FILE *out );

// Get mime type of a mime_string

mime_type_t metaidx_mime_type( char *path, char *mime_str );

// Convert

void metaidx_convert_old_site_file( char *infile );
void metaidx_convert_old_doc_file( char *infile );

#endif
