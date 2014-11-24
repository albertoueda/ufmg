
#ifndef _GATHERER_H_INCLUDED_
#define _GATHERER_H_INCLUDED_

#include <config.h>

// System libraries

#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <map>
#include <string>
#include <ctype.h>
#include <signal.h>

using namespace std;

// Local libraries

#include "const.h"
#include "xmlconf.h"
#include "storage.h"
#include "utils.h"
#include "harvestidx.h"
#include "metaidx.h"
#include "cleanup.h"
#include "linkidx.h"
#include "activepool.h"
#include "md5.h"
#include "irudiko.h"

// #include <gatherer/parser.h>
extern void parser_init();
extern off64_t parser_process( doc_t *doc, char *inbuf, char *outbuf );
extern void parser_save_extensions_stats( FILE *links_stat );

// Globals

storage_t *storage;
metaidx_t *metaidx;

FILE *links_download;
FILE *links_log;
FILE *links_stat;

// Functions

void gather_harvester(harvest_t *harvest, char *buf1, char *buf2);
void gather_fetcher(harvest_t *harvest, int fetcherid, 
		site_t *sites,
		map<siteid_t, uint> *map_sites,
		map<siteid_t, bool> *site_readed,
		char *inbuf, char *outbuf);
void gatherer_open_indexes();
void gatherer_usage();
void gatherer_show_legend();

#endif
