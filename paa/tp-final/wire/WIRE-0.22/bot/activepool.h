

#ifndef _ACTIVEPOOL_H_INCLUDED_
#define _ACTIVEPOOL_H_INCLUDED_

#include <config.h>

// System libraries

#include <assert.h>
#include <adns.h>

// Local libraries

#include "xmlconf.h"
#include "die.h"
#include "server.h"
#include "waitq.h"
#include "pollvec.h"
#include <perfect_hash.h>

// External

extern server_t *servers;

// Constants

#define ACTIVEPOOL_FILENAME_LOG		"activepool.log"
#define ACTIVEPOOL_FILENAME_CONTENT	"%04d_content.raw"
#define ACTIVEPOOL_FILENAME_DOCS	"%04d_documents.doc_t"
#define ACTIVEPOOL_FILENAME_SITES	"sites.site_t"


// Structs

// This is the main active pool, it consists
// of two lists of servers: poll and nopoll.
// - poll contains servers that will be polled
// - nopoll contains server entering or leaving the active pool
//
typedef struct {
	uint			maxactive;			// Maximum number of slots
										// nactive_poll + nactive_nopoll

	uint			nactive_poll;		// Num servers for polling
	uint			*serverids_poll;	// Servers for polling
	bool			*poll_slot_free;	// Free slots

	uint			nactive_nopoll;		// Num server w/o polling
	uint			*serverids_nopoll;	// Servers w/o polling

	pollevent_t		*pollevents;		// For receiving poll events
	pollvec_t		*pollvec;			// Polling vector

	adns_state		*adns_states;		// For holding adns states
	adns_query		*adns_queries;		// For holding adns queries

	uint err_dns;
	uint err_connect;
	uint err_other;
	uint ok;

	char			*useragent;			// Identification for crawler
	perfhash_t      *blocked_ip;        // Blocked IP addresses
	char			*dirname;			// Directory
	FILE			*file_log;			// Logfile

	// Output files
	FILE			**content_files;
	FILE			**doc_files;
} activepool_t;

// Functions

// Create and destroy
activepool_t *activepool_create( uint maxactive, pollvec_t *pollvec, char *useragent, perfhash_t *blocked_ip, char *dirname );
void activepool_destroy( activepool_t *activepool );
void activepool_save_sites( activepool_t *activepool, uint total_sites );

// Insert and remove servers
void activepool_insert_poll( activepool_t *activepool, uint serverid, short events, uint timeout_seconds );
void activepool_remove_poll( activepool_t *activepool, uint serverid );
void activepool_insert_nopoll( activepool_t *activepool, uint serverid );
void activepool_remove_nopoll( activepool_t *activepool, uint serverid );

// Process, for each tick
void activepool_process( activepool_t *activepool, waitq_t *waitq );
void activepool_process_poll( activepool_t *activepool, waitq_t *waitq, pollevent_t *pollevent );
bool activepool_process_nopoll( activepool_t *activepool, waitq_t *waitq, uint slot, uint serverid );

// Fill
void activepool_fill( activepool_t *activepool, waitq_t *waitq );

// Query 
int activepool_has_free_slots( activepool_t *activepool );
int activepool_empty( activepool_t *activepool );
void activepool_dump( activepool_t *activepool );
void activepool_minidump( activepool_t *activepool, waitq_t *waitq );

#endif
