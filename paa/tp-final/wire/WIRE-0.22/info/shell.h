
#ifndef _INFO_H_INCLUDED_
#define _INFO_H_INCLUDED_

#include <config.h>

// System libraries

#include <iostream>
#include <assert.h>
#include <string>

using namespace std;

// Local libraries

#include "const.h"
#include "xmlconf.h"
#include "xmlconf-main.h"
#include "utils.h"
#include "urlidx.h"
#include "metaidx.h"
#include "harvestidx.h"
#include "storage.h"
#include "cleanup.h"
#include "linkidx.h"
#include "sitelink.h"

// Globals

metaidx_t	 *metaidx;
urlidx_t	 *urlidx;
storage_t    *storage;
linkidx_t    *linkidx;

// Functions

void shell_help();
siteid_t shell_get_siteid( char *sitename );
void shell_parser( char *cmdline );

#endif
