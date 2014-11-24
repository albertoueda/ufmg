
#include "server.h"

char SERVER_NAMESERVERS[SERVER_MAX_NAMESERVERS][MAX_STR_LEN];
uint SERVER_COUNT_NAMESERVERS;

//
// Name: server_resolver_prepare
//
// Description:
//   Prepares the name servers
//
// Input:
//   CONF_HARVESTER_RESOLVCONF
//
// Output:
//   Prepares SERVER_NAMERVERS
//

void server_resolver_prepare() {
	cerr << "Preparing resolvconf ... " << endl;
	char buff[MAX_STR_LEN];
	cerr << "- Config  : " << CONF_HARVESTER_RESOLVCONF << endl;
	cerr << "- Maximum : " << SERVER_MAX_NAMESERVERS << endl;

	strcpy( buff, CONF_HARVESTER_RESOLVCONF );
	SERVER_COUNT_NAMESERVERS	= 0;

	// Tokenize
	char *s = strtok( buff, CONF_LIST_SEPARATOR );
	while( s != NULL ) {
		if( !strcmp( s, "nameserver" ) ) {
			s = strtok( NULL, CONF_LIST_SEPARATOR );
			if( s == NULL ) {
				die( "Problem while parsing resolvconf" );
			}
			strcpy( SERVER_NAMESERVERS[SERVER_COUNT_NAMESERVERS], "nameserver " );
			strcat( SERVER_NAMESERVERS[SERVER_COUNT_NAMESERVERS], s );
			cerr << "- " << SERVER_NAMESERVERS[SERVER_COUNT_NAMESERVERS] << endl;
			SERVER_COUNT_NAMESERVERS++;
			assert( SERVER_COUNT_NAMESERVERS < SERVER_MAX_NAMESERVERS );

		}
		s = strtok( NULL, CONF_LIST_SEPARATOR );
	}
	cerr << SERVER_COUNT_NAMESERVERS << " found." << endl;
	assert( SERVER_COUNT_NAMESERVERS > 0 );
	srand(0);
}

//
// Name: server_has_ip
//
// Description:
//   Checks if the server has a valid ip
//
// Input:
//   server - the server structure
//
// Return:
//   true if the server has a nonzero IP
//

bool server_has_ip( server_t *server ) {
	// Create a Zero IP
	char zeros[sizeof(struct in_addr)];
	memset( zeros, 0, sizeof(struct in_addr) );

	// Compare
	if( memcmp( (void *)zeros, (void *)(&(server->site->addr)), sizeof(struct in_addr) ) == 0 ) {

		// The address is equal to zero
		// We don't have an IP
		return false;

	} else {

		// Check if the IP we have is valid
		time_t now = time(NULL);

		if( (uint)(now - (server->site->last_resolved)) > CONF_HARVESTER_DNSEXPIRE ) {
			// We have an IP, but it is not valid
			return false;
		} else {
			// We have an IP and it is valid
			return true;
		}
	}
}

//
// Name: server_resolver_start
//
// Description:
//   Starts the DNS resolver
//
// Input:
//   server - the server structure
//   adnsstate - an adns_state slot
//   adns_query - an adns_query slot
//
// Output:
//   events - events we are interested for poll
//   timeout - timeout for poll in seconds
//

void server_resolver_start( server_t *server, adns_state *adns_ptr, adns_query *adns_query_ptr, short *events, uint *timeout_seconds ) {
	assert( server != NULL );
	assert( adns_ptr != NULL );
	assert( adns_query_ptr != NULL );
	assert( *events == 0 );
	assert( *timeout_seconds == 0 );

	char filename_resolver_debug[MAX_STR_LEN];
	sprintf( filename_resolver_debug, "/tmp/adns_debug_serverid%d", server->serverid );

	// Use a random resolver every time
	int nresolver	= rand() % SERVER_COUNT_NAMESERVERS;

	// We don't reuse the adns_state structure
	// until we're completely sure about how adns works :)

	errno = adns_init_strcfg(
			adns_ptr,
			adns_if_noenv,
			NULL,
			SERVER_NAMESERVERS[nresolver] );

	if( CONF_HARVESTER_LOGLEVEL == LOGLEVEL_VERBOSE ) {
		cerr << "Resolver for '" << server->hostname << "' will use: " << SERVER_NAMESERVERS[nresolver] << endl;
	}

	if( errno != 0 ) {
		perror( "adns_init_strcfg" );
		die( "Error in adns_init_strcfg: possible cause is number of descriptors available" );
	}
	assert( *adns_ptr != NULL );

	// Check hostname
	assert( server->hostname != NULL );
	assert( strlen( server->hostname ) > 0 ) ;

	// Submit query
	int rc = adns_submit( (*adns_ptr),
        server->hostname,
		adns_r_a,
		(adns_queryflags)(adns_qf_quoteok_cname|adns_qf_cname_loose),
         NULL,
		adns_query_ptr);
	assert( rc == 0 );

	// New conn_status
	server->conn_status = CONN_RESOLVING;

	// Write the time stamp
	struct timezone tz;
	gettimeofday( &(server->timestamp_resolver_start), &(tz) );

	// Check the query
	adns_answer *answer;
	rc = adns_check( (*adns_ptr), adns_query_ptr, &(answer), NULL );

	if( rc == EWOULDBLOCK ) {

		struct pollfd fdset_single;
		int nfds			= 1;
		int adns_timeout	= 1000;	// this won't be used
		int err;

		// Make room for receiving fdset_single
		nfds	= 1;

		// Prepare for polling; we must be sure there is only
		// one fd needed. Get the fd for polling
		err = adns_beforepoll( (*adns_ptr), &(fdset_single), &nfds, &adns_timeout, 0);
		assert( nfds == 1 ); // Otherwise, adns wants more than 1 fd
		assert( err == 0 );

		// Data for polling
		server->socket		= fdset_single.fd;
		(*events)			= fdset_single.events;
		(*timeout_seconds)	= CONF_HARVESTER_DNSTIMEOUT;


	} else {

		// The result is here
		server_resolver_done( server, adns_ptr, answer );
		assert( server->conn_status == CONN_ACTIVATING
			 || server->conn_status == CONN_DEACTIVATING );
	}
}

//
// Name: server_resolver_afterpoll
//
// Description:
//   Handles a polling event
//
// Input:
//   server - the server
//   adns_ptr - pointer to adns_state
//   adns_query_ptr - pointer to query state
//   revents - poll events received
//
// Output:
//   server->addr - the address resolved, if any
//   server->conn_status -
//     CONN_ACTIVATING		Ok, server->addr is the address
//     CONN_DEACTIVATING	Failed
//     CONN_RESOLVING		Continue resolving
//

void server_resolver_afterpoll( server_t *server, adns_state *adns_ptr, adns_query *adns_query_ptr, int revents ) {
	assert( server != NULL );
	assert( adns_ptr != NULL );
	assert( *adns_ptr != NULL );
	assert( adns_query_ptr != NULL );
	assert( *adns_query_ptr != NULL );

	assert( server->conn_status == CONN_RESOLVING );
	assert( server->socket >= 0 );

	struct pollfd fds;
	fds.fd	= server->socket;

	bool timedout = false;
	if( revents == 0 ) {
		// It timedout, but we will give it a last chance
		timedout = true;
	}

	// Let adns receive/send data. This is nonblocking
	adns_afterpoll( (*adns_ptr), &(fds), 1, 0 );

	// Check if we have an answer
	adns_answer *answer;
	int rc;

	rc = adns_check( (*adns_ptr), adns_query_ptr, &(answer), NULL );
	if( rc == EWOULDBLOCK ) {
		// Continue resolving

		if( timedout ) {

			// Resolver done
			server_resolver_done( server, adns_ptr, NULL );
			assert( server->conn_status == CONN_DEACTIVATING );

		}

	} else {

		assert( rc >= 0 );

		// We have an answer
		server_resolver_done( server, adns_ptr, answer );

		assert( server->conn_status == CONN_ACTIVATING
			 || server->conn_status == CONN_DEACTIVATING );

	}

}

//
// Name: server_resolver_done
//
// Description:
//   Reads the answer for a adns_check
//
// Input:
//   server - the server
//   adns_ptr - the corresponding adns state
//   answer - the adns answer
//

void server_resolver_done( server_t *server, adns_state *adns_ptr, adns_answer *answer ) {
	assert( server != NULL );
	assert( server->conn_status == CONN_RESOLVING );
	assert( adns_ptr != NULL );

	// Keep the time stamp
	struct timezone tz;
	gettimeofday( &(server->timestamp_resolver_end), &(tz) );

	// Check if we have a valid answer
	if( answer != NULL && answer->status == adns_s_ok ) {

		// We have an answer, copy it
		memcpy( &(server->site->addr), answer->rrs.inaddr, sizeof(struct in_addr) );
		server->site->last_resolved = time(NULL);

		// The answer may be valid. We don't trust DNS servers,
		// in some cases 0.0.0.0 is the IP of a host.
		if( server_has_ip( server ) ) {

			// The IP is good
			cerr << GRE << "IP for " << server->hostname << " is " << inet_ntoa( server->site->addr ) << NOR << endl;

			server->conn_status = CONN_ACTIVATING;

		} else {
			// The IP is bad
			cerr << RED << "Answer seems ok, but the IP resolved for " << server->hostname << " is not valid: " << inet_ntoa( server->site->addr ) << NOR << endl;

			server->conn_status = CONN_DEACTIVATING;
			server->http_status	= HTTP_ERROR_DNS;
		}

		if( server->socket != -1 ) {
			server_close( server );
		}

	} else {

		// We don't have a valid answer
		server->conn_status = CONN_DEACTIVATING;
		server->http_status	= HTTP_ERROR_DNS;

		cerr << RED << "IP lookup failed for " << server->hostname << NOR << endl;

		if( server->socket != -1 ) {
			assert( server->socket != -1 );
			server_close( server );
		}

	}

	// Answer can be null due to a timeout
	if( answer != NULL ) {
		free( answer );
	}

	adns_finish( (*adns_ptr) );
}

//
// Name: server_connect_start
//
// Description:
//   Starts connecting
//
// Input:
//   server - the server to connect
//   useragent - the useragent
//
// Output:
//   server->socket - socket
//   events - for polling
//   timeout - for polling
//

void server_connect_start( server_t *server, short *events, uint *timeout_seconds, char *useragent ) {
	assert( server != NULL );
	assert( server->conn_status = CONN_ACTIVATING );
	assert( server->socket == -1 );
	assert( server_has_ip( server ) );
	assert( (*events) == 0 );
	assert( (*timeout_seconds) == 0 );

	int rc;

	// Copy address to structure
	struct sockaddr_in address;
	memset((char *)&address, 0, sizeof(struct sockaddr_in));
	memcpy(&address.sin_addr, &(server->site->addr), sizeof(in_addr) );

	// Prepare address
	address.sin_family = AF_INET;
	address.sin_port = htons(HTTP_PORT);

	// Create socket, this should not fail unless we
	// ran out of file descriptors
	server->socket = socket(address.sin_family, SOCK_STREAM, 0);
	if( server->socket < 0 ) {
		perror( "socket" );
		die( "Error when creating a socket" );
	}
	assert( server->socket > 0 );

	// Put socket in nonblocking mode
	rc = fcntl( server->socket, F_SETFL, O_RDWR|O_NONBLOCK );
	assert( rc == 0 );

    // Connect (async)
    rc = connect(server->socket,
            (struct sockaddr *)&(address), sizeof(address));

	// Note time
	struct timezone tz;
	gettimeofday( &(server->pages->timestamp_begin_connect), &(tz) );

	// Check connect status
	if( errno == EINPROGRESS ) {

		// Connection in progress
		server->conn_status = CONN_CONNECTING;
		assert( server->socket >= 0 );
		(*events)			= POLLOUT;
		(*timeout_seconds)	= CONF_CONNECTION_TIMEOUT;

	} else if( rc < 0 ) {

		// Connect failed
		server->socket = -1;
		server->conn_status = CONN_DEACTIVATING;
		server->http_status	= HTTP_ERROR_CONNECT;

	} else {

		// Connection ok, start request
		server_request_start( server, events, timeout_seconds, useragent );
	}
}

//
// Name: server_request_start
//
// Description:
//   Start the request
//
// Input:
//   server - the server to start request
//   useragent - the identification of the robot
//  
// Output:
//   events - for polling
//   timeout_seconds - for polling
//

void server_request_start( server_t *server, short *events, uint *timeout_seconds, char *useragent ) {
	assert( server != NULL );
	assert( server->socket >= 0 );
	assert( server->pages != NULL );
	assert( server->pages->doc != NULL );
	assert( (*events) == 0 );
	assert( (*timeout_seconds) == 0 );

	// Note time
	struct timezone tz;
	gettimeofday( &(server->pages->timestamp_end_connect), &(tz) );

	// Get memory for request
	char *request		= (char *)malloc(sizeof(char)*REQUEST_MAXSIZE);
	uint request_size	= 0;

	page_t *page		= server->pages;
	assert( page != NULL );

	// Create request
	page_create_request( server, page, useragent, REQUEST_MAXSIZE, request, &(request_size) );
	assert( request_size > 0 );


	// Write the request
	ssize_t written			= write( server->socket, request, request_size);
	bytes_out				+= written;
	server->site->bytes_out	+= written;

	// Check if we wrote all the bytes we intend to write
	// Notice that we are will discard this server if
	// we cannot write the request on a single packet
	if( written != (ssize_t)request_size ) { 

		// Failed while writing request
		perror( "Writing request" );
		cerr << RED << "> Request for " << server->hostname << "/" << page->path << " failed " << NOR << endl;

		// Close
		server_close( server );

		server->conn_status = CONN_DEACTIVATING;
		server->http_status	= HTTP_ERROR_REQUESTING;

	} else {

		// Request sent
		if( CONF_HARVESTER_LOGLEVEL == LOGLEVEL_VERBOSE ) {
			cerr << GRE << "> Wrote request for " << server->hostname << "/" << page->path << NOR << endl;
		}

		// Continue
		server->conn_status	= CONN_REQUESTING;
		(*events)			= POLLIN;
		(*timeout_seconds)	= CONF_HARVESTER_TIMEOUT_READWRITE;
	}

	// Free the memory for the request
	free( request );
}

//
// Name: server_receive
//
// Description:
//   Receives data from the server
// 
// Input:
//   server - the server
//
//

void server_receive( server_t *server, short *events, uint *timeout_seconds, FILE *content_file ) {
	assert( server != NULL );
	assert( server->socket >= 0 );
	assert( server->pages != NULL );
	assert( server->pages->doc != NULL );

	char *buffer = (char *)malloc(sizeof(char)*BUFFER_SIZE);
	assert( buffer != NULL );

	ssize_t readed			= read( server->socket, buffer, BUFFER_SIZE );
	bytes_in				+= readed;
	server->site->bytes_in	+= readed;

	// Note time and size, for measuring speed
	struct timezone tz;
	gettimeofday( &(server->pages->timestamp_last_read), &(tz) );
	if( !timerisset( &(server->pages->timestamp_first_read) ) ) {
		memcpy( &(server->pages->timestamp_first_read), &(server->pages->timestamp_last_read), sizeof(struct timeval) );
	}
	server->pages->bytes_total	+=	readed;
	if( server->pages->bytes_first_packet == 0 ) {
		server->pages->bytes_first_packet	= server->pages->bytes_total;
	}

	// Check if we readed something
	if( readed > 0 ) {

		if( server->conn_status == CONN_REQUESTING ) {

			// The packet contains headers
			ssize_t headers_end = server_parse_headers( server, buffer, readed );
			// Check the status after parsing headers
			if( headers_end == -1 ) {
				
				// A problem was found while parsing headers
				assert( server->http_status == HTTP_ERROR_PROTOCOL );

				cerr << RED << "Problem while parsing headers" << NOR << endl;

				server->conn_status = CONN_DEACTIVATING;
				server_close( server );

			} else if( HTTP_IS_REDIRECT(server->http_status) &&
					   server->http_status != HTTP_NOT_MODIFIED ) {

				char *relocation = server->pages->relocation;

				if( strlen( relocation ) > 0 ) {
					// A special type, for documents containing the
					// redirect address
					server->pages->doc->mime_type = MIME_REDIRECT;
					fwrite( relocation, strlen( relocation), 1, content_file );
					server->pages->doc->raw_content_length = strlen(relocation);

				} else {
					cerr << RED << "Got http code " << server->http_status
						<< " but no Location header" << NOR << endl;
				}

				server->conn_status = CONN_DEACTIVATING;
				server_close( server );
				
				
			} else if( !HTTP_IS_OK(server->http_status) ) {

				// There is no need to read the data
				if( CONF_HARVESTER_LOGLEVEL == LOGLEVEL_VERBOSE ) {
					cerr << RED << "Status is not ok: " << HTTP_STR( server->http_status ) << NOR << endl;
				}
				server->conn_status = CONN_DEACTIVATING;
				server_close( server );

			} else {

				server->pages->doc->number_visits_changed ++;

				if( headers_end < readed ) {
					// Write the rest
					fwrite( (buffer+headers_end), (readed-headers_end), 1, content_file );
					server->pages->doc->raw_content_length += (readed-headers_end);
				}
				// Now we are receiving
				server->conn_status	= CONN_RECEIVING;

			}

		} else {
			assert( server->conn_status == CONN_RECEIVING );

			// Check for max size. We don't check for this on the
			// first packet, because we expect CONF_MAX_FILE_SIZE to
			// be substantially larget than the packet size.
			if( server->pages->doc->raw_content_length + readed < CONF_MAX_FILE_SIZE ) {
				// Write data to file
				fwrite( buffer, readed, 1, content_file );
				server->pages->doc->raw_content_length += readed;
			} else {

				// Truncate. Downloaded file is too large
				cerr << BRO << "Truncating web page: too large" << NOR << endl;
				server->http_status = HTTP_PARTIAL;
				server->conn_status = CONN_DEACTIVATING;
				server_close( server );
			}
		}

		if( CONF_HARVESTER_LOGLEVEL == LOGLEVEL_VERBOSE ) {
			cerr << "< Received " << readed << " from " << server->hostname << endl;
		}

	} else {

		// Nothing readed, deactivating
		server->conn_status = CONN_DEACTIVATING;

		// Check if the status is unknown, then we haven't read
		// an appropiate response header, so there was a problem
		// with the protocol
		if( server->http_status == HTTP_STATUS_UNDEFINED ) {
			server->http_status				= HTTP_ERROR_PROTOCOL;
		}
		server_close( server );

		if( readed < 0 ) {
			perror( "Error in read" );
			cerr << RED << "Error while reading from " << server->hostname << NOR << endl;
		}

	}
	free( buffer );
}

//
// Name: server_parse_headers
//
// Description:
//   Parses an incoming buffer with headers
//
// Input:
//   server - the server object
//   buffer - the buffer with data
//   bufsize - the size of the buffer
//
// Output:
//   server->site is updated to reflect the headers
//   server->page->doc (the current page being fetched) is updated also
//
// Return:
//   the number of bytes parsed as headers
//   -1 on protocol error
//

ssize_t server_parse_headers( server_t *server, char *buffer, ssize_t bufsize ) {
	assert( server != NULL );
	assert( server->conn_status == CONN_REQUESTING );
	assert( server->socket > 0 );
	assert( server->site != NULL );
	assert( server->pages != NULL );
	assert( server->pages->doc != NULL );

	// HTTP_status is undefined until informed by server
	server->http_status = HTTP_STATUS_UNDEFINED;

	// Flag when readed code
	bool readed_http_code = false;
	char line[MAX_STR_LEN];
	ssize_t pos		 = 0;
	uint hpos		 = 0;

	// Go through the line
	while( pos < bufsize ) {
		register char c = buffer[pos];

		if( c == '\r' ) {
			// Ignore
		} else if( c == '\n' ) {
			// Done with the line
			line[hpos]	= '\0';

			if( hpos == 0 ) {

				// We received a blank line
				if( readed_http_code ) {

					// Blank line means headers done
					assert( server->http_status != HTTP_STATUS_UNDEFINED );
					return pos;

				} else {

					// Blank line means protocol error,
					// no status code was received
					server->http_status = HTTP_ERROR_PROTOCOL;
					cerr << RED << "No headers were received, blank line found before http code" << NOR << endl;
					return -1;

				}

			} else {
				// Normal line: parse as http code or line
				if( readed_http_code ) {

					// Normal http header
					server_parse_http_header( server, line, hpos );

				} else {

					// Http header with code
					server->http_status = server_parse_http_code( server, line, hpos );
					// Check code
					if( server->http_status == HTTP_ERROR_PROTOCOL ) {
						// Protocol error (no code)
						cerr << RED << "No http code received in this line " << line << NOR << endl;
						return -1;

					} else {
						// Ok, continue with the other headers
						readed_http_code = true;
						if( CONF_HARVESTER_LOGLEVEL == LOGLEVEL_VERBOSE ) {
							cerr << GRE << "< http_status=" << server->http_status << NOR << endl;
						}
					}
				}

				// Reset the line line
				hpos = 0;
			}
		} else {
			// Copy, honoring the maximum string length
			if( hpos < MAX_STR_LEN - 1 ) {
				line[hpos] = c;
				hpos++;
			} else {
				line[hpos] = '\0';
				cerr << RED << "Header line to wide: " << line << NOR;
				server->http_status = HTTP_ERROR_PROTOCOL;
				return -1;
			}
		}

		pos++;
	}

	// I exhausted the buffer but couldn't find a blank
	// line signaling the end of the headers
	if( server->http_status == HTTP_STATUS_UNDEFINED ) {
		server->http_status = HTTP_ERROR_PROTOCOL;
		return -1;
	} else if( HTTP_IS_OK( server->http_status ) ) {
		// Status ok, but no blank line at the end,
		// protocol error
		server->http_status = HTTP_ERROR_PROTOCOL;
		return -1;
	} else {
		// HTTP status is not ok, we are not
		// expecting data, so we are done with the headers,
		// even if there is no blank line at the end of them
		return pos;
	}
}


//
// Name: server_skip_all
//
// Description:
//   Skip the rest of the server; this just cleans the
//   server queue and assigns the http_code to all the
//   documents remaining in the queue.
//   Tipically it is used if DNS lookup fails.
//
// Input:
//   server - the server object
//   http_code - the http code for it
//

void server_skip_all( server_t *server, FILE *doc_file, uint http_code ) {
	assert( server != NULL );
	assert( http_code != 0 );
	assert( http_code == HTTP_ERROR_CONNECT
		 || http_code == HTTP_ERROR_TIMEOUT
		 || http_code == HTTP_ERROR_DISCONNECTED
		 || http_code == HTTP_ERROR_DNS
		 || http_code == HTTP_ERROR_SKIPPED
		 || http_code == HTTP_ERROR_BLOCKED_IP );

	// Check if this was a real error
	if( http_code == HTTP_ERROR_SKIPPED ) {
		// We are prematurely stopping this harvest because
		// there are too few sites available
		cerr << RED << "Skipping all from " << server->hostname << NOR << endl;
		// Error count must be zeroed
		server->site->count_error = 0;
	} else {
		// We are skiping because of an error
		server->site->count_error ++;
		cerr << RED << "Web server had an error, now skipping all from " << server->hostname << ", server error count now is " << server->site->count_error << NOR << endl;

		// We reset the error count in this harvest,
		// because we're not going to use it anymore,
		// except to check if a website had only errors when
		// saving site data; in that case, we don't want
		// to add a second error
		server->errors_in_this_harvest = 0;
	}

	// Go through the list of pages
	while( server->ndocs_done < server->ndocs ) {
		server->pages->doc->http_status			= http_code;
		server->pages->doc->raw_content_length	= 0;
		server_save_current_page( server, doc_file );
	}

	// Dont left the server with an undef status
	if( server->http_status == HTTP_STATUS_UNDEFINED ) {
		server->http_status = http_code;
	}

	assert( server->ndocs_done == server->ndocs );
}

//
// Name: server_has_more_documents
//
// Description:
//   Checks if the server has more documents that need
//   to be downloaded
//
// Input:
//   server - the server
//
// Return:
//   true if there are more documents in the queue
//   false otherwise
//

bool server_has_more_documents( server_t *server ) {
	assert( server != NULL );
	assert( server->ndocs > 0 );

	if( server->ndocs_done == server->ndocs ) {
		assert( server->pages == NULL );
		return false;
	} else {
		assert( server->pages != NULL );
		return true;
	}
}

//
// Name: server_copy_site
//
// Description:
//   Copy the information from a site
//
// Input:
//   server - the server structure
//   site - the site structure
//   hostname - the site hostname
//   serverid - the place in the servers list
//

void server_copy_site( server_t *server, site_t *site, const char *hostname, uint serverid ) {
	assert( server != NULL );
	assert( site != NULL );
	assert( hostname != NULL );
	assert( serverid > 0 );

	// Copy site
	server->site				= site;

	// Copy hostname
	server->hostname = (char *)malloc(sizeof(char)*MAX_STR_LEN);
	if( strlen( hostname ) > 0 ) {
		assert( strlen( hostname ) <= MAX_STR_LEN );
		strcpy( server->hostname, hostname );
	} else {
		// Bug: something has no hostname
		strcpy( server->hostname, "null.null" );
	}

	// Server
	server->pool_slot	= -1;
	server->serverid	= serverid;
	server->socket		= -1;
	server->http_status	= HTTP_STATUS_UNDEFINED;

	// Set number of documents
	server->ndocs_done	= 0;
	server->ndocs		= 0;
	server->pages		= NULL;

	// Set data to store speeds
	timerclear( &(server->timestamp_resolver_start) );
	timerclear( &(server->timestamp_resolver_end) );

	// Waiting time
	server->wait		= CONF_HARVESTER_WAIT_NORMAL;
	if( site->count_doc >= CONF_HARVESTER_WAIT_COUNTBIG ) {
		server->wait	= CONF_HARVESTER_WAIT_BIG;
	}

	// Consecutive errors
	server->errors_in_this_harvest	= 0;

	// Connection status
	server->conn_status	= CONN_NONE;
}

//
// Name: server_parse_http_code
//
// Description:
//   Parses the first line of a response from the server
//
// Input:
//   server - the server object
//   header - The header line
//   len - the length of the line
//
// Return:
//   http_code or HTTP_ERROR_PROTOCOL
//   

uint server_parse_http_code( server_t *server, char *header, uint len ) {
	assert( header != NULL );
	assert( len > 0 );
	assert( header[len] == '\0' );
	assert( server != NULL );
	assert( server->http_status == HTTP_STATUS_UNDEFINED );

    // Skip the initial portion
    if( len < 9 ) { // "HTTP/x.x "
        return HTTP_ERROR_PROTOCOL;
    } else {
		uint code = strtol( (header)+9, (char **)NULL, 10 );
		if( code >= 100 ) {
			return code;
		} else {
			return HTTP_ERROR_PROTOCOL;
		}
    }
}

//
// Name: server_parse_http_header
//
// Description:
//   Parses a line of the header. Not the initial line, that
//   is parsed by server_parse_http_code.
//
// Input:
//   server - the server object
//   header - The header line
//   len - the length of the line
//   
// Output:
//   server->page

void server_parse_http_header(server_t *server, char *header, uint len ) {
	assert( header != NULL );
	assert( len > 0 );
	assert( header[len] == '\0' );
	assert( server != NULL );
	assert( server->http_status != HTTP_STATUS_UNDEFINED );

	// Note that server->pages contains at this moment
	// the document that is being downloaded, as it is at
	// the beginning of the list
	assert( server->pages != NULL );
	assert( server->pages->doc != NULL );

	// Get the name of the header
	if( len>15 && !strncasecmp(header,"Last-Modified: ", 15) ) {
		// Date
		(header)+=15;

		// Get date
		struct tm last_modified;
		strptime( header, HTTP_TIME_FORMAT, &last_modified );

		// Store, check for the condition of a wrong date
		// from the server
		time_t now	= time(NULL);
		server->pages->doc->last_modified = mktime( &(last_modified) );
		if( server->pages->doc->last_modified > now ) {
			server->pages->doc->last_modified	= now;
		}

	} else if( len>14 && !strncasecmp(header,"Content-Type: ", 14) ) {
        char *charset;
		// Content-Type
		(header)+=14;

		// Skip leading spaces in content-type
		while( *header == ' ' ) {
			(header)++;
		}

		server->pages->doc->mime_type = metaidx_mime_type(server->pages->path,header);
        
        // Get Charset
        (charset)=strchr(header, ';');

        if (charset == NULL) {
            charset = CONF_GATHERER_DEFAULTCHARSET;
        }
        else {
            (charset)++;
            while (*charset == ' ' || *charset != '=') {
                (charset)++;
            }
            (charset)++;
        }
        
        server->pages->doc->charset = http_charset(charset);

	} else if( len>10 && !strncasecmp(header,"Location: ", 10 ) ) {
		// Location
		(header)+=10;

		// Relocation header
		strcpy( server->pages->relocation, header );

	} else {

		// We don't care about content-length
		// We don't care about anything else
	}
}

//
// Name: server_close
//
// Description:
//   Closes the connection with the server
//
// Input:
//   server->socket - the current socket, must not be -1
//
// Output:
//   server->socket - set to -1
//

void server_close( server_t *server ) {
	assert( server != NULL );

	if( server->socket == -1 ) {
		cerr << RED << "Warning: attempt to close -1 socket in " << server->hostname << NOR << endl;
		return;
	}

	int rc = close( server->socket );
	if( rc != 0 ) {
		perror( "close" );
		assert( 0 );
	}
	server->socket = -1;
}

//
// Name: server_save_current_page
//
// Description:
//   Saves the current page (the first page in the document list).
//   Advances to the next page.
//   This is usually called after removing the server from
//   activepool_poll.
//
// Input:
//   server->pages - the list of pages
//
// Output:
//   server->ndocs_done - increases by one
//   server->pages - advances to the next page
//


void server_save_current_page( server_t *server, FILE *doc_file ) {
	assert( server != NULL );
	assert( server->site != NULL );
	assert( server->pages != NULL );
	assert( server->ndocs_done < server->ndocs );

	// Get the page and document
	site_t *site = server->site;
	page_t *page = server->pages;
	doc_t  *doc  = page->doc;

	// Copy the last http status observed with the server to
	// this document's http_status
	if( doc->http_status == HTTP_STATUS_UNDEFINED ) {
		if( server->http_status == HTTP_STATUS_UNDEFINED ) {
			cerr << "Doc has undefined status while trying to save: siteid=" << site->siteid << " docid=" << doc->docid << " path=" << page->path << endl;
			die( "Server had undefined status" );
		}

		doc->http_status = server->http_status;
	}

	// Save some data about the request. Notice that we set last_visit=now
	// AFTER we made the request, because we need the actual last visit
	// to issue a 'If-Modified-Since' header.
	doc->last_visit = time(NULL);

	// We must be absolutely sure that the document last-modified
	// date is less or equal than the last-visit date or we may
	// end up in a loop
	if( doc->last_modified > doc->last_visit ) {
		doc->last_modified	= doc->last_visit;
	}

	if( doc->number_visits == 0 ) {
		doc->first_visit = doc->last_visit;
	}
	doc->number_visits ++;

	// Calculate latency and speed
	if( timerisset( &(page->timestamp_begin_connect) )
	 && timerisset( &(page->timestamp_end_connect) ) 
	 && timerisset( &(page->timestamp_last_read) ) 
	 && (page->bytes_total > 0) ) {

		doc->latency_connect	= timer_delta( &(page->timestamp_end_connect), &(page->timestamp_begin_connect) );

		// Check if we have more than one packet (in that case, the
		// calculation is more accurate)
		if( page->bytes_first_packet == page->bytes_total ) {

			// We had only one packet, latency is latency of connect,
			// but we cannot measure the latency of writing and reading,
			// so we will (wrongly) asume it is zero
			doc->latency			= 0;
			doc->effective_speed	= (float)(page->bytes_total) / timer_delta( &(page->timestamp_last_read), &(page->timestamp_end_connect) );

		} else {

			// We had more than one packet, measure the speed using all packets
			// except the first one
			doc->effective_speed	= (float)((page->bytes_total) - page->bytes_first_packet) / timer_delta( &(page->timestamp_last_read), &(page->timestamp_first_read) );

			// Start with a latency that is all the time between the end
			// of the connect and the receiving of the first packet
			doc->latency			= timer_delta( &(page->timestamp_first_read), &(page->timestamp_end_connect) );
			
			// Correct this latency by discounting the time is took to
			// transfer the first packet.
			doc->latency		   -= (float)(page->bytes_first_packet) / (doc->effective_speed);

			// We can get a very low latency, correct to 0
			if( doc->latency < 0 ) {
				doc->latency	= 0;
			}
		}

	}


	// If the page was sucesfully fetched, reset the error counter for the website
	if( HTTP_IS_OK( doc->http_status ) || HTTP_IS_REDIRECT( doc->http_status ) ) {
		site->count_error = 0;
	}

	// Update data about the site
	site->last_visit	= doc->last_visit;
	if( doc->raw_content_length > 0 ) {
		site->raw_content_length += doc->raw_content_length;
	}

	// Report this page
	if( HTTP_IS_ERROR(server->pages->doc->http_status) ) {
		// Error
		cerr << RED;
	} else if( HTTP_IS_OK(server->pages->doc->http_status) ) {
		// Ok, downloaded
		cerr << GRE;
	} else {
		// Ok, but no data
		cerr << BRO;
	}

	cerr << HTTP_STR(server->pages->doc->http_status) << " " << server->hostname << "/" << server->pages->path << " " << doc->raw_content_length;

	cerr << NOR << endl;

	// Save the doc_t structure to disk
	int rc = fwrite( (void *)(doc), sizeof(doc_t), 1, doc_file );
	if( rc != 1 ) {
		perror( "fwrite" );
		die( "Can't write doc_t to disk" );
	}

	// Advance the pointer to the next page
	server->pages = page->next;
	server->ndocs_done++;

	if( server->ndocs_done == server->ndocs ) {
		assert( server->pages == NULL );
	}
}

//
// Name: server_save_site
//
// Description:
//   Saves the data about a site
//
// Input:
//   server->site - the site structure
//   site_file - the file that we must write to
//


void server_save_site( server_t *server, FILE *site_file ) {
	assert( server != NULL );
	assert( server->site != NULL );
	assert( server->site->last_visit > 0 );

	cerr << "Saving site data for " << server->hostname << ": last http_status was " << HTTP_STR(server->http_status) << endl;

	// Check if the site had only errors
	if( server->errors_in_this_harvest >= server->ndocs ) {
		server->site->count_error ++;
	}

	// Check if we have a latency for the resolver
	if( timerisset( &(server->timestamp_resolver_start) )
	 && timerisset( &(server->timestamp_resolver_end) ) ) {
		server->site->resolver_latency	= timer_delta( &(server->timestamp_resolver_end), &(server->timestamp_resolver_start) );
	}

	// Save the site_t structure to disk
	int rc = fwrite( (void *)(server->site), sizeof(site_t), 1, site_file );
	assert( rc == 1 );
}
