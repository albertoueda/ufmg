
#include "reset.h"

//
// Name: main
//
// Description:
//   Deletes files from all directories of the crawler
//

int main( int argc, char **argv ) {
	// Init
	wire_start( "reset" );

	cerr << endl;
	cerr << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << endl;
	cerr << "*** About to DELETE ALL FILES from the collection ***" << endl;
	cerr << "*** Directory: " << CONF_COLLECTION_BASE << endl;
	cerr << endl;
	cerr << "Waiting 10 seconds (Press Ctrl-C to abort)" << endl;
	for( uint i=10; i>0; i-- ) {
		cerr << " Delete all subdirs of " << CONF_COLLECTION_BASE << " in " << i << " sec., Ctrl-C to abort" << endl;
		sleep(1);
	}
	sleep(1);
	cerr << "continuing." << endl;

	// Remove indexes
	cerr << "Removing indexes ... ";
	urlidx_remove( COLLECTION_URL );
	storage_remove( COLLECTION_TEXT );
	linkidx_remove( COLLECTION_LINK );
	harvest_remove_all( COLLECTION_HARVEST );
	metaidx_remove( COLLECTION_METADATA );
	cerr << "done." << endl;

	// Create directories
	cerr << "Creating directories ... ";
	queue <string> dirs;
	dirs.push( COLLECTION_URL );
	dirs.push( COLLECTION_TEXT );
	dirs.push( COLLECTION_LINK );
	dirs.push( COLLECTION_SITELINK );
	dirs.push( COLLECTION_HARVEST );
	dirs.push( COLLECTION_METADATA );
	dirs.push( COLLECTION_ANALYSIS );
	dirs.push( COLLECTION_INDEX );
	dirs.push( COLLECTION_LOG );

	while( ! dirs.empty() ) {
		char dirname[MAX_STR_LEN];
		sprintf( dirname, "%s", dirs.front().c_str() );

		int rc = mkdir( dirname, 0777 );
		if( rc != 0 && errno != EEXIST ) {
			perror( dirname );
			die( "Failed to create directory" );
		}

		dirs.pop();
	}
	cerr << "done." << endl;

	// Create indexes again
	cerr << "Creating indexes ... ";

	cerr << "[metaidx] ";
	metaidx_t *metaidx = metaidx_open( COLLECTION_METADATA, false );
	assert( metaidx != NULL );
	metaidx_close( metaidx );

	cerr << "[linkidx] ";
	linkidx_t *linkidx = linkidx_open( COLLECTION_LINK, false );
	assert( linkidx != NULL );
	linkidx_close( linkidx );

	cerr << "[urlidx] ";
	urlidx_t *urlidx = urlidx_open( COLLECTION_URL, false );
	assert( urlidx != NULL );
	urlidx_close( urlidx );

	cerr << "[storage] ";
	storage_t *storage = storage_open( COLLECTION_TEXT, false );
	assert( storage != NULL );
	storage_close( storage );

	cerr << "done." << endl;

	// Log
	syslog( LOG_NOTICE, "reset deleted and created all indexes" );

	cerr << "The next step should be to use the seeder program to load start URLs" << endl;

	// End
	wire_stop(0);
}

//
// Name: cleanup
//
// Description: 
//   Closes files and clean everything
//

void cleanup() {
}


