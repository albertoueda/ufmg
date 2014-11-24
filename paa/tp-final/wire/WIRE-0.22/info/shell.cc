
#include "shell.h"

// 
// Name: main
//
// Description:
//   Main program for extracting information

int main( int argc, char **argv ) {
	// Check arguments
	if( argc != 1 ) {
		cout << "Usage: " << argv[0] << endl;
		die("Parameters");
	}

	// Init
	wire_start("info" );

	// Open url index
	cerr << "Opening indices in read-only mode: ";
	
	cerr << "urlindex ... ";
	urlidx = urlidx_open( COLLECTION_URL, true );
	assert( urlidx != NULL );

	// Open metaindex
	cerr << "metaindex ...";
	metaidx = metaidx_open( COLLECTION_METADATA, true );
	assert( metaidx != NULL );

	// Open storage
	cerr << "storage ... ";
	storage = storage_open( COLLECTION_TEXT, true );
	assert( storage != NULL );

	// Open linkidx
	cerr << "linkidx ... ";
	linkidx = linkidx_open( COLLECTION_LINK, true );
	assert( linkidx != NULL );

	cerr << "ok." << endl;
	// Help
	cout << endl;
	shell_help();

	// Close all files
	while( !cin.eof() ) {
		char cmdline[MAX_STR_LEN];
		cout << endl << "(info) ";
		cin.getline( cmdline, MAX_STR_LEN );
		if( !strcmp(cmdline, "quit" ) ) {
			break;
		} else if( cmdline != NULL ) {
			shell_parser( cmdline );
		}
	}

	// End
	wire_stop(0);
}

//
// Name: shell_parser
//
// Description:
//   Parses a commandline
//
// Input:
//   cmdline - Command line
//

void shell_parser( char *cmdline ) {
	char *cmd;
	
	cmd = strtok( cmdline, " " );

	if( cmd == NULL || strlen(cmd) == 0 ) {
		return;
	}

	// Check the command
	if( !strcmp( cmd, "?" ) || !strcmp( cmd, "help" ) ) {
		// Help

		cout << "Help:" << endl;
		shell_help();

	} else if( !strcmp( cmd, "urlidx" ) ) {
		// Url index

		cout << "Url index info:" << endl;
		urlidx_dump_status( urlidx );

	} else if( !strcmp( cmd, "harvest" ) ) {
		// Harvest

		cout << "Harvest info:" << endl;
		int harvestid = atoi( strtok( NULL, " " ) );
		assert( harvestid > 0 );

		if( harvest_exists( COLLECTION_HARVEST, harvestid ) ) {
			harvest_t *harvest = harvest_open( COLLECTION_HARVEST, harvestid, true );
			harvest_dump_status( harvest );
			harvest_close( harvest );
		} else {
			cout << "No such harvest" << endl;
		}

	} else if( !strcmp( cmd, "read" ) ) {
		// Content of document

		docid_t docid = atol( strtok( NULL, " " ) );
		assert( docid > 0 );
		storage_status_t rc;
		char *buf;
		buf = (char *)malloc(sizeof(char)*MAX_DOC_LEN);
		assert( buf != NULL );
		off64_t size;

		rc = storage_read( storage, docid, buf, &(size) );
		if( rc != STORAGE_OK ) {
			cout << "Not found" << endl;
		} else {
			cout << "Content of document #" << docid << ", " << size << " bytes" << endl;
			cout << "---START---" << buf << "---END---" << endl;
		}

	} else if( !strcmp( cmd, "linkidx" ) ) {
		// Linkidx info
		cout << "Linkidx info:" << endl;
		linkidx_dump_status( linkidx );

	} else if( !strcmp( cmd, "metaidx" ) ) {
		// Metaidx info
		cout << "Metaidx info:" << endl;
		metaidx_dump_status( metaidx );

	} else if( !strcmp( cmd, "summary" ) ) {
		// Metaidx summary

		cout << "Sites known          : " << metaidx->count_site << endl;
		cout << "Documents known      : " << metaidx->count_doc << endl;

		int harvestid = 1;
		docid_t count_ok			= 0;
		docid_t count_gathered		= 0;

		while( harvest_exists( COLLECTION_HARVEST, harvestid ) ) {
			harvest_t *harvest = harvest_open( COLLECTION_HARVEST, harvestid, true );
			if( harvest->status == STATUS_HARVEST_SEEDED ) {
				count_ok		+= harvest->count_ok;
				count_gathered	+= harvest->count;
			}
			harvest_close( harvest );
			harvestid++;
		}

		cout << "Documents downloaded : " << count_gathered			<< endl;
		cout << "Documents OK         : " << count_ok 	<< endl;

	} else if( !strcmp( cmd, "links" ) ) {
		// Links

		docid_t docid = atol( strtok( NULL, " " ) );
		assert( docid > 0 );

		linkidx_dump_links_with_url( linkidx, urlidx, docid );

	} else if( !strcmp( cmd, "storage" ) ) {
		// Storage info

		cout << "Storage info:" << endl;
		storage_dump_status( storage );

	} else if( !strcmp( cmd, "doc" ) ) {
		// Info for a document

		cout << "Info for document: " << endl;

		// Get docid
		doc_t doc;
		doc.docid = atol( strtok( NULL, " " ) );

		// Retrieve info
		metaidx_status_t rc;
		rc = metaidx_doc_retrieve( metaidx, &(doc) );

		if( rc == METAIDX_OK ) {
			// Dump the doc info
			metaidx_dump_doc_status( &(doc) );

			// Retrieve the path and sitename
			char url[MAX_STR_LEN];
			urlidx_url_by_docid( urlidx, doc.docid, url );
			cout << "Url: " << url << endl;

		} else {
			cout << "Couldn't retrieve document " << doc.docid << endl;
		}

	} else if( !strcmp( cmd, "find" ) ) {
		// Search a document
		doc_field_t 	thefield	= DOC_FIELD_UNDEF;

		char *fieldstr	= strtok( NULL, " " );
		if( !strcmp( fieldstr, "depth" ) ) {
			thefield	= DOC_FIELD_DEPTH;
		} else if( !strcmp( fieldstr, "in_degree" ) ) {
			thefield	= DOC_FIELD_IN_DEGREE;
		} else if( !strcmp( fieldstr, "out_degree" ) ) {
			thefield	= DOC_FIELD_OUT_DEGREE;
		} else if( !strcmp( fieldstr, "http_status" ) ) {
			thefield	= DOC_FIELD_HTTP_STATUS;
		} else {
			cout << "Unrecognized field" << endl;
			cout << "Use: depth, in_degree, out_degree, http_status" << endl;
			return;
		}

		uint thevalue	= atol( strtok( NULL, " " ) );
		doc_t doc;

		cout << "Searching documents:" << endl;

		char url[MAX_STR_LEN];	
		for( doc.docid=1; doc.docid <= metaidx->count_doc; doc.docid++ ) {
			metaidx_doc_retrieve( metaidx, &(doc) );
			if(
			  ( thefield == DOC_FIELD_DEPTH
			    && ( doc.depth == thevalue ) )
			  ||
			  ( thefield == DOC_FIELD_IN_DEGREE
			    && ( doc.in_degree == thevalue ) )
			  ||
			  ( thefield == DOC_FIELD_OUT_DEGREE
			    && ( doc.out_degree == thevalue ) )
			  ||
			  ( thefield == DOC_FIELD_HTTP_STATUS
			    && ( doc.http_status == (int)thevalue ) )
			) {
				urlidx_url_by_docid( urlidx, doc.docid, url );
				cout << " " << doc.docid << " " << url << endl;
			}
		}

	} else if( !strcmp( cmd, "sitedocs" ) ) {
		// Docs for a site
		site_t site;
		site.siteid = shell_get_siteid( strtok( NULL, " " ) );

		if( site.siteid > 0 ) {

			cout << "Documents in site " << site.siteid << " (press Ctrl-C to abort):" << endl;

			char url[MAX_STR_LEN];	
			doc_t doc;
			for( docid_t docid=1; docid <= metaidx->count_doc; docid++ ) {
				doc.docid	= docid;
				metaidx_doc_retrieve( metaidx, &(doc) );
				assert( doc.docid == docid );

				if( doc.siteid == site.siteid ) {
					urlidx_url_by_docid( urlidx, doc.docid, url );
					cout << " " << doc.docid << " " << url << endl;
				}
			}

		}

	} else if( !strcmp( cmd, "sitelinks" ) ) {

		cout << "Links for siteid ";

		siteid_t siteid	= shell_get_siteid( strtok( NULL, " " ) );

		if( siteid > 0 ) {
	        sitelinkidx_t *sitelinkidx = sitelink_load( COLLECTION_SITELINK );
			sitelink_dump_links_with_sitename( sitelinkidx, urlidx, siteid );
			free( sitelinkidx );
		}

	} else if( !strcmp( cmd, "siterevlinks" ) ) {

		cout << "Links pointing to siteid ";
		siteid_t siteid	= shell_get_siteid( strtok( NULL, " " ) );
		char sitename[MAX_STR_LEN];
		cout << siteid << endl;

		if( siteid > 0 ) {
	        sitelinkidx_t *sitelinkidx = sitelink_load( COLLECTION_SITELINK );
			for( siteid_t i=1; i<=sitelinkidx->count_site; i ++ ) {
				sitelink_t *ptr	= sitelinkidx->links[i];
				while( ptr != NULL ) {
					if( ptr->siteid == siteid ) {
						urlidx_site_by_siteid( urlidx, i, sitename );
						cout << " " << i << "(" << ptr->weight << ") " << sitename << endl;
					}
					ptr = ptr->next;
				}
			}
			free( sitelinkidx );
		}

	} else if( !strcmp( cmd, "site" ) ) {
		// Info for a site

		cout << "Info for site: " << endl;
		site_t site;
		site.siteid	= shell_get_siteid( strtok( NULL, " " ) );

		if( site.siteid > 0 ) {

			// Retrieve info
			metaidx_status_t rc;
			rc = metaidx_site_retrieve( metaidx, &(site) );

			if( rc == METAIDX_OK ) {
				// Dump the site info
				metaidx_dump_site_status( &(site) );

				// Retrieve the sitename
				char sitename[MAX_STR_LEN];
				urlidx_site_by_siteid( urlidx, site.siteid, sitename );
				cout << "Site name: " << sitename << endl;

			} else {
				cout << "Couldn't retrieve site " << site.siteid << endl;
			}
		}


	} else {
		cout << "Undefined command: \"" << cmd << "\". Try \"help\"." << endl;
	}
}

//
// Name: shell_get_siteid
//
// Description:
//   Gets a siteid based on a sitename,
//   or converts a siteid from string to siteid_t
//
// Input:
//   sitename - name of the site or siteid as string
//
// Returns:
//   siteid, or 0 if error
//

siteid_t shell_get_siteid( char *sitename ) {
	if( sitename == NULL ) {
		return (siteid_t)0;
	}

	// Get siteid from the string
	siteid_t siteid = atol( sitename );

	// Check
	if( siteid == 0 ) {

		// The string was not a number, resolve
		urlidx_status_t rc = urlidx_resolve_site( urlidx, sitename, NULL );
		if( rc == URLIDX_NOT_FOUND ) {
			cerr << "Site unknown" << endl;
			return (siteid_t)0;

		} else {
			rc = urlidx_resolve_site( urlidx, sitename, &(siteid) );
			assert( rc == URLIDX_EXISTENT );
			return siteid;
		}

	} else {
		return siteid;
	}
}

//
// Name: shell_help
//
// Description:
//   Prints the help message
//

void shell_help() {
	cout << "? -- this message" << endl;
	cout << "summary -- summary information" << endl;
	cout << "urlidx -- info of url index" << endl;
	cout << "harvest <harvestid> -- info of harvest index" << endl;
	cout << "linkidx -- info of link index" << endl;
	cout << "metaidx -- info of metadata index" << endl;
	cout << "storage -- info of storage" << endl;
	cout << "doc <docid> -- info for a document" << endl;
	cout << "read <docid> -- content of a document" << endl;
	cout << "find <depth|in_degree|out_degree|http_status> value -- find docs" << endl;
	cout << "site <siteid|hostname> -- info for a site" << endl;
	cout << "sitedocs <siteid> -- documents on a site" << endl;
	cout << "links <docid> -- links from document" << endl;
	cout << "sitelinks <siteid|hostname> -- links from a website" << endl;
	cout << "siterevlinks <siteid|hostname> -- links to a website" << endl;
	cout << "quit -- exit this program" << endl;
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
	if( storage != NULL ) {
		storage_close( storage );
		cerr << "[storage] ";
	}
	if( linkidx != NULL ) {
		linkidx_close( linkidx );
		cerr << "[linkidx] ";
	}
}
