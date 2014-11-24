
#ifndef _LANG_H_INCLUDED_
#define _LANG_H_INCLUDED_

#include <config.h>

// System libraries

#include <stdio.h>
#include <ctype.h>
#include <fstream>
#include <locale.h>

// Local libraries

#include "const.h"
#include "xmlconf.h"
#include "utils.h"
#include "storage.h"
#include "metaidx.h"
#include "perfect_hash.h"
#include "cleanup.h"

// Constants

#define LANG_MAX_COUNT  20

// Globals

extern storage_t *storage;
extern metaidx_t *metaidx;

// Functions

void lang_init();
int lang_identify_document( char *buffer, off64_t buffer_length,
		FILE *output, uint *word_count );
int lang_check_word( char *word );
bool lang_is_separator( char c );
void lang_analyze();

#endif
