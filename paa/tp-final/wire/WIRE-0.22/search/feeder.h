#ifndef _FEEDER_H_INCLUDED_
#define _FEEDER_H_INCLUDED_

#include <config.h>

// System libraries

#include <features.h>
#include <getopt.h>
#include <iostream>
#include <assert.h>
#include <string>

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

// Types

// Data formats 
enum feeder_format_t {
	FEEDER_FORMAT_UNDEFINED = 0,
	FEEDER_FORMAT_SWISHE=1,
	FEEDER_FORMAT_LUCENE=2
};

#define FEEDER_PROGRAM_NAME	"wire-search-feeder"

// Globals

typedef long score_t;

metaidx_t	 *metaidx;
urlidx_t	 *urlidx;
storage_t    *storage;
score_t      *pagerank = NULL;
score_t      *wlrank = NULL;
score_t      *cscore = NULL;
docid_t      *order = NULL;

// Functions

docid_t calculate_scores();
int feeder_compare_by_score( const void *a, const void *b );
void feed_idx(docid_t docid, docid_t score);
int feed_program(docid_t docid, char *buffer, score_t pagerank, score_t wlrank, score_t cscore, feeder_format_t format);
int feed_swish(docid_t docid, char *buffer, score_t pagerank, score_t wlrank, score_t cscore);
int feed_lucene(docid_t docid, char *buffer, score_t pagerank, score_t wlrank, score_t cscore);
void feeder_usage();

#endif
