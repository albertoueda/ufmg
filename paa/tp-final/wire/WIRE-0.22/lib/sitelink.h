
#ifndef _SITELINK_H_INCLUDED_
#define _SITELINK_H_INCLUDED_

#include <config.h>

// System libraries

#include <iostream>
#include <assert.h>
#include <getopt.h>
#include <map>

using namespace std;

// Local libraries

#include "const.h"
#include "xmlconf.h"
#include "utils.h"
#include "metaidx.h"
#include "cleanup.h"
#include "linkidx.h"
#include "urlidx.h"
#include "storage.h"

// Constants

#define SITELINK_MAX_OUTDEGREE	(10000)
#define SITELINK_MAX_ITERATIONS (500)
#define SITELINK_FILENAME	"sitelinkidx.sitelinkidx_t"

// Typedefs

typedef struct _sitelink_t {
	siteid_t	siteid;
	docid_t		weight;
	_sitelink_t	*next;
} sitelink_t;

typedef struct {
	siteid_t	siteid;
	docid_t		weight;
} saved_sitelink_t;

typedef struct {
	char		dirname[MAX_STR_LEN];
	siteid_t	count_site;
	storage_t	*storage;
	sitelink_t	**links;
} sitelinkidx_t;

// Globals

extern metaidx_t	 *metaidx;
extern linkidx_t    *linkidx;

// Functions

// Load and save
void sitelink_save( sitelinkidx_t *sitelinkidx );
sitelinkidx_t *sitelink_load( const char *dirname );

// Generate the sitelink index
void sitelink_create( const char *dirname, linkidx_t *linkidx, metaidx_t *metaidx );

// Analyze
void sitelink_analysis_components( sitelinkidx_t *sitelinkidx, metaidx_t *metaidx );
void sitelink_analysis_siterank( sitelinkidx_t *sitelinkidx, metaidx_t *metaidx,
		bool use_internal_links, bool use_site_size, bool linearize );

// Dump
void sitelink_dump_links( sitelinkidx_t *sitelink_t, siteid_t siteid );

void sitelink_dump_links_with_sitename( sitelinkidx_t *sitelinks, urlidx_t *urlidx, siteid_t siteid );
void sitelink_dump_structure( sitelinkidx_t *sitelinkidx );

#endif
