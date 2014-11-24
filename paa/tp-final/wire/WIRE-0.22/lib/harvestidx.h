#ifndef _HARVESTIDX_INCLUDED_
#define _HARVESTIDX_INCLUDED_

#include <config.h>

// System libraries

#include <errno.h>
#include <map>
#include <string>
#include <queue>
#include <sys/types.h>
#include <dirent.h>

// Local libraries

#include "die.h"
#include "const.h"
#include "xmlconf.h"
#include "metaidx.h"

// Filenames

#define HARVESTIDX_FILENAME_MAIN	 "%s/%d_harvestidx.harvest_t"

#define HARVESTIDX_FILENAME_DOC		 "%s/%d_harvestidx.doc_t"
#define HARVESTIDX_FILENAME_SITE	 "%s/%d_harvestidx.site_t"
#define HARVESTIDX_FILENAME_PATH     "%s/%d_harvestidx.path.txt"
#define HARVESTIDX_FILENAME_SITENAME "%s/%d_harvestidx.sitename.txt"

// Possible status of a harvest

enum harvest_status_t {
	STATUS_HARVEST_EMPTY	= 0,
	STATUS_HARVEST_ASSIGNED = 1,
	STATUS_HARVEST_RUNNING	= 2,
	STATUS_HARVEST_ABORTED	= 3,
	STATUS_HARVEST_DONE	= 4,
	STATUS_HARVEST_GATHERED	= 5,
	STATUS_HARVEST_SEEDED   = 6
};
#define HARVEST_STATUS_STR(x) (\
		x==0 ? "STATUS_HARVEST_EMPTY" :\
		x==1 ? "STATUS_HARVEST_ASSIGNED" :\
		x==2 ? "STATUS_HARVEST_RUNNING" :\
		x==3 ? "STATUS_HARVEST_ABORTED" :\
		x==4 ? "STATUS_HARVEST_DONE" :\
		x==5 ? "STATUS_HARVEST_GATHERED" :\
		x==6 ? "STATUS_HARVEST_SEEDED" : "(invalid)" )


// Structure for a harvest

typedef struct {
	// Directory
	char dirname[MAX_STR_LEN];

	// Global id
	int	id;

	// Number and size
	uint count;
	uint count_ok;
	uint count_site;
	off64_t raw_size;               // Number of bytes downloaded
	off64_t size;                   // Number of bytes kept after parsing

	// Links discovered
	uint link_total;
	uint link_old_pages;
	uint link_new_pages;
	uint link_new_sites;

	// Host in which the harvest was run
	char hostname[MAX_STR_LEN];

	// Times
	time_t creationtime;            
	time_t begintime;
	time_t endtime;

	// Status for the harvest
	harvest_status_t status;

	// Volatile data
	// Readonly flag
	bool readonly;
	doc_t *doc_list;
	site_t *site_list;
	
	// Files
	FILE *file_doc;
	FILE *file_site;
	FILE *file_path;
	FILE *file_sitename;

	// Doc_map
	map <docid_t,string> *map_path;
	map <siteid_t,string> *map_sitename;
	map <siteid_t,site_t> *map_site;

} harvest_old_t;

// Structure for a harvest

typedef struct {
	// Directory
	char dirname[MAX_STR_LEN];

	// Global id
	int	id;

	// Number and size
	uint count;
	uint count_ok;
	uint count_site;
	off64_t raw_size;
	off64_t size;

	// Links discovered
	uint link_total;
	uint link_old_pages;
	uint link_new_pages;
	uint link_new_sites;

	// Host in which the harvest was run
	char hostname[MAX_STR_LEN];

	// Times
	time_t creationtime;            
	time_t begintime;
	time_t endtime;

	// Status for the harvest
	harvest_status_t status;

	// Statistics
	double	speed_ok;		// docs/sec.
	double	links_ratio;	// newlinks/total
	off64_t	bytes_in;		// total bytes read
	off64_t	bytes_out;		// total bytes written

	double	sum_pagerank;
	double	sum_wlrank;
	double	sum_hubrank;
	double	sum_authrank;

	docid_t	sum_in_degree;
	uint	sum_depth;
	docid_t	sum_ok;

	// Reserved
	char reserved[500];

	// Volatile data
	// Readonly flag
	bool readonly;
	doc_t *doc_list;
	site_t *site_list;
	
	// Files
	FILE *file_doc;
	FILE *file_site;
	FILE *file_path;
	FILE *file_sitename;

	// Doc_map
	map <docid_t,string> *map_path;
	map <siteid_t,string> *map_sitename;
	map <siteid_t,site_t> *map_site;

} harvest_t;

// File functions
harvest_t *harvest_open( const char *dirname, int harvestid, bool readonly );
harvest_t *harvest_create( const char *dirname );
bool harvest_exists( const char *dirname, int harvestid );
void harvest_save_info( harvest_t *harvest );
void harvest_close( harvest_t *harvest );
void harvest_remove( const char *dirname, int harvestid );
void harvest_remove_files( const char *dirname, int harvestid );
void harvest_remove_all( const char *dirname );

// List manipulation

void harvest_append_doc( harvest_t *harvest, doc_t *doc, char *path );
void harvest_append_site( harvest_t *harvest, site_t *site, char *sitename );
void harvest_read_list_doc_only( harvest_t *harvest );
void harvest_read_list( harvest_t *harvest );

// Analyze
void harvest_analyze( metaidx_t *metaidx );

// Show data
void harvest_dump_harvests( FILE *out );
void harvest_dump_data_header( FILE *out );
void harvest_dump_data( harvest_t *harvest, FILE *out );
void harvest_dump_status( harvest_t *harvest );
void harvest_dump_list( harvest_t *harvest );

// Other
int harvest_compare_by_siteid( const void *a, const void *b );
void harvest_default( harvest_t *harvest );


#endif
