
#ifndef _SEEDER_H_INCLUDED_
#define _SEEDER_H_INCLUDED_

#include <config.h>

// System libraries

#include <features.h>
#include <fstream>
#include <string>
#include <assert.h>
#include <getopt.h>
#include <map>

using namespace std;

// Local libraries

#include "const.h"
#include "xmlconf.h"
#include "xmlconf-main.h"
#include "perfect_hash.h"
#include "utils.h"
#include "urlidx.h"
#include "metaidx.h"
#include "cleanup.h"
#include "linkidx.h"
#include "harvestidx.h"
#include "string_list.h"

// Constants

const int SEEDER_PAGE_WIDTH = 50;

// Globals

string_list_t *domain_suffixes;
perfhash_t accept_protocol;
perfhash_t extensions_ignore;
perfhash_t extensions_dynamic;

regex_t reject_patterns;

regex_t sessionid_patterns;
char **sessionid_variables		= NULL;
int sessionid_variables_count	= 0;

metaidx_t	*metaidx = NULL;
urlidx_t	*urlidx  = NULL;
linkidx_t	*linkidx = NULL;

// Functions

void seeder_init_maps();
void seeder_show_legend();
void seeder_process_harvest( harvest_t *harvest );
docid_t seeder_resolve_link( harvest_t *harvest, doc_t *src, char *src_path, char *url, char *caption, char *path_buffer, site_t *site );
void seeder_save_links( doc_t *src, out_link_t adjacency_list[], int adjacency_list_length );
void seeder_open_indexes();
bool seeder_is_excluded( doc_t *doc, char *path );
void seeder_usage();

#endif
