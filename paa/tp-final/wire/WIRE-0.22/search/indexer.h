
#ifndef _INDEXER_H_INCLUDED_
#define _INDEXER_H_INCLUDED_

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
#include "cleanup.h"

// Constants

#define SWISHE_FILE_CONFIG	"swish-e.conf"
#define SWISHE_INDEX_FILE	"index.swish-e"
#define SWISHE_PROGRAM		"swish-e"

#define LUCENE_FILE_CONFIG	"WireIndexer.conf"
#define LUCENE_PROGRAM		"java" //-classpath lucene/:lucene/lucene-1.4.3.jar:lucene/lucene-demos-1.4.3.jar HTMLIndexer 

// Functions
void write_lucene_conf(FILE * file_conf, char * generator, char * full_feeder_path, docid_t opt_from, docid_t opt_to);
void write_swishe_conf(FILE * file_conf, char * generator, char * full_feeder_path, docid_t opt_from, docid_t opt_to);
void indexer_usage();

#endif
