
#ifndef _PARSER_H_INCLUDED_
#define _PARSER_H_INCLUDED_

#include <config.h>

// System libraries

#include <fstream>
#include <map>
#include <string>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

using namespace std;

// Local libraries

#include "perfect_hash.h"
#include "const.h"
#include "xmlconf.h"
#include "utils.h"
#include "metaidx.h"
#include "linkidx.h"
#include "die.h"
#include "int_stack.h"

// Global vars (from gatherer.c)

extern metaidx_t *metaidx;
extern FILE *links_download;
extern FILE *links_log;

// Constants

// Maximum number of parsed attributes in a html element
#define MAX_HTML_ATTRIBUTES 50

// Structures and types
 
// Enumeration for the html event (SAX-like) parser
enum event_t {
    EVENT_UNDEF,
    EVENT_START_TAG,
    EVENT_END_TAG,
    EVENT_EMPTY_TAG,
    EVENT_TEXT,
    EVENT_EOF
};

// Status for the html parser
enum status_t {
    STATUS_NORMAL,
    STATUS_IGNORE,
    STATUS_INANCHOR
};

// Type of link
enum extension_type_t {
	EXTENSION_NORMAL,
	EXTENSION_IGNORE,
	EXTENSION_LOG,
	EXTENSION_STAT
};
 
// Structure for a html attribute
typedef struct {
    char    name[MAX_STR_LEN];
    char    value[MAX_STR_LEN];
} attribute_t;
 
// Structure for a html tag with an attributes list
typedef struct {
    char    name[MAX_STR_LEN];
    attribute_t attributes[MAX_HTML_ATTRIBUTES];
    int attnum;
} tag_t;

// Functions

void parser_init();
off64_t parser_process(doc_t *doc, char *inbuf, char *outbuf);
void parser_init_link_attributes(char *str);

void parser_analyze_link(doc_t *doc, char *href, char *caption, double rel_pos, em_tag_t tag);
void parser_save_internal_link(doc_t *doc, char *path, char *caption);
void parser_save_absolute_link(doc_t *doc, char *uri, char *caption);

// Parsers

off64_t parser_process_html(doc_t *doc, char *inbuf, char *outbuf);
off64_t parser_process_robotstxt(doc_t *doc, char *inbuf, char *outbuf);
off64_t parser_process_robotsrdf(doc_t *doc, char *inbuf, char *outbuf);

// Tags
void update_tag_stack(int_stack_t *tag_stack, char *current_tag, event_t event);

// Extensions
extension_type_t parser_check_extension( char *href, char *extension );

// Save
void parser_save_extensions_stats( FILE *links_stat );

#endif
