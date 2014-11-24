
#ifndef _ANALYSIS_H_INCLUDED_
#define _ANALYSIS_H_INCLUDED_

#include <config.h>

// System libraries

#include <features.h>
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
#include "metaidx_analysis.h"
#include "harvestidx.h"
#include "storage.h"
#include "linkidx.h"
#include "urlidx.h"
#include "cleanup.h"
#include "sitelink.h"
#include "lang.h"

// Globals

metaidx_t	*metaidx	= NULL;
linkidx_t	*linkidx	= NULL;
urlidx_t	*urlidx		= NULL;
storage_t	*storage	= NULL;

// Functions

void analysis_usage();

#endif
