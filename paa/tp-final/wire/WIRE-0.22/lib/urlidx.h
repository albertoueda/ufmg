
#ifndef _URLIDX_H_
#define _URLIDX_H_

#include <config.h>

// System libraries

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <string.h>
#include <queue>
#include <string>
#include <errno.h>

using namespace std;

// Local libraries

#include "const.h"
#include "xmlconf.h"
#include "perfect_hash.h"
#include "utils.h"

// Constants

#define FILENAME_ROBOTS_TXT			"robots.txt"
#define FILENAME_ROBOTS_RDF			"robots.rdf"

// Filenames

#define URLIDX_FILENAME_SITE_LIST	"urlidx.sitelist"
#define URLIDX_FILENAME_PATH_LIST   "urlidx.pathlist"
#define URLIDX_FILENAME_SITE		"urlidx.site"
#define URLIDX_FILENAME_PATH		"urlidx.path"
#define URLIDX_FILENAME_SITE_HASH   "urlidx.sitehash"
#define URLIDX_FILENAME_PATH_HASH   "urlidx.pathhash"
#define URLIDX_FILENAME_ALL         "urlidx.main"

#define URLIDX_PATH_LEN (sizeof(siteid_t) + MAX_STR_LEN + sizeof(docid_t))

#define URLIDX_MAX_OCCUPANCY 		((float)0.8)

// Status

enum urlidx_status_t { 
	URLIDX_ERROR 			= 0,
	URLIDX_CREATED_SITE		= 1,
	URLIDX_CREATED_PATH		= 2,
	URLIDX_EXISTENT			= 3,
	URLIDX_NOT_FOUND		= 4
};

// Constants

const off64_t URLIDX_EXPECTED_SITENAME_SIZE = 100;

// Urlidx structure
// Note that this is saved "as-is" to disk, so if you change it,
// it will misbehave.

typedef struct {
	// Sites
	char *site;								// Strings    [memory]
	off64_t *site_hash;                       // Hash table [memory]
	siteid_t site_count;					// Count      [memory]
	off64_t site_next_char;					// String Pos [memory]
	FILE *site_list;                        // List       [disk]

	// Path
	FILE *path_file;						// Strings    [disk]
	off64_t *path_hash;                       // Hash table [memory]
	docid_t path_count;						// Count      [memory]
	off64_t path_next_char;					// String Pos [memory]
	FILE *path_list;                        // List       [disk]

	// Info
	char dirname[MAX_STR_LEN];
	bool readonly;
} urlidx_t;

// Functions

// New URL index
urlidx_t *urlidx_new( const char *dirname );

// Open a URL index
urlidx_t *urlidx_open( const char *dirname, bool readonly );

// Removes a URL index
void urlidx_remove( const char *dirname );

// Dumps the URL index status
void urlidx_dump_status(urlidx_t *u);

// Close an URL index
void urlidx_close(urlidx_t *u);

// Get the siteid for a site
urlidx_status_t urlidx_resolve_site( urlidx_t *u, const char *site, siteid_t *siteid );

// Get a docid for a path
urlidx_status_t urlidx_resolve_path( urlidx_t *u, siteid_t siteid, const char *path, docid_t *docid );

// Get a siteid and docid for a "site/path"
urlidx_status_t urlidx_resolve_url( urlidx_t *u, char *url, siteid_t *siteid, docid_t *docid );

// Get a sitename from a siteid
void urlidx_site_by_siteid( urlidx_t *u, siteid_t siteid, char *name );

// Get a "site/path" from a docid
void urlidx_url_by_docid(   urlidx_t *u, docid_t  docid,  char *url );

// Get the siteid of a docid, useful for double-checking
siteid_t urlidx_siteid_by_docid( urlidx_t *u, docid_t docid );

// Get a path from a docid
void urlidx_path_by_docid( urlidx_t *u, docid_t docid, char *path );

// Check if a docid is a homepage
bool urlidx_is_homepage( urlidx_t *u, docid_t docid );

// Operations on paths

// Convert path to canonical path
void urlidx_canonicalize_path( char *src, char *dest );

// Check is url is dynamic
bool urlidx_is_dynamic( perfhash_t *extensions_dynamic, char *url );

// Parse absolute url
bool urlidx_parse_complete_url( char *url, char *protocol, char *sitename, char *path );

// Convert url to absolute
void urlidx_relative_path_to_absolute( char *src_path, char *url, char *path );

// Sanitize the URL and remove sessionids
void urlidx_remove_variable( char *url, const char *varname );
void urlidx_sanitize_url( char *url );
void urlidx_remove_sessionids_heuristic( char *url );

// Get the extension of a string
void urlidx_get_lowercase_extension( char *examined, char *extension );


// Hash functions
siteid_t urlidx_hashing_site( const char *text );
docid_t urlidx_hashing_path( const char *text, int size );

#endif
