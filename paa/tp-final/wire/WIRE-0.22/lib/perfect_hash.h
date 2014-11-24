
#ifndef _PERFHASH_INCLUDED_
#define _PERFHASH_INCLUDED_

#include <config.h>

// System libraries

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

// Local libraries

#include "xmlconf.h"
#include "die.h"
#include "cleanup.h"

// Constants

#define PERFHASH_MAX_WORD_LEN	254
#define PERFHASH_MAX_WORDS		1000
#define PERFHASH_MAX_RETRIES	100

/* The probability of false positives will be 1/255 * 1/(1-SECURITY) */
#define PERFHASH_SECURITY_FACTOR		0.99

// Typedefs

typedef struct {
	int *table;
	char *keywords[PERFHASH_MAX_WORDS];
	uint length;
	uint count;
	uint A;
	uint B;
	bool   check_matches;
} perfhash_t;

// Functions

void perfhash_create( perfhash_t *perfhash, char *keywords );
bool perfhash_check( perfhash_t *perfhash, char *keyword );
void perfhash_destroy( perfhash_t *perfhash );
void perfhash_dump( perfhash_t *perfhash );

void _perfhash_clear_table( perfhash_t *perfhash );
void _perfhash_hash( perfhash_t *perfhash, char *word, int *hash, int *vrfy );
bool _perfhash_try_parameters( perfhash_t *perfhash, char *last_collision );

#endif
