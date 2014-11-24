
#ifndef _EXTRACT_H_INCLUDED_
#define _EXTRACT_H_INCLUDED_

#include <config.h>

// System libraries

#include <map>
#include <fstream>
#include <assert.h>
#include <getopt.h>

using namespace std;

// Local libraries

#include "const.h"
#include "xmlconf.h"
#include "xmlconf-main.h"
#include "utils.h"
#include "metaidx.h"
#include "urlidx.h"
#include "harvestidx.h"
#include "storage.h"
#include "linkidx.h"
#include "cleanup.h"
#include "sitelink.h"
#include "irudiko.h"

// Globals

metaidx_t	*metaidx	= NULL;
urlidx_t	*urlidx		= NULL;
linkidx_t	*linkidx	= NULL;
storage_t	*storage	= NULL;

// Functions

void extract_usage();
void extract_dump_document( docid_t docid, char *url, char *buf );

#endif
