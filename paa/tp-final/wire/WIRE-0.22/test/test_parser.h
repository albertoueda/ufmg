
#ifndef _TEST_PARSER_H_INCLUDED_
#define _TEST_PARSER_H_INCLUDED_

#include <config.h>

// System libraries

#include <fstream>
#include <getopt.h>

// Local libraries

#include "const.h"
#include "md5.h"
#include "utils.h"
#include "xmlconf.h"
#include "metaidx.h"
#include "int_stack.h"
#include "parser.h"

// Constants

#define FILENAME_TEST_LINKS_DOWNLOAD	"/tmp/links_download.txt"
#define FILENAME_TEST_LINKS_LOG			"/tmp/links_log.txt"

// Internal

FILE *links_download;
FILE *links_log;

// Functions

void test_parser_usage();

#endif
