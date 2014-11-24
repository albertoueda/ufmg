
#ifndef _POLLVEC_H_INCLUDED_
#define _POLLVEC_H_INCLUDED_

#include <config.h>

// System libraries

#include <assert.h>
#include <sys/poll.h>
#include <stdlib.h>

// Local libraries

#include "xmlconf.h"

// Constants
#define POLLVEC_FILENAME_LOG "pollvec.log"

// Structs

// The polling vector
typedef struct {
	struct pollfd *fds;	// Poll structures {fd,events,revents}
	uint nfds;			// Current number of fds
	uint max_nfds;		// Max fds supported
	uint *serverids;	// Serverids
	time_t *timeouts;	// Timeouts
	FILE *file_log;		// Logfile
} pollvec_t;

// An event received after poll()
typedef struct {
	uint serverid;		// Server that received the event
	int revents;		// Received events
} pollevent_t;

// Local libraries

#include "die.h"
#include "server.h"

extern server_t *servers;

// Functions

// Constructor and destructor
pollvec_t *pollvec_create(uint max_nfds, FILE *file_log);
void pollvec_destroy( pollvec_t *pollvec );

// Insert and remove servers from the pollvec
void pollvec_insert( pollvec_t *pollvec, server_t *server, short events, uint timeout_seconds );
void pollvec_remove( pollvec_t *pollvec, server_t *server );
void pollvec_update( pollvec_t *pollvec, server_t *server, short events, uint timeout_seconds );


// Poll
uint pollvec_poll( pollvec_t *pollvec, pollevent_t *pollevents );

int pollvec_empty( pollvec_t *pollvec );

#endif
