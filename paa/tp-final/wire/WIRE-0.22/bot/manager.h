
#ifndef _MANAGER_H_INCLUDED_
#define _MANAGER_H_INCLUDED_

#include <config.h>

// System libraries

#include <features.h>
#include <getopt.h>
#include <fstream>
#include <assert.h>
#include <string.h>
#include <signal.h>
#include <time.h>

// Local libraries

#include "const.h"
#include "xmlconf.h"
#include "xmlconf-main.h"
#include "harvestidx.h"
#include "metaidx.h"
#include "metaidx_analysis.h"
#include "cleanup.h"
#include "linkidx.h"
#include "sitelink.h"
#include "urlidx.h"
#include "utils.h"

// Types

enum manager_site_status_t {
	MANAGER_SITE_STATUS_AVAILABLE	= 0,
	MANAGER_SITE_STATUS_STORED,
	MANAGER_SITE_STATUS_CHECK_SPECIAL,
	MANAGER_SITE_STATUS_TOO_MANY_ERRORS,
	MANAGER_SITE_STATUS_ASSIGNED_TO_OTHER_HARVEST
};

// Globals

metaidx_t	*metaidx;
linkidx_t	*linkidx;
urlidx_t    *urlidx;

priority_t	*priority = NULL;
docid_t     *order = NULL;
siteid_t	*docid_to_siteid = NULL;

docid_t     ndocs;
siteid_t	nsites;

// Functions

void manager_create_harvest_list( harvest_t *harvest );
int manager_compare_by_priority( const void *a, const void *b );
void manager_open_indexes();
void manager_calculate_scores();
void manager_order_documents();
void manager_usage();

#endif
