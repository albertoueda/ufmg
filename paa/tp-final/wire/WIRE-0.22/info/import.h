
#ifndef _IMPORT_H_INCLUDED_
#define _IMPORT_H_INCLUDED_

#include <config.h>

// System libraries

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
#include "linkidx.h"
#include "cleanup.h"

// Globals

metaidx_t	*metaidx	= NULL;
urlidx_t	*urlidx		= NULL;
linkidx_t	*linkidx	= NULL;

// Functions

void import_usage();

#endif
