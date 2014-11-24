
#ifndef _STRING_LIST_INCLUDED_
#define _STRING_LIST_INCLUDED_

#include <config.h>

// System libraries

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <xmlconf.h>

// Local libraries

#include "die.h"
#include "const.h"

// Constants

#define MAX_STRING_LIST_LENGTH	(254)

// Typedefs

typedef struct {
	char *str[MAX_STRING_LIST_LENGTH];
	uint len[MAX_STRING_LIST_LENGTH];
	uint count;
} string_list_t;

// Functions

string_list_t *string_list_create( char *words );
void string_list_free( string_list_t *list );
bool string_list_suffix( string_list_t *list, char *str );

#endif
