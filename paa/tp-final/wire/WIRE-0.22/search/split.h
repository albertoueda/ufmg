
#ifndef _SPLIT_H_INCLUDED_
#define _SPLIT_H_INCLUDED_

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
#include "storage.h"
#include "cleanup.h"

// Globals

#define SPLIT_PAGE_LEN 60

metaidx_t	 *metaidx;
storage_t    *storage;

// Functions

void split_usage();

#endif
