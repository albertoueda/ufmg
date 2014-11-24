
#include "harvester.h"
#include "xmlconf-main.h"

server_t *servers	= NULL; // Notice that servers[0] is always EMPTY
harvest_t *harvest	= NULL;
uint HARVESTER_ID	= 0;

//
// Name: main()
//
// Description:
//


int main( int argc, char **argv ) {
	wire_start( "harvester" );
	char filename[MAX_STR_LEN];

	// Scan for a harvester
	HARVESTER_ID = 0;

	// Parse options
	while(1) {
		int option_index = 0;
		static struct option long_options[] = {
			{"help", 0, 0, 0},
			{"force", 1, 0, 0}
		};

		char c = getopt_long( argc, argv, "hf:",
				long_options, &option_index );

		if( c == -1)
			break;

		switch(c) {
			case 0:
				if( !strcmp( long_options[option_index].name, 
							"force" ) ) {
					HARVESTER_ID = atol( optarg );
				} else if( !strcmp( long_options[option_index].name,
							"help" ) ) {
					harvester_usage();
				}
				break;
			case 'f':
				HARVESTER_ID = atol(optarg);
				break;
			case 'h':
				harvester_usage();
				break;
		}
	}

	// Check if a HARVESTER_ID was passed
	if( ! HARVESTER_ID ) {

		for( int i=1; harvest_exists(COLLECTION_HARVEST, i ); i++ ) {
			harvest = harvest_open( COLLECTION_HARVEST, i, true );
			assert( harvest != NULL );

			// Report
			cerr << "Checking harvester #" << harvest->id << " ... ";

			// Check status
			if( harvest->status == STATUS_HARVEST_ASSIGNED ) {
				cerr << "ok, created on " << ctime(&(harvest->creationtime));

				HARVESTER_ID = harvest->id;
				break;

			} else {
				cerr << "skiped, status=" << HARVEST_STATUS_STR(harvest->status) << endl;
			}

			// Close
			harvest_close( harvest );
		}
	} else {
		// If the harvest exists, and we were forced to do it,
		// we should first remove temporary files from the harvest
		if( harvest_exists( COLLECTION_HARVEST, HARVESTER_ID ) ) {
			harvest_remove_files( COLLECTION_HARVEST, HARVESTER_ID );
		}
	}

	if( HARVESTER_ID == 0 ) {
		syslog( LOG_NOTICE, "harvester has no tasks to do. You may force a task with --force harvest_id" );
		// This will prevent the cleanup handler from trying to close
		// an unopened harvester
		harvest = NULL;
		wire_stop(1);
	}

	// Open harvester
	harvest = harvest_open( COLLECTION_HARVEST, HARVESTER_ID, false );
	assert( harvest->readonly == false );
	assert( harvest != NULL );
	assert( harvest->count > 0 );

    // Init useragent
    char useragent[MAX_STR_LEN];
    struct utsname myname;
    uname( &(myname) );

    sprintf( useragent, "%s/%s (%s; %s; %s)",
            PACKAGE,
            VERSION,
            myname.sysname,
            myname.machine,
			CONF_HARVESTER_USER_AGENT_COMMENT);

	cerr << "User-Agent is " << useragent << endl;

	assert( MAX_DOC_LEN > CONF_MAX_FILE_SIZE );

	// Init forbidden IPs
	perfhash_t blocked_ip;
	blocked_ip.check_matches = true;
	perfhash_create( &(blocked_ip), CONF_HARVESTER_BLOCKED_IP );
	cerr << "- Blocked IP: " << CONF_HARVESTER_BLOCKED_IP << endl;

	// Init resolver
	server_resolver_prepare();

	// Prepare harvester
	harvester_prepare();

	// Init the servers list
	harvester_init_servers();


	// Init the auxiliary structures
	uint NSERVERS				= harvest->count_site;
	uint POOLSIZE				= CONF_HARVESTER_NTHREADS_START < NSERVERS ?
									CONF_HARVESTER_NTHREADS_START :
									NSERVERS + 1;

	// Create the waiting queue
	waitq_t *waitq				= waitq_create( NSERVERS );
	assert( waitq != NULL );

	// Directory name
	char dirname[MAX_STR_LEN];
	sprintf( dirname, "%s/%d", COLLECTION_HARVEST, HARVESTER_ID );
	sprintf( filename, "%s/%s/%s", CONF_COLLECTION_BASE, dirname, POLLVEC_FILENAME_LOG );

	// Open polling vector
	FILE *pollvec_log;

	// Check verbosity level
	if( CONF_HARVESTER_LOGLEVEL == LOGLEVEL_VERBOSE ) {
		pollvec_log = fopen64( filename, "w" );
		if( pollvec_log == NULL ) {
			perror( filename );
		}
		assert( pollvec_log != NULL );
		setbuf( pollvec_log, NULL );
	} else {
		pollvec_log = NULL;
	}
	pollvec_t *pollvec			= pollvec_create( (POOLSIZE * 2) + 1, pollvec_log );
	assert( pollvec != NULL );

	// Active Pool
	cerr << "Creating active pool of size " << POOLSIZE << " ... ";
	activepool_t *activepool	= activepool_create( POOLSIZE, pollvec, useragent, &(blocked_ip), dirname );
	assert( activepool != NULL );
	cerr << "done." << endl;

	// Insert documents in the waitq
	cerr << "Creating waiting queue for " << NSERVERS << " servers ... ";
	time_t now = time(NULL);
	for( uint serverid=1; serverid<=NSERVERS; serverid++ ) {
		waitq_push( waitq, serverid, now );
	  	assert( server_has_more_documents( &(servers[serverid]) ) );
	}
	cerr << "done." << endl;

	// Main loop

	time_t last_minidump	= time(NULL);
	do {
		time_t now = time(NULL);

		// Show a minidump, but every 10 seconds at most
		if( now - last_minidump > 10 ) {
			activepool_minidump( activepool, waitq );
			last_minidump	= now;
		}

                if( activepool_empty(activepool) &&
		   !waitq_empty(waitq) &&
		   !waitq_available(waitq,now) ) {
			sleep( CONF_HARVESTER_TIMEOUT_POLL/1000 ); // in seconds
		}

		// Try to keep the activepool full
		while( activepool_has_free_slots( activepool ) &&
			   waitq_available( waitq, now ) ) {
			activepool_fill( activepool, waitq );
		}

		// Process the activepool
		if( !activepool_empty( activepool ) ) {
			activepool_process( activepool, waitq );

		}

		// If we have things in the waitq, but we have
		// too few threads running, cancel the rest of the waitq
		if( !waitq_empty(waitq)) {
			
			uint current_threads = waitq->nservers + activepool->nactive_poll + activepool->nactive_nopoll;

			if( (
					( (uint)(now - harvest->begintime) > CONF_HARVESTER_NTHREADS_SOFTMINTIME )
						&&
					( current_threads < CONF_HARVESTER_NTHREADS_SOFTMIN )
				)
					||
				( current_threads < CONF_HARVESTER_NTHREADS_HARDMIN ) ) {

				// There are too few servers available, cancel the rest
				if( current_threads < CONF_HARVESTER_NTHREADS_HARDMIN ) {
					cerr << "Hard minimum of servers reached" << endl;
				} else {
					cerr << "Soft minimum of servers reached" << endl;
				}
				cerr << "Too few servers available (" << current_threads << "), canceling the rest" << endl;

				// Empty the waitq by skipping all documents
				while( !waitq_empty( waitq ) ) {
					uint serverid;
					time_t nextvisit;
					waitq_pop( waitq, &(serverid), &(nextvisit) );

					server_t *server;
					server = &(servers[serverid]);
					cerr << "Cancelling " << server->hostname << " with " << server->ndocs_done << " of " << server->ndocs << " pages retrieved." << endl;
					server_skip_all( server, activepool->doc_files[activepool->maxactive], HTTP_ERROR_SKIPPED );
				}
			}
		}
	}
	while(!( waitq_empty( waitq ) && activepool_empty(activepool)) );

	// Check if nothing could be downloaded
	if( activepool->ok == 0 ) {
		die( "Zero documents downloaded! Please check network and the 'resolvconf' variable in the configuration file" );	
	}

	// Save sites
	activepool_save_sites( activepool, NSERVERS );

	// Done
	activepool_destroy( activepool );

	// Number of bytes
	cerr << "Total network usage:" << endl;
	cerr << "BYTES_IN  : " << bytes_in << endl;
	cerr << "BYTES_OUT : " << bytes_out << endl;

	// Save harvester
	cerr << "Reporting harvest ended ok at ";
	harvest->bytes_in	= bytes_in;
	harvest->bytes_out	= bytes_out;
	harvest->status		= STATUS_HARVEST_DONE;
	harvest->endtime	= time(NULL);
	cerr << ctime(&(harvest->endtime));

	// Exit
	wire_stop(0);
}

//
// Name: harvester_prepare
//
// Description:
//   Prepares the harvester. There is a global
//   variable with the harvester
//
//

void harvester_prepare() {
	// Get local host name
	char localhost[MAX_STR_LEN];
	gethostname( localhost, MAX_STR_LEN );
	cerr << "Saving hostname: " << localhost << " ... ";
	strcpy( harvest->hostname, localhost );
	cerr << "done." << endl;

	// Save current time
	time_t current_time;
	time(&(current_time));
	cerr << "Saving begintime: ";
	harvest->begintime = current_time;
	cerr << ctime(&(current_time));

	// Create work directory
	char workdir[MAX_STR_LEN];
	sprintf( workdir, "%s/%d", COLLECTION_HARVEST, HARVESTER_ID );
	cerr << "Creating " << workdir << " ... ";
	if( mkdir( workdir, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH ) ) {

		// Something went wrong with mkdir
		if( errno == EEXIST ) {

			// The directory existed, reuse
			cerr << "already exists, using ... ";
		} else {

			// Other error, exit
			perror( "Failed to create to working directory" );
			die( "create work directory" );
		}
	}
	cerr << "done, chdir ... ";

	// Check harvest consistency
	if( harvest->count_site < CONF_HARVESTER_NTHREADS_HARDMIN ) {
		die( "Too few Web sites, less than the configured minimum threads. If this is the first time you use the crawler and you are testing it in a few sites, you should set hardmin to 0 in the configuration." );
	}

	// Save harvest status
	harvest->status = STATUS_HARVEST_RUNNING;
	harvest_save_info( harvest );

	// Change to work directory
	if( chdir( workdir ) ) {
		perror( "Failed to change to working directory" );
		die( "chdir" );
	}
	cerr << "done." << endl;

}

//
// Name: harvester_init_servers
//
// Description:
//   Prepare the servers list

void harvester_init_servers() {
	// Read the pages
	cerr << "                   |--------------------------------------------------|" << endl;
	cerr << "Reading sites     : ";
	harvest_read_list( harvest );
	assert( harvest->doc_list != NULL );

	// Get memory for the servers structure
	// the servers start at 1.
	assert( harvest->count_site > 0 );
	servers = (server_t *)malloc(sizeof(server_t)*(harvest->count_site+1));
	assert( servers != NULL );

	// Add servers to the servers queue
	// Notice that servers[0] is EMPTY, servers
	// start at position 1.
	uint nservers = harvest->count_site;
	uint progress = 0;
	map <siteid_t,uint> map_servers;
	for( uint serverid=1; serverid<=nservers; serverid++ ) {
		// Report progress
		progress++;
		if( progress == (nservers / 50) ) {
			progress = 0;
			cerr << ".";
		}
		
		site_t *site	= &(harvest->site_list[serverid-1]);
		const char *hostname	= (*(harvest->map_sitename))[site->siteid].c_str();

		if( hostname == NULL || strlen(hostname) == 0 ) {
			cerr << "Warning! Site " << site->siteid << " has empty hostname, default to null.localdomain" << endl;
		}

		// Copy to map
		assert( site != NULL );
		assert( site->siteid > (siteid_t)0 );
		map_servers[site->siteid]	= serverid;

		// Copy to site
		server_copy_site( &(servers[serverid]), site, hostname, serverid );

	}
	cerr << " ok" << endl;


	// Add documents
	progress = 0;
	cerr << "Reading documents : "; 
	for( uint i=0; i<harvest->count; i++ ) {
		doc_t *doc = &(harvest->doc_list[i]);

		progress++;
		if( progress == (harvest->count / 50) ) {
			progress = 0;
			cerr << ".";
		}

		// Locate the server
		server_t *server = &(servers[map_servers[doc->siteid]]);

		// Locate the path
		const char *path	= ((*(harvest->map_path))[doc->docid]).c_str();
		assert( path != NULL );

		// This document is from this server
		assert( doc->siteid == server->site->siteid );

		page_t *page = (page_t *)malloc(sizeof(page_t));
		assert( page != NULL );
		page->next	= NULL;
		page->doc	= doc;

		// Insert at the end of the list
		if( server->pages == NULL ) {
			server->pages = page;
		} else {
			page_t *ptr = server->pages;
			while( ptr->next != NULL ) {
				ptr = ptr->next;
			}
			ptr->next	= page;
		}

		// Copy path 
		assert( strlen(path) < MAX_STR_LEN );
		strcpy( page->path, path );

		// Copy relocation
		page->relocation[0]	= '\0';

		server->ndocs++;

		// If the page was not ok, don't use
		// last_visited information
		if( ! ( HTTP_IS_OK( doc->http_status )
			||  doc->http_status == HTTP_NOT_MODIFIED ) ) {
			doc->last_visit	= 0;
		}

		// Set initial parameters for this request
		doc->raw_content_length = 0;
		doc->http_status		= HTTP_STATUS_UNDEFINED;

		// Blank values for measuring times
		timerclear( &(page->timestamp_begin_connect) );
		timerclear( &(page->timestamp_end_connect) );
		timerclear( &(page->timestamp_first_read) );
		timerclear( &(page->timestamp_last_read) );

		page->bytes_first_packet	= 0;
		page->bytes_total			= 0;


	}
	cerr << " ok" << endl;
}

//
// Name: cleanup()
//
// Description:
//   Cleans
//

void cleanup() {
	chdir( CONF_COLLECTION_BASE );
	if( harvest != NULL ) {
		harvest_close( harvest );
	}
}

//
// Name: harvester_usage
//
// Description:
//   Prints a usage message
//

void harvester_usage() {
	cerr << "Usage: program [OPTION]" << endl;
	cerr << "Fetchs the list of pages defined for a harvest" << endl;
	cerr << endl;
	cerr << " -f, --force id       forces it to a fixed harvest_id" << endl;
	cerr << " -h, --help           this help message" << endl;
	cerr << endl;
	wire_stop(0);
}

