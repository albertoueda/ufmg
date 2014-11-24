
#include "pollvec.h"

//
// Name: pollvec_create
//
// Description:
//   Creates a polling vector structure
//
// Input:
//   max_nfds - maximum number of sockets being polled
//   file_log - logfile, or NULL for NO LOG
//
// Return:
//   the created object
//

pollvec_t *pollvec_create(uint max_nfds, FILE *file_log) {
	assert( max_nfds > 0 );

	pollvec_t *pollvec = (pollvec_t *)malloc(sizeof(pollvec_t));
	assert( pollvec != NULL );

	// Log
	pollvec->file_log = file_log;
	if( pollvec->file_log != NULL ) {
		fprintf( pollvec->file_log, "Allocating ... " );
	}


	// Current and maximum number of sockets
	pollvec->nfds = 0;
	pollvec->max_nfds = max_nfds;


	// Sockets being polled
	pollvec->fds = (struct pollfd *)malloc(sizeof(struct pollfd)*max_nfds);
	assert( pollvec->fds != NULL );

	// Servers in the sockets
	pollvec->serverids = (uint *)malloc(sizeof(uint)*max_nfds);
	assert( pollvec->serverids != NULL );

	// Timeouts
	pollvec->timeouts = (time_t *)malloc(sizeof(time_t)*max_nfds);
	assert( pollvec->timeouts != NULL );

	// Clear the structures
	time_t now = time(NULL);
	for( uint i=0; i<max_nfds; i++ ) {
		pollvec->fds[i].fd		= -1;
		pollvec->fds[i].events	= 0;
		pollvec->serverids[i]	= 0;
		pollvec->timeouts[i]	= now;
	}

	if( pollvec->file_log != NULL ) {
		fprintf( pollvec->file_log, "done.\n" );
	}
	return pollvec;
}

//
// Name: pollvec_destroy
//
// Description:
//   Destroys a polling vector structure
//
// Input:
//   pollvec - the structure
//

void pollvec_destroy( pollvec_t *pollvec ) {
	assert( pollvec != NULL );
	assert( pollvec->nfds == 0 );

	free( pollvec->fds );
	free( pollvec->serverids );
	free( pollvec->timeouts );

	if( pollvec->file_log != NULL ) {
		fprintf( pollvec->file_log, "Destroying.\n" );
		fclose( pollvec->file_log );
	}
	free( pollvec );
}

//
// Name: pollvec_insert
//
// Description:
//   Inserts a filedescriptor for polling
//
// Input:
//   server - the associated server
//   events - the events that will be polled
//   timeout - the timeout in seconds
//

void pollvec_insert(pollvec_t *pollvec, server_t *server, short events, uint timeout_seconds ) {
	assert( pollvec != NULL );
	assert( pollvec->nfds < pollvec->max_nfds );
	assert( server != NULL );
	assert( events > 0 );
	assert( (timeout_seconds * 1000) > CONF_HARVESTER_TIMEOUT_POLL );

	assert( server->conn_status == CONN_RESOLVING
		 || server->conn_status == CONN_CONNECTING );

	assert( server->socket >= 0 );
	assert( server->pool_slot >= 0 );


	// Get a new slot
	uint slot = pollvec->nfds++;

	// Copy information
	pollvec->fds[slot].fd		= server->socket;
	pollvec->fds[slot].events	= events;
	pollvec->serverids[slot]	= server->serverid;
	pollvec->timeouts[slot]		= time(NULL) + timeout_seconds;

	// Report
	if( pollvec->file_log != NULL ) {
		fprintf( pollvec->file_log, "Server %s inserted in slot %d of %d (%d max)\n", server->hostname, slot, pollvec->nfds, pollvec->max_nfds );
	}
}

//
// Name: pollvec_remove
//
// Description:
//   Removes a file descriptor from the polling
//
// Input:
//   server - the file descriptor to be removed
//

void pollvec_remove( pollvec_t *pollvec, server_t *server ) {
	assert( pollvec != NULL );
	assert( pollvec->nfds > 0 );
	assert( server->socket == -1 );
	assert( server->serverid > 0 );

	// Go through all slots
	for( uint slot=0; slot<pollvec->nfds; slot++ ) {

		// We cannot search for the fd, because it can be
		// closed now
		if( pollvec->serverids[slot] == server->serverid ) {

			uint last_slot = pollvec->nfds - 1;

			// Reduce the number of nfds
			(pollvec->nfds)--;

			// Check if this is the last slot
			if( last_slot == slot ) {
				// This is the last slot

			} else {
				// This is not the last slot, copy the last slot here
				pollvec->fds[slot].fd		= pollvec->fds[last_slot].fd;
				pollvec->fds[slot].events	= pollvec->fds[last_slot].events;
				pollvec->serverids[slot]	= pollvec->serverids[last_slot];
				pollvec->timeouts[slot]		= pollvec->timeouts[last_slot];
			}

			if( pollvec->file_log != NULL ) {
				fprintf( pollvec->file_log, "Server %s removed from slot %d, now there are %d free\n", server->hostname, slot, pollvec->nfds );
			}
			return;
		}
	}
	if( pollvec->file_log != NULL ) {
		fprintf( pollvec->file_log, "Attempt to remove %s, it is not (there are %d of %d slots used)\n", server->hostname, pollvec->nfds, pollvec->max_nfds );
	}
	
	die( "Removed server->serverid does not exist" );
}

//
// Name: pollvec_update
//
// Description:
//   Removes a file descriptor from the polling
//
// Input:
//   pollvec - the structure
//   server - the file descriptor to be removed
//   event - the new events
//   new_timeout - in seconds, now + time

void pollvec_update( pollvec_t *pollvec, server_t *server, short events, uint timeout_seconds ) {
	assert( pollvec != NULL );
	assert( pollvec->nfds > 0 );
	assert( server->socket >= 0 );
	assert( server->serverid > 0 );

	// Go through all slots
	for( uint slot=0; slot<pollvec->nfds; slot++ ) {

		// We cannot search for the fd, because it can be
		// closed now. We search for the serverid
		if( pollvec->serverids[slot] == server->serverid ) {

			// Update
			assert( timeout_seconds > (uint)time(NULL) );

			pollvec->fds[slot].events	= events;
			pollvec->timeouts[slot]		= timeout_seconds;

			if( pollvec->file_log != NULL ) {
				fprintf( pollvec->file_log, "Server %s updated in slot %d, now events=%d, timeout=%d\n", server->hostname, slot, events, timeout_seconds );
			}
			return;
		}
	}
	if( pollvec->file_log != NULL ) {
		fprintf( pollvec->file_log, "Attempt to update %s, it is not (there are %d of %d slots used)\n", server->hostname, pollvec->nfds, pollvec->max_nfds );
	}
	
	die( "Updated server->serverid does not exist" );
}

//
// Name: pollvec_poll
//
// Description:
//   Calls poll()
//
// Input:
//   pollvec - the structure
//
// Output:
//   pollevents - the received events
//
// Return
//   the number of servers with events
//

uint pollvec_poll( pollvec_t *pollvec, pollevent_t *pollevents ) {
	assert( pollvec != NULL );
	assert( pollevents != NULL );
	if( pollvec->nfds == 0 ) {
		return 0;
	}

	// Check how many events are waiting
	int nevents_poll = poll( pollvec->fds, pollvec->nfds, CONF_HARVESTER_TIMEOUT_POLL );
	if( nevents_poll < 0 ) {
		perror( "Poll error" );
		die( "Error while polling" );
	}

	assert( (nevents_poll == 0) || ((uint)nevents_poll <= pollvec->nfds) );
	if( pollvec->file_log != NULL ) {
		fprintf( pollvec->file_log, "Poll() returned %d events in %d sockets\n", nevents_poll, pollvec->nfds );
	}

	// Number of events checked
	uint nevents	= 0;
	time_t now		= time(NULL);

	for( uint slot=0; slot<pollvec->nfds; slot++ ) {
		short revents		= pollvec->fds[slot].revents;
		uint serverid		= pollvec->serverids[slot];

		if( revents != 0 ) {

			// Received a poll event
			pollevents[nevents].serverid	= serverid;
			pollevents[nevents].revents		= revents;

			if( pollvec->file_log != NULL ) {
				fprintf( pollvec->file_log, "-> %s received event %d\n", servers[serverid].hostname, revents );
			}

			// Event ok
			nevents++;

		} else {

			// Check timeout
			if( now > pollvec->timeouts[slot] ) {

				// Timedout
				pollevents[nevents].serverid	= serverid;
				pollevents[nevents].revents		= 0;

				if( pollvec->file_log != NULL ) {
					fprintf( pollvec->file_log, "-> %s has a timeout\n", servers[serverid].hostname );
				}

				nevents++;

			}
		}
	}

	assert( nevents >= (uint)nevents_poll );
	assert( nevents <= pollvec->nfds );
	return nevents;
}

//
// Name: pollvec_empty
//
// Returns:
//   1 iff the polling vector is empty
//

int pollvec_empty( pollvec_t *pollvec ) {
	assert( pollvec != NULL );
	assert( pollvec->nfds < pollvec->max_nfds );

	return ( pollvec->nfds == 0 ) ? 1 : 0;
}
