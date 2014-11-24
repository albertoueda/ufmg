
#include "activepool.h"

//
// Name: activepool_create
//
// Description:
//   Creates an active pool
//
// Input:
//    maxactive - the maximum number of servers in the pool
//    pollvec - the polling vector
//    useragent - to identify with servers
//    dirname - the directory for logfile & output
//
// Return:
//    the created structure
//

activepool_t *activepool_create( uint maxactive, pollvec_t *pollvec, char *useragent, perfhash_t *blocked_ip, char *dirname ) {
	assert( maxactive > 0 );

	// Create the pool
	activepool_t *activepool = (activepool_t *)malloc(sizeof(activepool_t));
	assert( activepool != NULL );

	// Dirname
	assert( dirname != NULL );
	assert( strlen( dirname ) > 0 );
	activepool->dirname			= dirname;

	// Logfile
	char filename[MAX_STR_LEN];
	sprintf( filename, "%s", ACTIVEPOOL_FILENAME_LOG );

	if( CONF_HARVESTER_LOGLEVEL == LOGLEVEL_VERBOSE ) {
		activepool->file_log		= fopen64( filename, "w" );
		assert( activepool->file_log != NULL );
		setbuf( activepool->file_log, NULL );
	} else {
		activepool->file_log		= NULL;
	}

	if( activepool->file_log != NULL ) {
		fprintf( activepool->file_log, "Allocating ... " );
	}

	// Useragent
	assert( useragent != NULL );
	assert( strlen(useragent) > 0 );
	activepool->useragent		= useragent;

	// Blocked IPs
	activepool->blocked_ip		= blocked_ip;

	activepool->serverids_poll	= (uint *)malloc(sizeof(uint)*maxactive);
	activepool->poll_slot_free	= (bool *)malloc(sizeof(bool)*maxactive);
	activepool->serverids_nopoll= (uint *)malloc(sizeof(uint)*maxactive);
	activepool->adns_states		= (adns_state *)malloc(sizeof(adns_state)*maxactive );
	activepool->adns_queries	= (adns_query *)malloc(sizeof(adns_query)*maxactive );
	activepool->pollevents		= (pollevent_t *)malloc(sizeof(pollevent_t)*maxactive );

	// Mark all pool slots as free
	for( uint i=0; i<maxactive; i++ ) {
		activepool->poll_slot_free[i] = true;
	}

	// Set max number of servers
	activepool->maxactive		= maxactive;
	activepool->nactive_poll	= 0;
	activepool->nactive_nopoll	= 0;

	activepool->err_dns			= 0;
	activepool->err_connect		= 0;
	activepool->err_other		= 0;
	activepool->ok				= 0;

	// Pollvec
	assert( pollvec != NULL );
	assert( pollvec_empty( pollvec ) );
	if( activepool->file_log != NULL ) {
		fprintf( activepool->file_log, " pollvec has %d slots, activepool has %d ", pollvec->max_nfds, activepool->maxactive ); 
	}
	assert( pollvec->max_nfds >= activepool->maxactive );
	activepool->pollvec			= pollvec;

	// Open output files
	// Content: the raw content of the fetched documents
	// Doc: doc_t structures for the pages fetched
	// An extra file for storing documents where the transfer failed is
	// created, and a dummy content file also

	activepool->content_files	= (FILE **)malloc(sizeof(FILE *)*(activepool->maxactive+1));
	activepool->doc_files		= (FILE **)malloc(sizeof(FILE *)*(activepool->maxactive+1));

	// String holding the filenames
	char output_filename[MAX_STR_LEN];

	// Open the content and doc files. There is one extra
	// doc file for the failed documents; and a corresponding dummy
	// content file.
	for( uint fileno=0; fileno<=activepool->maxactive; fileno++ ) {

		// Content files
		sprintf( output_filename, ACTIVEPOOL_FILENAME_CONTENT, fileno );
		sprintf( filename, "%s", output_filename );
		(activepool->content_files)[fileno] = fopen64( filename, "w" );
		if( (activepool->content_files)[fileno] == NULL ) {
			perror( filename );
			cerr << "Failed to open content file #" << fileno << endl;
			die( "Error while opening output files. Too few descriptors available for this process ?" );
		}

		// Doc files
		sprintf( output_filename, ACTIVEPOOL_FILENAME_DOCS, fileno );
		sprintf( filename, "%s", output_filename );
		(activepool->doc_files)[fileno] = fopen64( filename, "w" );
		if( (activepool->content_files)[fileno] == NULL ) {
			cerr << "Failed to open doc file #" << fileno << endl;
			die( "Error while opening output files. Too few descriptors available for this process ?" );
		}
		
		// TODO: set buffers on content files
	}

	// Return the pool
	if( activepool->file_log != NULL ) {
		fprintf( activepool->file_log, "done.\n" );
	}
	return activepool;
}

//
// Name: activepool_insert_nopoll
//
// Description:
//   Inserts an element in the pool
//
// Input:
//   activepool - the structure
//   serverid - the server to insert
// 

void activepool_insert_nopoll( activepool_t *activepool, uint serverid ) {
	assert( activepool != NULL );
	assert( activepool->nactive_poll + activepool->nactive_nopoll < activepool->maxactive );
	assert( servers[serverid].conn_status == CONN_ACTIVATING
		 || servers[serverid].conn_status == CONN_DEACTIVATING );
	assert( servers[serverid].socket == -1 );

	// Get a free slot
	uint slot = activepool->nactive_nopoll;
	activepool->nactive_nopoll++;

	// Insert
	activepool->serverids_nopoll[slot] = serverid;

	if( activepool->file_log != NULL ) {
		fprintf( activepool->file_log, "Inserted %s in active nopoll, slot %d.\n", servers[serverid].hostname, slot );
	}
}

//
// Name: activepool_insert_poll
//
// Description:
//   Inserts an element in the pool, polling
//
// Input:
//   activepool - the structure
//   serverid - the server to insert
//   events - the events we are interested in
//   timeout - the timeout, in seconds
// 

void activepool_insert_poll( activepool_t *activepool, uint serverid, short events, uint timeout_seconds ) {
	assert( activepool != NULL );
	assert( activepool->nactive_poll + activepool->nactive_nopoll < activepool->maxactive );

	conn_status_t conn_status = servers[serverid].conn_status;

	assert( conn_status == CONN_RESOLVING
		||	conn_status == CONN_CONNECTING
		||	conn_status == CONN_REQUESTING
		||	conn_status == CONN_RECEIVING );
	assert( servers[serverid].socket >= 0 );

	// Locate a free slot
	uint slot = 0;
	while( ! activepool->poll_slot_free[slot] ) {
		slot++;
		assert( slot < activepool->maxactive );
	}

	// Mark the slot as not free, and assign it to the serverid
	activepool->serverids_poll[slot]	= serverid;
	activepool->poll_slot_free[slot]	= false;
	servers[serverid].pool_slot	 = slot;

	// Increase the number of used slots
	activepool->nactive_poll++;

	// Insert into pollvec
	pollvec_insert( activepool->pollvec, &(servers[serverid]), events, timeout_seconds );
	assert( activepool->nactive_poll == activepool->pollvec->nfds );

	// Report
	if( activepool->file_log != NULL ) {
		fprintf( activepool->file_log, "Inserted %s in active poll, slot %d.\n", servers[serverid].hostname, slot );
	}
}

//
// Name: activepool_remove_poll
//
// Description:
//   Removes an element from the pool: the element must
//   be in the CONN_ACTIVATING or CONN_DEACTIVATING state
//
// Input:
//   activepool - the structure
//   serverid - the server to remove
//

void activepool_remove_poll( activepool_t *activepool, uint serverid ) {
	assert( activepool != NULL );
	assert( activepool->nactive_poll > 0 );

	conn_status_t conn_status = servers[serverid].conn_status;
	assert( conn_status == CONN_DEACTIVATING
		||	conn_status	== CONN_ACTIVATING );
	assert( servers[serverid].socket == -1 );

	// Remove from pollvec first
	pollvec_remove( activepool->pollvec, &(servers[serverid]) );

	// Search in the poll
	for( uint slot=0; slot<activepool->maxactive; slot++ ) {
		if( activepool->poll_slot_free[slot] == false &&
			serverid == activepool->serverids_poll[slot] ) {

			// Mark slot as free
			activepool->poll_slot_free[slot] = true;

			activepool->nactive_poll --;

			// Mark server
			servers[serverid].pool_slot = -1;

			// Report
			if( activepool->file_log != NULL ) {
				fprintf( activepool->file_log, "Removed %s from active poll, slot %d (now nactive_poll=%d).\n", servers[serverid].hostname, slot, activepool->nactive_poll );
			}

			assert( activepool->nactive_poll == activepool->pollvec->nfds );

			return;
		}
	}

	cerr << "Couldn't find element with serverid=" << serverid << endl;
	die( "Element not found in activepool_remove_poll" );
}


//
// Name: activepool_remove_nopoll
//
// Description:
//   Removes an element from the pool: the element must
//   be in the CONN_DEACTIVATING state
//
// Input:
//   activepool - the structure
//   serverid - the server to insert
//

void activepool_remove_nopoll( activepool_t *activepool, uint serverid ) {
	assert( activepool != NULL );
	assert( activepool->nactive_nopoll > 0 );

	conn_status_t conn_status = servers[serverid].conn_status;
	assert( conn_status == CONN_DEACTIVATING
		||	conn_status	== CONN_RESOLVING
		||	conn_status == CONN_CONNECTING );

	// Search in the nopoll
	for( uint slot=0; slot<activepool->nactive_nopoll; slot++ ) {
		if( serverid == activepool->serverids_nopoll[slot] ) {
			// Check if this is the last server
			if( activepool->nactive_nopoll == 1 ) {
				// Last server
				activepool->nactive_nopoll = 0;
			} else {
				// There are more servers
				activepool->serverids_nopoll[slot] = activepool->serverids_nopoll[activepool->nactive_nopoll - 1];
				activepool->nactive_nopoll--;

			}
			if( activepool->file_log != NULL ) {
				fprintf( activepool->file_log, "Removed %s from active nopoll, slot %d (now nactive_nopoll=%d).\n", servers[serverid].hostname, slot, activepool->nactive_nopoll );
			}
			return;
		}
	}
	die( "Element not found in activepool_remove_nopoll" );
}


//
// Name: activepool_fill
//
// Description:
//   Tries to fill the active pool with items in the 
//   waiting queue. One item for each call.
//
// Input:
//   activepool - the structure
//   waitq - the waiting queue
//

void activepool_fill( activepool_t *activepool, waitq_t *waitq ) {
	assert( activepool != NULL );

	// Check that it has free slots
	if( !activepool_has_free_slots( activepool ) ) {
		die( "No free slots in active pool" );
	}
	if( waitq_empty( waitq ) ) {
		die( "Waiting queue is empty" );
	}

	// Locate a free server
	time_t now = time(NULL);
	assert( waitq_available( waitq, now ) );
	uint serverid;
	time_t timestamp;
	waitq_pop( waitq, &(serverid), &(timestamp) );
	assert( serverid > 0 );
	assert( timestamp <= now );

	// Report
	if( activepool->file_log != NULL ) {
		fprintf( activepool->file_log, "Adding %s (activepool_fill)\n", servers[serverid].hostname );
	}

	// Insert
	servers[serverid].conn_status = CONN_ACTIVATING;
	activepool_insert_nopoll( activepool, serverid );
}

//
// Name: activepool_has_free_slots
//
// Description:
//   Tells if it is necessary to activepool_fill
//
// Input:
//   activepool - the structure
//
// Return:
//    1 if has free slots
//    0 if it is full
//

int activepool_has_free_slots( activepool_t *activepool ) {
	if( (activepool->nactive_poll + activepool->nactive_nopoll)
			< activepool->maxactive  ) {
		return 1;
	} else {
		return 0;
	}
}

//
// Name: activepool_empty
//
// Description:
//   Tells if it has both queues empty (the poll and nopoll)
//
// Input:
//   activepool - the structure
//
// Return:
//   1 if both are empty
//   0 if any of them is not empty
//

int activepool_empty( activepool_t *activepool ) {
	assert( activepool != NULL );
	assert( activepool->nactive_poll >= 0 );
	assert( activepool->nactive_nopoll >= 0 );

	if( activepool->nactive_poll + activepool->nactive_nopoll == 0 ) {
		return 1;
	} else {
		return 0;
	}
}

//
// Name: activepool_process
//
// Description:
//   Process items in the active queue
// 
// Input:
//   activepool - the structure
//   waitq - the waiting queue (to put servers when needed)
//

void activepool_process( activepool_t *activepool, waitq_t *waitq ) {
	assert( activepool != NULL );

	if( activepool->file_log != NULL ) {
		fprintf( activepool->file_log, "Proccessing activepool (nopoll)\n" );
	}

	// Try to empty 'nopoll' list
	// Elements may be removed, so we need to be careful
	uint slot = 0;
	while( slot < activepool->nactive_nopoll ) {


		// Get serverid 
		uint serverid = activepool->serverids_nopoll[slot];
		assert( serverid > 0 );
		assert( strlen(servers[serverid].hostname) > 0 );

		// Process
		if( activepool->file_log != NULL ) {
			fprintf( activepool->file_log, "- nopoll %s (slot %d of %d) ", servers[serverid].hostname, slot, activepool->nactive_nopoll );
		}
		bool was_removed = activepool_process_nopoll( activepool, waitq, slot, serverid );

		if( !was_removed ) {
			slot++;
			if( activepool->file_log != NULL ) {
				fprintf( activepool->file_log, "!was_removed" );
			}
		} else {
			if( activepool->file_log != NULL ) {
				fprintf( activepool->file_log, "was_removed" );
			}
		}
		if( activepool->file_log != NULL ) {
			fprintf( activepool->file_log, "\n" );
		}
	}

	if( activepool->nactive_poll == 0 ) {
		assert( pollvec_empty( activepool->pollvec ) );
		if( activepool->file_log != NULL ) {
			fprintf( activepool->file_log, "Nothing to poll()\n" );
		}
		return;
	}

	//
	// Poll
	//
	
	if( activepool->file_log != NULL ) {
		fprintf( activepool->file_log, "Processing activepool (poll)\n" );
	}
	uint nevents = pollvec_poll( activepool->pollvec, activepool->pollevents );
	assert( ( nevents == 0 ) || ( nevents > 0 && nevents <= activepool->nactive_poll ) );

	if( activepool->file_log != NULL ) {
		fprintf( activepool->file_log, "- received %d events of %d items\n", nevents, activepool->nactive_poll );
	}

	// Handle polled events
	for( uint evnum=0; evnum<nevents; evnum++ ) {
		// Get serverid of server that received events
		pollevent_t *pollevent = &(activepool->pollevents[evnum]);

		// Report
		if( activepool->file_log != NULL ) {
			fprintf( activepool->file_log, "- poll event for %s\n", servers[pollevent->serverid].hostname );
		}

		// Process
		activepool_process_poll( activepool, waitq, pollevent );
	}

	if( activepool->file_log != NULL ) {
		fprintf( activepool->file_log, "Done processing activepool\n" );
	}
}

//
// Name: activepool_process_nopoll
//
// Description:
//   Process a server in the nopoll list
//
// Input:
//   activepool - the structure
//   waitq - the waiting queue
//   slot - the slot number, to get the adns state
//   serverid - the number of the server
//

bool activepool_process_nopoll( activepool_t *activepool, waitq_t *waitq, uint slot, uint serverid ) {
	assert( servers[serverid].conn_status == CONN_ACTIVATING );

	server_t *server = &(servers[serverid]);


	bool was_removed = false;

	// We need to check if the server has a valid IP
	if( server_has_ip( server ) ) {

		if( activepool->file_log != NULL ) {
			fprintf( activepool->file_log, " has_ip " );
		}

		// Check if the server has more pages to be downloaded
		if( server_has_more_documents( server ) ) {

			if( activepool->file_log != NULL ) {
				fprintf( activepool->file_log, " has_more_documents " );
			}

			// Check if the IP address is blocked
			// We compare the IPs as strings (e.g.: "11.11.11.11")
			bool ip_is_blocked = false;
			if( perfhash_check(activepool->blocked_ip,
				inet_ntoa( server->site->addr ) ) ) {
				cerr << RED << "IP " << inet_ntoa(server->site->addr)
					<< " is blocked" << NOR << endl;
				ip_is_blocked = true;
			}

			// Start to connect
			short events	= 0;
			uint timeout	= 0;

			if( ip_is_blocked ) {
				server->conn_status = CONN_DEACTIVATING;
			} else {
				server_connect_start( server, &(events), &(timeout), activepool->useragent );
			}

			switch( server->conn_status ) {
				// Is connecting
				case CONN_CONNECTING:
					activepool_remove_nopoll( activepool, serverid );
					assert( server->socket >= 0 );
					assert( events > 0 );
					assert( timeout > 0 );
					activepool_insert_poll( activepool, serverid, events, timeout );
					was_removed = true;
					break;

				// Is not connecting (failed)
				case CONN_DEACTIVATING:
					assert( server->socket == -1 );
					assert( events == 0 );
					assert( timeout == 0 );
					activepool_remove_nopoll( activepool, serverid );
					servers[serverid].conn_status = CONN_NONE;
					was_removed = true;

					// Add 1 error 
					servers[serverid].errors_in_this_harvest ++;
					activepool->err_connect ++;

					// If the IP was blocked, skip all
					if( ip_is_blocked ) { 
							server_skip_all( server, activepool->doc_files[activepool->maxactive], HTTP_ERROR_BLOCKED_IP );
						
					// If we can retry, insert again
					} else if( servers[serverid].errors_in_this_harvest <= CONF_MANAGER_MAX_ERRORS_SAME_BATCH ) {
						waitq_push( waitq, serverid, time(NULL) + servers[serverid].wait ); 
					} else {
						server_skip_all( server, activepool->doc_files[activepool->maxactive], HTTP_ERROR_CONNECT );
					}
					break;

				default:
					die( "Inconsistency after server_connect_start" );
			}

		} else {

			if( activepool->file_log != NULL ) {
				fprintf( activepool->file_log, " !has_more_documents " );
			}

			// No more documents
			servers[serverid].conn_status = CONN_DEACTIVATING;
			activepool_remove_nopoll( activepool, serverid );
			servers[serverid].conn_status = CONN_NONE;
			was_removed = true;
		}

	} else {

		// Resolv
		if( activepool->file_log != NULL ) {
			fprintf( activepool->file_log, " !has_ip " );
		}

		// Create a temporal adns state and query slots, these will
		// be eventually copied to adns_states and adns_queries
		adns_state tmp_adns_state;
		adns_query tmp_adns_query;

		// Start resolving
		if( activepool->file_log != NULL ) {
			fprintf( activepool->file_log, " resolver_start " );
		}
		short events		= 0;
		uint timeout		= 0;
		server_resolver_start( server, &(tmp_adns_state), &(tmp_adns_query), &(events), &(timeout) );

		int pool_slot;

		// Check server status
		switch( server->conn_status ) {
			case CONN_ACTIVATING:
				// The server is ok, now has an IP address
				if( activepool->file_log != NULL ) {
					fprintf( activepool->file_log, " now has_ip " );
				}
				cerr << " now has_ip ";
				assert( server_has_ip( server ) );
				assert( server->socket == -1 );
				assert( events == 0 );
				assert( timeout == 0 );
				was_removed = false;
				break;

			case CONN_DEACTIVATING:
				// The dns resolver failed
				if( activepool->file_log != NULL ) {
					fprintf( activepool->file_log, " failed !has_ip " );
				}
				assert( !server_has_ip( server ) );
				assert( server->socket == -1 );
				assert( events == 0 );
				assert( timeout == 0 );

				// Remove
				activepool_remove_nopoll( activepool, serverid );

				// Skip the rest for this server
				server_skip_all( server, activepool->doc_files[activepool->maxactive], HTTP_ERROR_DNS );
				activepool->err_dns ++;

				// Status is CONN_NONE
				servers[serverid].conn_status = CONN_NONE;
				was_removed = true;
				break;

			case CONN_RESOLVING:
				// The server is DNS resolving,
				if( activepool->file_log != NULL ) {
					fprintf( activepool->file_log, " resolving " );
				}

				activepool_remove_nopoll( activepool, serverid );

				assert( server->socket >= 0 );
				assert( events > 0 );
				assert( timeout > 0 );
				activepool_insert_poll( activepool, serverid, events, timeout );
			
				// Check that the adns state is valid
				assert( tmp_adns_state != NULL );
				assert( tmp_adns_query != NULL );

				// Copy the state to one slot
				pool_slot = servers[serverid].pool_slot;
				memcpy( &(activepool->adns_states[pool_slot]), &(tmp_adns_state), sizeof(adns_state ) );
				memcpy( &(activepool->adns_queries[pool_slot]), &(tmp_adns_query), sizeof(adns_query ) );
				was_removed = true;


				break;

			default:
				die( "Inconsistency after server_resolver_start" );
		}
	}

	if( activepool->file_log != NULL ) {
		fprintf( activepool->file_log, " done " );
	}
	return was_removed;
}

//
// Name: activepool_process_poll
//
// Description:
//   Process a server in the poll list
//
// Input:
//   activepool - the structure
//   waitq - the queue
//   pollevent - the recived event
//

void activepool_process_poll( activepool_t *activepool, waitq_t *waitq, pollevent_t *pollevent ) {
	assert( activepool != NULL );
	assert( pollevent != NULL );
	assert( pollevent->serverid > 0 );


	uint serverid		= pollevent->serverid;
	server_t *server	= &(servers[serverid]);
	int revents			= pollevent->revents;
	uint pool_slot		= server->pool_slot;
	assert( pool_slot >= 0 );

	if( activepool->file_log != NULL ) {
		fprintf( activepool->file_log, " event in pool_slot=%d revent=%d ", pool_slot, revents );
	}
	
	// Check server status
	assert( server->conn_status == CONN_RESOLVING
		 || server->conn_status == CONN_CONNECTING
		 || server->conn_status == CONN_REQUESTING
		 || server->conn_status == CONN_RECEIVING );

	// Depending on the connection status
	adns_state *adns_ptr		= NULL;
	adns_query *adns_query_ptr	= NULL;
	short events				= 0;
	uint timeout				= 0;

	switch( server->conn_status ) {

		// The server was resolving and received a poll event
		// probably it is the IP address we are waiting for
		case CONN_RESOLVING:
			adns_ptr		= &(activepool->adns_states[pool_slot]);
			adns_query_ptr	= &(activepool->adns_queries[pool_slot]);
			assert( adns_ptr != NULL );
			assert( (*adns_ptr) != NULL );
			assert( adns_query_ptr != NULL );
			assert( *adns_query_ptr != NULL );
			
			if( activepool->file_log != NULL ) {
				fprintf( activepool->file_log, " poll event for resolver " );
			}

			// Handle event while resolving
			server_resolver_afterpoll( server, adns_ptr, adns_query_ptr, revents );

			switch( server->conn_status ) {

				// Continue resolving
				case CONN_RESOLVING:
					if( activepool->file_log != NULL ) {
						fprintf( activepool->file_log, " answer not ready yet " );
					}
					break;

				// Ok, we have an address, now we go to the nopoll
				// queue and then againt to the poll queue
				case CONN_ACTIVATING:
					if( activepool->file_log != NULL ) {
						fprintf( activepool->file_log, " has_ip " );
					}
					assert( server_has_ip( server ) );
					assert( server->socket == -1 );
					activepool_remove_poll( activepool, serverid );
					activepool_insert_nopoll( activepool, serverid );
					
					break;

				// The dns resolv failed
				case CONN_DEACTIVATING:
					if( activepool->file_log != NULL ) {
						fprintf( activepool->file_log, " resolver failed " );
					}

					// Remove
					assert( server->socket == -1 );
					activepool_remove_poll( activepool, serverid );

					// Skip the other pages
					server_skip_all( server, activepool->doc_files[activepool->maxactive], HTTP_ERROR_DNS );

					activepool->err_dns ++;

					// Status is CONN_NONE
					servers[serverid].conn_status = CONN_NONE;
					break;
				default:
					die( "Inconsisteny after server_resolver_afterpoll" );
			}
			break;

		// We got a poll event while connecting, probably
		// the connection is ready for the request
		case CONN_CONNECTING:

			if( revents & POLLOUT ) {
				server_request_start( server, &(events), &(timeout), activepool->useragent );
			} else {
				server_close( server );
				server->conn_status	= CONN_DEACTIVATING;
			}

			switch( server->conn_status ) {

				// The connection failed, we will NOT try again
				case CONN_DEACTIVATING:
					if( activepool->file_log != NULL ) {
						fprintf( activepool->file_log, " connection failed " );
					}

					// Remove
					activepool_remove_poll( activepool, serverid );

					// Skip the other pages
					// We never retry a connect error in the same harvest
					// (other errors e.g.: NOT_FOUND, etc, can be retried)
					server_skip_all( server, activepool->doc_files[activepool->maxactive], HTTP_ERROR_CONNECT );

					// Status is CONN_NONE
					servers[serverid].conn_status = CONN_NONE;

					// Add 1 error 
					servers[serverid].errors_in_this_harvest ++;
					activepool->err_connect ++;

					break;

				// The connection was sucesful, and the request
				// was sucesfully sent. Notice that we don't consider
				// blocking requests: we must be able to write the request
				// on a single packet
				case CONN_REQUESTING:
					if( activepool->file_log != NULL ) {
						fprintf( activepool->file_log, " requested " );
					}

					// Update parameters
					pollvec_update( activepool->pollvec, &(servers[serverid]), events, time(NULL) + timeout );

					break;

				default:
					die( "Inconsistency after server_request_start" );
			}

			break;

		// We got a poll event after requesting, or
		// while receiving
		case CONN_REQUESTING:
		case CONN_RECEIVING:

			if( revents & (POLLIN|POLLPRI) ) {
				server_receive( server, &(events), &(timeout), activepool->content_files[pool_slot] );
			} else {
				if( revents & POLLERR ) {
					cerr << RED << "POLLERR in " << server->hostname << NOR << endl;
				} else if( revents & POLLHUP ) {
					cerr << RED << "POLLHUP in " << server->hostname << NOR << endl;
				} else if( revents == 0 ) {
					cerr << RED << "POLL TIMEOUT in " << server->hostname << NOR << endl;	
					server->http_status	= HTTP_ERROR_TIMEOUT;
				} else {
					cerr << RED << "Unexpected poll event received: " << revents << " " << server->hostname << NOR << endl;
				}

				server_close( server );
				server->conn_status	= CONN_DEACTIVATING;

				// Check for undefined status
				if( server->http_status == HTTP_STATUS_UNDEFINED ) {

					// We have readed packets but we still
					// have no http code, the server is misbehaving
					cerr << RED << "Server " << server->hostname << " misbehaving: readed packets but got no header data" << NOR << endl;
					server->http_status				= HTTP_ERROR_DISCONNECTED;
				} 
			}

			switch( server->conn_status ) {
				case CONN_DEACTIVATING:
					if( activepool->file_log != NULL ) {
						fprintf( activepool->file_log, " deactivating " );
					}

					// Remove
					activepool_remove_poll( activepool, serverid );
					
					// Page done. Notice that this is the only
					// case in which we want to save the doc status
					// in the corresponding file, to be consistent
					// with saved content data. In all other cases,
					// we just pick the first file.
					server_save_current_page( server, activepool->doc_files[pool_slot] );

					// Check for error conditions
					if( ! HTTP_IS_ERROR( server->http_status ) ) {

						activepool->ok ++;

						// Check if site done
						if( server_has_more_documents( server ) ) {
							// Not done yet, push it in the waiting queue
							waitq_push( waitq, serverid, time(NULL) + servers[serverid].wait );
						}

					} else {

						// Add 1 error 
						servers[serverid].errors_in_this_harvest ++;
						activepool->err_connect ++;

						// Check if there are more documents
						if( server_has_more_documents( server ) ) {
							// Check if we have had too many errors
							if( servers[serverid].errors_in_this_harvest <= CONF_MANAGER_MAX_ERRORS_SAME_BATCH ) { 
								// We haven't had too many errors
								waitq_push( waitq, serverid, time(NULL) + servers[serverid].wait );
							} else {
								// Skip the rest for this server, we have had
								// too many errors with it
								if( server->http_status == HTTP_ERROR_TIMEOUT
								 || server->http_status == HTTP_ERROR_DISCONNECTED ) {
									server_skip_all( server, activepool->doc_files[activepool->maxactive], server->http_status );
								} else {
									server_skip_all( server, activepool->doc_files[activepool->maxactive], HTTP_ERROR_DISCONNECTED );
								}
							}
						}
					}


					// Status is CONN_NONE
					servers[serverid].conn_status = CONN_NONE;

					break;

				case CONN_RECEIVING:
					if( activepool->file_log != NULL ) {
						fprintf( activepool->file_log, " continue receiving " );
					}

					break;

				default:
					die( "Inconsistency after server_receive" );

			}
			break;

		default:
			die( "Inconsistency after receiving poll event" );

	}
}

//
// Name: activepool_save_sites
//
// Description:
//   Saves all sites
//

void activepool_save_sites( activepool_t *activepool, uint total_servers ) {
	assert( activepool != NULL );

	// Open a file for saving servers
	char filename[MAX_STR_LEN];
	sprintf( filename, "%s", ACTIVEPOOL_FILENAME_SITES );
	FILE *fsites = fopen64( filename, "w" );
	assert( fsites != NULL );

	// Go through the list of servers
	map <uint, uint> map_http_status;
	for( uint serverid=1; serverid<=total_servers; serverid++ ) {
		server_t *server = &(servers[serverid]);
		assert( server != NULL );

		// Save this server's site data
		server_save_site( server, fsites );
		map_http_status[server->http_status] ++;
	}

	// Show stats
	map<uint,uint>::iterator status_it;
	cerr << endl << "Site count per last http status" << endl;
	for( status_it = map_http_status.begin(); status_it != map_http_status.end();
			status_it++ ) {
		cerr << HTTP_STR((*status_it).first) << " " << (*status_it).second << endl;
	}


	// Close the site file
	fclose( fsites );


}

//
// Name: activepool_destroy
//
// Description:
//   Destroys the active pool
//

void activepool_destroy( activepool_t *activepool ) {
	assert( activepool != NULL );
	assert( activepool->nactive_poll == 0 );
	assert( activepool->nactive_nopoll == 0 );

	free( activepool->serverids_poll );
	free( activepool->serverids_nopoll );
	free( activepool->pollevents );

	// Close files
	for( uint fileno=0; fileno<=activepool->maxactive; fileno++ ) {
		fclose( (activepool->content_files)[fileno] );
		fclose( (activepool->doc_files)[fileno] );
	}
	free( activepool->content_files );
	free( activepool->doc_files );

	if( activepool->file_log != NULL ) {
		fprintf( activepool->file_log, "Done.\n" );
		fclose( activepool->file_log );
	}
	
	free( activepool );
}

//
// Name: activepool_dump
//
// Description:
//   Shows the active pool
//

void activepool_dump( activepool_t *activepool ) {
	cerr << BLU;
	cerr << "BEGIN activepool dump" << endl;
	cerr << "maxactive      " << activepool->maxactive << endl;
	cerr << "nactive_nopoll   " << activepool->nactive_nopoll << endl;
	for( uint slot=0; slot<activepool->nactive_nopoll; slot++ ) {
		cerr << " slot " << slot << " " << servers[activepool->serverids_nopoll[slot]].hostname << endl;
	}
	cerr << "nactive_poll   " << activepool->nactive_poll << endl;
	for( uint slot=0; slot<activepool->nactive_poll; slot++ ) {
		cerr << " slot " << slot << " " << servers[activepool->serverids_poll[slot]].hostname;
		if( activepool->adns_states[slot] == NULL ) {
			cerr << " adns_state NULL ";
		} else {
			cerr << " adns_state NOT NULL ";
		}
		cerr << endl;
		assert( (uint)(servers[activepool->serverids_poll[slot]].pool_slot) == slot );
	}
	cerr << "END activepool dump" << endl;
	cerr << NOR;
}

//
// Name: activepool_minidump
//
// Description:
//   Shows some info about the active pool, in one line
//   acts like a 'tick' of the clock
//


void activepool_minidump( activepool_t *activepool, waitq_t *waitq ) {
	assert( activepool != NULL );
	cerr << BLU;
	cerr << "   ";
	cerr << "SERVERS ";
	cerr << "Poll=" << activepool->nactive_poll << ", ";
	cerr << "Wait="	<< waitq->nservers;
	cerr << " ";
	cerr << "PAGES ";
	cerr << "ok="       << activepool->ok << ", ";
	cerr << "connErr="  << activepool->err_connect << ", ";
	cerr << "dnsErr="   << activepool->err_dns;
   	cerr << NOR << endl;
}
