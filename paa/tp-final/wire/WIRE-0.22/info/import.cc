
#include "import.h"

// 
// Name: main
//
// Description:
//   Main program for the import
//

int main( int argc, char **argv ) {
	wire_start("import" );

	// Parse options
	bool opt_import_date	= false;
	char opt_filename_date[MAX_STR_LEN];

	bool opt_import_link	= false;
	char opt_filename_link[MAX_STR_LEN];

	bool opt_import_content_length	= false;
	char opt_filename_content_length[MAX_STR_LEN];

	bool opt_import_url	= false;
	char opt_filename_url[MAX_STR_LEN];

	while(1) {
		int option_index = 0;

		static struct option long_options[] = {
			{"help", 0, 0, 0},
			{"date", 1, 0, 0},
			{"link", 1, 0, 0},
			{"content_length", 1, 0, 0},
			{"url", 1, 0, 0},
			{0, 0, 0, 0}
		};

		char c = getopt_long (argc, argv, "h",
			long_options, &option_index);

		if (c == -1)
			break;

		switch (c) {
			case 0:
				if( !strcmp( long_options[option_index].name, "date" ) ) {
					opt_import_date = true;
					strcpy( opt_filename_date, optarg );

				} else if( !strcmp( long_options[option_index].name, "link" ) ) {
					opt_import_link = true;
					strcpy( opt_filename_link, optarg );

				} else if( !strcmp( long_options[option_index].name, "content_length" ) ) {
					opt_import_content_length = true;
					strcpy( opt_filename_content_length, optarg );

				} else if( !strcmp( long_options[option_index].name, "url" ) ) {
					opt_import_url = true;
					strcpy( opt_filename_url, optarg );

				} else if( !strcmp( long_options[option_index].name,
							"help" ) ) {
					import_usage();
				}
				break;
			case 'h':
				import_usage();
				break;
		}
	}

	if( !(opt_import_link||opt_import_url) ) {
		import_usage();
	}

	// Input files
	ifstream input_url;
	ifstream input_link;
	ifstream input_date;
	ifstream input_content_length;

	// Open input files and indices
	metaidx = metaidx_open( COLLECTION_METADATA, false );
	assert( metaidx != NULL );

	if( metaidx->count_doc > 0 ) {
//		die( "Metaidx must be empty for importing data" );
	}

	// Filename of URLs
	if( opt_import_url ) {
		urlidx = urlidx_open( COLLECTION_URL, false );
		assert( urlidx != NULL );
		input_url.open( opt_filename_url );
		if( ! input_url.is_open() ) {
			perror( opt_filename_url );
			die( "Opening input file of URLs" );
		}
	}

	// Filename of dates
	if( opt_import_date ) {
		if( ! opt_import_url ) {
			die( "Importing dates requires import url" );
		}
		input_date.open( opt_filename_date );
		if( ! input_date.is_open() ) {
			perror( opt_filename_date );
			die( "Opening input file of dates" );
		}
	}

	// Content length
	if( opt_import_content_length ) {
		if( ! opt_import_url ) {
			die( "Importing content length requires import url" );
		}
		input_content_length.open( opt_filename_content_length );
		if( ! input_content_length.is_open() ) {
			perror( opt_filename_content_length );
			die( "Opening input file of content_length" );
		}
	}

	// Filename of links
	if( opt_import_link ) {
		linkidx = linkidx_open( COLLECTION_LINK, false );
		assert( linkidx != NULL );
		input_link.open( opt_filename_link );
		if( ! input_link.is_open() ) {
			perror( opt_filename_link );
			die( "Opening input file of links" );
		}
	}

	if( opt_import_url ) {

		doc_t doc;
		doc.docid = 1;
		docid_t docid = 1;
		site_t site;
		siteid_t siteid;
		char line[(MAX_STR_LEN * 2) + 1];
		char url[MAX_STR_LEN];
		char protocol[MAX_STR_LEN];
		char sitename[MAX_STR_LEN];
		char path[MAX_STR_LEN];
		urlidx_status_t urlidx_rc;

		while( !input_url.eof() && input_url.getline( line, MAX_STR_LEN ) ) {

			// Copy url
			strcpy( url, line );

			if( docid % 10000 == 0 ) {
				cerr << docid << " " << url << " ";
			}

			// Check relative URL
			if( ! strchr( url, ':' ) ) {
				cerr << url << endl;
				die( "Relative URLs not allowed while importing" );
			}

			// Parse URL
			if( ! urlidx_parse_complete_url( url, protocol, sitename, path ) ) {
				cerr << url << endl;
				die( "Problem while parsing URL" );
			}

			// Sitename to lowercase
			for( uint i = 0; i < strlen(sitename); i++ ) {
				sitename[i] = tolower( sitename[i] );

				// Check port numbers
				if( sitename[i] == ':' ) {
					// Strip port number
					sitename[i] = '\0';
					continue;
				}

				// Check other chars
				if( !isalnum(sitename[i]) && sitename[i] != '.' && sitename[i] != '-' && sitename[i] != '_' ) {

					// Site name not acceptable
					die( "Site name not acceptable" );
				}
			}

			// Get the site
			urlidx_rc = urlidx_resolve_site( urlidx, sitename, &(siteid) );
			if( urlidx_rc == URLIDX_CREATED_SITE ) {
				assert( siteid > 0 );
				metaidx_site_default( &(site) );
				site.siteid		= siteid;
				site.count_doc	= 0;
				site.status		= STATUS_SITE_VISITED;
				metaidx_site_store( metaidx, &(site) );
			}

			// No post-processing is done to urls
			docid_t urlidx_docid;
			urlidx_rc = urlidx_resolve_path( urlidx, siteid, path, &(urlidx_docid) );
			if( urlidx_rc == URLIDX_CREATED_PATH
			 || urlidx_rc == URLIDX_EXISTENT ) {

				// If it is new
				if( urlidx_rc == URLIDX_CREATED_PATH && urlidx_docid != docid ) {
					// Check correlative docids
					cerr << "urlidx_docid : " << urlidx_docid << endl;
					cerr << "docid        : " << docid << endl;
					die( "Wrong correlative docid" );
				}

				metaidx_doc_default( &(doc) );
				doc.docid		= docid;

				// Set other parameters
				doc.siteid		= siteid;
				doc.status		= STATUS_DOC_GATHERED;
				doc.http_status	= HTTP_OK;

				// Check if this was duplicate (e.g.: the site name or path
				// was normalized by URLIDX)
				if( urlidx_rc == URLIDX_EXISTENT ) {

					// Mark as duplicate
					doc.duplicate_of	= urlidx_rc;

					// We still must store something in the URL idx,
					// otherwise the docids will not be correct.
					do {
						// We will try to add '-'s at the end of the path
						// until we store something
						assert( strlen( path ) < MAX_STR_LEN );
						strcat( path, "_" );
						urlidx_rc = urlidx_resolve_path( urlidx, siteid, path, &(urlidx_docid) );

					} while( urlidx_rc == URLIDX_EXISTENT );
				}

				// Set metadata
				if( opt_import_date && !input_date.eof() ) {
					input_date.getline( line, MAX_STR_LEN );
					doc.last_modified	= atol( line );
				}
				if( opt_import_content_length && !input_content_length.eof() ) {
					input_content_length.getline( line, MAX_STR_LEN );
					doc.content_length	= atol( line );
				}

				// Store
				metaidx_doc_store( metaidx, &(doc) );

			} else {
				cerr << "Problem with line " << line << endl;
				die( "Problem with urlidx_resolve_path" );
			}

			// Increase docid
			if( docid % 10000 == 0 ) {
				cerr << "OK" << endl;
			}
			docid	 ++;

			// Check limits
			assert( urlidx->site_count
					< (siteid_t)((float)CONF_COLLECTION_MAXSITE*URLIDX_MAX_OCCUPANCY) );
			assert( urlidx->path_count
					< (docid_t)((float)CONF_COLLECTION_MAXDOC*URLIDX_MAX_OCCUPANCY) );
		}
	}

	// Read links
	if( opt_import_link ) {
		cerr << endl << "Importing links" << endl;

		char line[MAX_STR_LEN];
		char c = '\0';
		uint adjacency_list_length;
		out_link_t adjacency_list[LINK_MAX_OUTDEGREE];
		uint linelen;
		docid_t src_docid;
		docid_t lineno = 0;

		while( !input_link.eof() ) {
			adjacency_list_length = 0;
			line[0] = '\0';
			linelen	= 0;
			src_docid = 0;
			lineno++;

			// Read character by character, we do this because
			// lines can be very long
			do {

				// Read one character
				input_link.get( c );

				// If this is a space, then it is a next token
				if( c == ' ' || c == '\n' ) {
					// Next token
					line[linelen]	= '\0';

					if( src_docid == 0 ) {
						src_docid	= atol( line );
					} else {
						if( adjacency_list_length < ( LINK_MAX_OUTDEGREE - 2) ) {
							adjacency_list[adjacency_list_length].dest	= (docid_t)(atol(line));
							adjacency_list[adjacency_list_length].tag	= TAG_UNDEF;
							adjacency_list[adjacency_list_length].rel_pos	= 0;
							adjacency_list[adjacency_list_length].anchor_length	= 0;

							adjacency_list_length ++;
						}
					}
					line[0] = '\0';
					linelen	= 0;
				} else {
					// Append to the current token
					line[linelen++]	= c;
				}
			} while( c != '\n' && !input_link.eof() );

			// Store
			if( adjacency_list_length > 0 ) {
				linkidx_status_t rc = linkidx_store( linkidx, src_docid, adjacency_list, adjacency_list_length );
				assert( rc == LINKIDX_OK );
			}
			if( lineno % 10000 == 0 ) {
				cerr << src_docid << " " << adjacency_list_length << " lnk " << endl;
			}
		}
	}

	// Final message
	cerr << "Import OK ***NEXT STEPS: MANDATORY*** " << endl;
	cerr << "% wire-info-analysis --mark-dynamic" << endl;
	cerr << "  For marking static/dynamic URLs" << endl;
	cerr << "% wire-info-analysis --recalculate-depth" << endl;
	cerr << "  For fixing page depths" << endl;
	cerr << "% wire-info-analysis --site-statistics" << endl;
	cerr << "  For fixing site counts" << endl;

	// End
	wire_stop(0);
}

//
// Name: cleanup
//
// Description: 
//   Closes files, indexes
//

void cleanup() {
	if( metaidx != NULL ) {
		metaidx_close( metaidx );
		cerr << "[metaidx] ";
	}
	if( urlidx != NULL ) {
		urlidx_close( urlidx );
		cerr << "[urlidx] ";
	}
	if( linkidx != NULL ) {
		linkidx_close( linkidx );
		cerr << "[linkidx] ";
	}
}

//
// Name: import_usage
//
// Description:
//   Prints an usage message, then stops
//

void import_usage() {
	cerr << "Usage: program [OPTION]" << endl;
	cerr << "Imports data" << endl;
	cerr << "Data is in different files, one URL per file, starting with id=1" << endl;
	cerr << "You must import at least URLs or links" << endl;
	cerr << endl;
	cerr << " --url FILENAME    containing urls, one per line" << endl;
	cerr << " --date FILENAME   containing date, one per line" << endl;
	cerr << " --link FILENAME   containing links, one per line, format:" << endl;
	cerr << "                   src dest1 dest2 dest3 ..." << endl;
	cerr << " --content_length FILENAME   containing size length" << endl;
	cerr << " --help, -h           this help message" << endl;
	cerr << endl;
	wire_stop(0);
}
