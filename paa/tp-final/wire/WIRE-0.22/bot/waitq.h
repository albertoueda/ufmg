
#ifndef _WAITQ_H_INCLUDED_
#define _WAITQ_H_INCLUDED_

#include <config.h>

// System libraries

#include <assert.h>
#include <stdlib.h>

// Local libraries

#include "die.h"
#include "xmlconf.h"

// Structs

typedef struct waitq_item_ptr {
	uint serverid;
	time_t nextvisit;
} waitq_item_t;

typedef struct {
	waitq_item_t *queue;
	uint nservers;
	uint maxservers; 
} waitq_t;

// Functions

waitq_t *waitq_create( uint maxservers );
void waitq_swap( waitq_t *waitq, uint a, uint b );
void waitq_push( waitq_t *waitq, uint serverid, time_t nextvisit );
void waitq_pop( waitq_t *waitq, uint *serverid, time_t *nextvisit );
int waitq_empty( waitq_t *waitq );
bool waitq_available( waitq_t *waitq, time_t now );
void waitq_destroy( waitq_t *waitq );
void waitq_dump( waitq_t *waitq );

#endif
