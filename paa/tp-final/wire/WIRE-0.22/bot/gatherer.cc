
#include "gatherer.h"
#include "xmlconf-main.h"

//
// Name: main
// 
// Description:
//   Gather pages from many harvesters
//

int main( int argc, char **argv ) {
	wire_start( "gatherer" );

	uint FORCED_HARVESTERID = 0;
	while(1) {
		int option_index = 0;
		static struct option long_options[] = {
			{ "help", 0, 0, 0 },
			{ "force", 1, 0, 0 }
		};

		char c = getopt_long( argc, argv, "hf:", long_options, &option_index );

		if( c== -1 ) 
			break;

		switch(c) {
			case 0:
				if( !strcmp( long_options[option_index].name, 
							"force" ) ) {
					FORCED_HARVESTERID = atol( optarg );
				} else if( !strcmp( long_options[option_index].name,
							"help" ) ) {
					gatherer_usage();
				}
				break;
			case 'f':
				FORCED_HARVESTERID = atol(optarg);
				break;
			case 'h':
				gatherer_usage();
				break;
		}
	}


	// Open indexes
	gatherer_open_indexes();

	// Initialize parser
	parser_init();

	// Get memory
	cerr << "Creating buffers ... ";
	char *buf1 = (char *)malloc(sizeof(char)*(MAX_DOC_LEN+MAX_STR_LEN));
	assert( buf1 != NULL );

	char *buf2 = (char *)malloc(sizeof(char)*(MAX_DOC_LEN+MAX_STR_LEN));
	assert( buf2 != NULL );
	cerr << "ok" << endl;
	
	// For each harvesterid that is done
	for( int harvester_id=1; harvest_exists( COLLECTION_HARVEST, harvester_id ); harvester_id++ ) {

		// Abrir
		cerr << "Opening harvester " << harvester_id << " ... ";
		harvest_t *harvester = harvest_open( COLLECTION_HARVEST, harvester_id, true );

		cerr << " checking ... ";
		
		// Check the status of this harvest
		if( harvester->status == STATUS_HARVEST_DONE ||
			(uint)(harvester->id) == FORCED_HARVESTERID ) {
			cerr << "ok" << endl;

			cerr << "Reopening in write mode ... ";
			harvest_close( harvester );
			harvester = harvest_open( COLLECTION_HARVEST, harvester_id, false );
			cerr << "ok" << endl;


			// Gatherer
			gather_harvester( harvester, buf1, buf2 );
			harvester->status = STATUS_HARVEST_GATHERED;

			// Log
			syslog( LOG_NOTICE, "gatherer parsed pages from harvest #%d",
					harvester->id );

			// Close
			harvest_close( harvester );

			// Remove files if necessary. Do not use harvester->id here,
			// as it might be removed
			if( CONF_GATHERER_REMOVESOURCE != 0 ) {
				harvest_remove_files( COLLECTION_HARVEST, harvester_id );
			}

		} else {

			// Ignore
			cerr << "ignored, status = " << HARVEST_STATUS_STR(harvester->status) << endl;
			harvest_close( harvester );
		}
	}

	// Exit
	wire_stop(0);
}

//
// Name: gather_harvester
//
// Description:
//   Gather pages from one harvester
//
// Input
//   harvester - structure with all data for this harvester
//   buf1, buf2 - buffers to use
//

void gather_harvester(harvest_t *harvester, char *buf1, char *buf2) {
	// Get directory name
	char dirname[MAX_STR_LEN];
	sprintf( dirname, "%s/%d", COLLECTION_HARVEST, harvester->id );

	// Open output files for saving links
	cerr << "Opening links output files ... ";
	char links_download_filename[MAX_STR_LEN];
	sprintf( links_download_filename, "%s/%d%s",
		COLLECTION_LINK, harvester->id, FILENAME_LINKS_DOWNLOAD );
	links_download = fopen64(links_download_filename, "w" );
	assert( links_download != 0 );
	cerr << " links_download ok ... " << endl;

	char links_log_filename[MAX_STR_LEN];
	sprintf( links_log_filename, "%s/%d%s",
		COLLECTION_LINK, harvester->id, FILENAME_LINKS_LOG );
	links_log = fopen64(links_log_filename, "w" );
	assert( links_log != 0 );
	cerr << "links_log ok." << endl;

	char links_stat_filename[MAX_STR_LEN];
	sprintf( links_stat_filename, "%s/%d%s",
		COLLECTION_LINK, harvester->id, FILENAME_LINKS_STAT );
	links_stat = fopen64(links_stat_filename, "w" );
	assert( links_stat != 0 );
	cerr << "links_stats ok." << endl;

	// Enter directory
	cerr << "Entering directory: " << dirname << " ... ";
	if( chdir( dirname ) ) {
		cerr << "Couldn't change to directory " <<  endl;
		die("chdir");
	}
	cerr << "ok." << endl;

	// This map has siteids and positions in the site list
	map<siteid_t, uint> map_sites;
	map<siteid_t, bool> site_readed;
	uint NSITES = harvester->count_site;
	assert( NSITES > 0 );
	site_t *sites = (site_t *)malloc(sizeof(site_t)*NSITES);
	assert( sites != NULL );

	// Check site data for correct size
	cerr << "Checking site data ... ";
	struct stat64 buf;
	stat64( ACTIVEPOOL_FILENAME_SITES, &(buf) );
	assert( buf.st_size == (NSITES * sizeof(site_t)) );
	cerr << buf.st_size << " bytes for " << NSITES << " sites ok." << endl;

	// Open site file and read it, 
	// the file is read into the sites[] array, and indexed
	// using map_sites associating a siteid with a position
	// in the array

	cerr << "Loading site data ... ";
	// Open
	FILE *f_sites = fopen64( ACTIVEPOOL_FILENAME_SITES, "r" );
	assert( f_sites != NULL );

	for( uint i=0; i<NSITES; i++ ) {
		// Read the site data
		int rc = fread( &(sites[i]), sizeof(site_t), 1, f_sites );
		assert( rc == 1 );
		siteid_t siteid = sites[i].siteid;
		map_sites[siteid] = i;

		// Mark site as readed, check that it was not marked
		assert( site_readed[siteid] == false );
		site_readed[siteid] = true;
		assert( site_readed[siteid] == true );
	}
	fclose( f_sites );
	cerr << NSITES << " sites readed." << endl;

	// Count the number of fetcher threads, and the size in kilobytes
	unsigned long totalsize = 0;
	int nfetchers = 0;

	gatherer_show_legend();

	while(1) {
		char filename[MAX_STR_LEN];
		struct stat64 statbuf;

		sprintf( filename, ACTIVEPOOL_FILENAME_CONTENT, nfetchers );
		if( stat64(filename,&(statbuf)) ) {
			if( errno == ENOENT ) {
				break;
			} else {
				die("Failed to open output file for fetcher");
			}
		}

		// Add to total size
		totalsize += statbuf.st_size;
		nfetchers++;
	}

	// Complain if 0 fetcher threads

	if( nfetchers == 0 ) {
		cerr << "Warning! files from 0 fetcher threads found" << endl;
		return;
	}
	cerr << "Found " << (nfetchers) << " fetch threads, " << totalsize << " bytes" << endl; 

	// Save raw size
	harvester->raw_size = totalsize;

	// Reset parsed size and count of good pages
	harvester->size     	= 0;
	harvester->count_ok	= 0;

	// Gather pages for one fetcher
	cerr << "Gathering pages:" << endl;
	for(int i=0;i<nfetchers;i++) {
		gather_fetcher(harvester, i, sites, &(map_sites), &(site_readed), buf1, buf2);
	}
	cerr << "Done." << endl;

	// Set harvester status as done
	harvester->status = STATUS_HARVEST_GATHERED;

	// Flush stats
	parser_save_extensions_stats( links_stat );

	// Close files
	fclose( links_download );
	fclose( links_log );
	fclose( links_stat );
	links_download = NULL;
	links_log = NULL;
	links_stat = NULL;

	// Back to old directory
	chdir( CONF_COLLECTION_BASE );

	cerr << "Saving site data ... ";
	for( uint i=0; i<NSITES; i++ ) {
		site_t *site = &(sites[i]);
		assert( site->siteid > 0 );

		// Mark site as unassigned
		site->harvest_id = 0;

		// Mark site as visited
		site->status		= STATUS_SITE_VISITED;

		// Store
		if( metaidx_site_store( metaidx, site )
				!= METAIDX_OK ) {
			die( "metaidx_site_store" );
		}
	}
	cerr << "done." << endl;

	// Free list of sites
	free( sites );
}

//
// Name: gather_fetcher
// 
// Description:
//   Gather pages from one fetcher
//
// Input:
//   harvester - the harvester structure
//   fetcherid - id of the fetcher, in current directory
//   inbuf - buffer for read
//   tmpbuf - temporary buffer
//

void gather_fetcher(harvest_t *harvester, int fetcherid,
        site_t *sites,
        map<siteid_t, uint> *map_sites,
		map<siteid_t, bool> *site_readed,
		char *inbuf, char *outbuf) {

	char filename[MAX_STR_LEN];
    
    //Create the detector
    char *tempbuf  = (char *)malloc(sizeof(char)*(MAX_DOC_LEN+MAX_STR_LEN));
	assert( tempbuf != NULL );
    wireUniversalDetector *detector = new wireUniversalDetector;

	cerr << fetcherid << " ";

	// Open all files (pages, docids, index )

	sprintf( filename, ACTIVEPOOL_FILENAME_CONTENT, fetcherid );
	int f_pages = open( filename, O_RDONLY|O_LARGEFILE );
	if( f_pages <= 0 ) {
		perror( filename );
		die( "unable to open pages file" );
	}

	sprintf( filename, ACTIVEPOOL_FILENAME_DOCS, fetcherid );
	int f_docs = open( filename, O_RDONLY|O_LARGEFILE );
	if( f_docs <= 0 ) {
		perror( filename );
		die( "unable to open docs file" );
	}

	// Start reading docs
	uint ndoc  = 0;
	doc_t doc;

	site_t *site;

	assert( MAX_DOC_LEN > CONF_MAX_FILE_SIZE );
	assert( CONF_GATHERER_MAXSTOREDSIZE > 0 );

	// Read the docid from the docids file
	while( read( f_docs, &(doc), sizeof(doc_t)) == sizeof(doc_t)) {

		docid_t saved_docid = doc.docid;

		assert( doc.docid >= 0 );

		assert( (*site_readed)[doc.siteid] == true );

		uint sitenum = (*map_sites)[doc.siteid];
		assert( sitenum < harvester->count_site );
		site = &(sites[sitenum]);

		if( site->siteid != doc.siteid ) {
			cerr << "Failed to read site data for doc " << doc.docid << " from site " << doc.siteid << endl;
			die( "Read site data" );
		}

		// Check special mime-types; we don't care if it
		// was sucesfull
		if( doc.docid == site->docid_robots_txt ) {
			site->last_checked_robots_txt	= doc.last_visit;
			site->has_valid_robots_txt		= ( HTTP_IS_OK(doc.http_status)
											  || doc.http_status == HTTP_NOT_MODIFIED );

		} else if( doc.docid == site->docid_robots_rdf ) {
			site->last_checked_robots_rdf	= doc.last_visit;
			site->has_valid_robots_rdf		= ( HTTP_IS_OK(doc.http_status)
											  || doc.http_status == HTTP_NOT_MODIFIED );

		}

		// If there is data to parse
		off64_t raw_content_length = doc.raw_content_length;

		if( raw_content_length > 0 ) {

			off64_t readed_bytes;

			// Check if document is too long
			assert( raw_content_length < MAX_DOC_LEN );

			// Read
			readed_bytes = read( f_pages, inbuf, raw_content_length );
			inbuf[raw_content_length]	= '\0';

			if( readed_bytes == raw_content_length ) {

				// See if it is necessary to search links
				if(HTTP_IS_OK(doc.http_status)
						|| HTTP_IS_REDIRECT(doc.http_status)) {
                    charset_t detected_charset = CHARSET_UNKNOWN;

					// Process. File may shrink
					doc.content_length = parser_process( &(doc), inbuf, tempbuf );

					if( doc.content_length > MAX_DOC_LEN ) {
						die( "After parser_process, the document is too large" );
					}

					if( ( doc.mime_type == MIME_TEXT_HTML || doc.mime_type == MIME_TEXT_PLAIN) && doc.content_length > 0 ) {

							//Try to detect the charset
							detected_charset = detector->GetCharset(tempbuf, doc.content_length);
							if (detected_charset != CHARSET_UNKNOWN) {
								doc.charset = detected_charset;
							}
							else {
								doc.charset = http_charset(CONF_GATHERER_DEFAULTCHARSET);
							}

							//Change the CHARSET of the document to UTF-8, File May Grow
							if (CONF_GATHERER_CONVERTTOUTF8 && (doc.charset != CHARSET_UTF_8) ) {
								off64_t converted_length = http_charset_convert(doc.charset, CHARSET_UTF_8, tempbuf, outbuf);
								assert( converted_length < MAX_DOC_LEN ); // Die to avoid memory corruption

								if (converted_length > 0) {
									doc.content_length = converted_length > MAX_DOC_LEN? MAX_DOC_LEN : converted_length; 
									//cout << "FILE SIZE = " << doc.content_length << endl;
									outbuf[doc.content_length] = '\0';
								}
								else {
									/** cout << "Couldn't convert to UTF-8" << endl; **/
									assert( strlen(tempbuf) < MAX_DOC_LEN );
									strcpy(outbuf, tempbuf);
								}
							}
							else {
								/** cout << "Document already was UTF-8" << endl; **/
								assert( strlen(tempbuf) < MAX_DOC_LEN );
								strcpy(outbuf, tempbuf);
							}

					} else {
						assert( strlen(tempbuf) < MAX_DOC_LEN );
						strcpy( outbuf, tempbuf );
					}

                    if( doc.mime_type == MIME_ROBOTS_TXT ) {
						// Content of robots is not stored, only links
						// The content_length tells if there are exclusions
						if( doc.content_length > 0 ) {
							cerr << "(robots.txt w/exclusions)";
						} else {
							cerr << "(robots.txt)";
						}

					} else if( doc.mime_type == MIME_ROBOTS_RDF ) {
						// Content of robots is not stored, only links
						// The content_length tells if there are exclusions
						if( doc.content_length > 0 ) {
							cerr << "(robots.rdf w/information)";
						} else {
							cerr << "(robots.rdf)";
						}

					} else if( HTTP_IS_REDIRECT(doc.http_status) ) {
						// Content of redirect is not stored, only links
						cerr << "^";


					} else {

						// Update harvester
						harvester->count_ok ++;

						if( doc.content_length > 0 ) {

							// Check maximum stored size
							if( doc.content_length > CONF_GATHERER_MAXSTOREDSIZE ) {
								doc.content_length	= CONF_GATHERER_MAXSTOREDSIZE;
								outbuf[CONF_GATHERER_MAXSTOREDSIZE] = '\0';

							}

							// If necessary, store only the hash function
							if( CONF_GATHERER_SAVEHASHONLY ) {
								md5_string( outbuf, doc.content_length, tempbuf );
								assert( strlen(tempbuf) < MAX_STR_LEN );
								strcpy( outbuf, tempbuf );
								doc.content_length = strlen(outbuf);

							}

							// Store content
							storage_status_t rcstorage = storage_write( storage, doc.docid, outbuf, doc.content_length, &(doc.hash_value), &(doc.duplicate_of) );	
							
							// Compute and store also LSH-sketch (optional)
							if(CONF_GATHERER_USESKETCHES) {
							  IrudikoSimpleReader iread;
							  irudiko_sketch_t ist;
							  iread.process_and_sketch(outbuf,doc.content_length, ist.sketch, IRUDIKO_SKETCHSIZE, IRUDIKO_SELECTIONTYPE, IRUDIKO_SELECTIONPARAM);
							  storage_status_t rc2 = storage_write_sketch(storage,doc.docid,&ist);
							}
							switch( rcstorage ) {

								case STORAGE_OK:

									// Update size of harvester
									// this includes only new bytes
									// downloaded and saved after parsin
									harvester->size += doc.content_length;

									cerr << ".";
									break;

								case STORAGE_DUPLICATE:
									// It's a duplicate
									cerr << "=";
									break;

								case STORAGE_UNCHANGED:
									// The server says it was modified, but
									// the content is the same.
									cerr << "/";
									assert( doc.number_visits_changed > 0 );
									doc.number_visits_changed--;
									break;

								default:
									die( "Inconsistency in return of storage_write" );
							}
						}

					}
				}
				else
				{
					// Neither OK nor redirect
					cerr << "x";

				}

			} else {
				perror( "read( content )" );
				cerr << "Failed in harvester " << harvester->id
					<< ", fetcher " << fetcherid << endl;
				cerr << "Expected " << raw_content_length << endl;
				cerr << "Got " << readed_bytes << endl;
				//die("Failed to read crawler content");
			}

		// Empty page
		} else {
			cerr << "_";
		}

		// Store the corresponding document
		doc.status = STATUS_DOC_GATHERED;

		// If the document changes too often, mark as dynamic
		if( !doc.is_dynamic
				&& (doc.number_visits == doc.number_visits_changed)
				&& (doc.number_visits > CONF_GATHERER_CHANGETODYNAMIC) ) {
			doc.is_dynamic = true;

		}

		assert( doc.docid > 0 );
		assert( doc.docid == saved_docid );
		if( metaidx_doc_store( metaidx, &(doc) ) != METAIDX_OK ) {
			die( "metaidx_doc_store" );
		}

		// Count page
		ndoc++;
	}

	// Note that this count may include robots_txt files.
	cerr << " " << ndoc << " pages" << endl;

    free(tempbuf);

	close( f_pages );
	close( f_docs );
}

//
// Name: gatherer_open_indexes
//
// Description:
//   Opens all indexes uses by this program
//

void gatherer_open_indexes() {
	cerr << "Opening indexes ... ";

	cerr << "[storage] ";
	storage = storage_open(COLLECTION_TEXT, false);
	assert( storage != NULL );

	cerr << "[metadata] ";
	metaidx = metaidx_open( COLLECTION_METADATA, false );
	assert( metaidx != NULL );

	cerr << "done." << endl;
}

//
// Name: cleanup()
//
// Description:
//   Closes all indexes and files
//
 

void cleanup() {
	chdir( CONF_COLLECTION_BASE );
	if( storage != NULL ) {
		storage_close( storage );
		cerr << "[storage] ";
	}
	if( metaidx != NULL ) {
		metaidx_close( metaidx );
		cerr << "[metaidx] ";
	}
	if( links_download != NULL ) {
		fclose( links_download );
		cerr << "[newurls] ";
	}
	if( links_log != NULL ) {
		fclose( links_log );
		cerr << "[links_log] ";
	}
}

//
// Name: gatherer_usage()
//
// Description:
//   Shows the usage message
//

void gatherer_usage() {
	cerr << "Usage: program [OPTION]" << endl;
	cerr << "Gathers all pages fetched by a harvester, and adds" << endl;
	cerr << "them to the collection." << endl;
	cerr << endl;
	cerr << " -f, --force id       forces it to a fixed harvest_id" << endl;
	cerr << " -h, --help           this help message" << endl;
	cerr << endl;
	wire_stop(0);
}

void gatherer_show_legend() {
	cerr << "------------------ LEGEND ---------------------" << endl;
	cerr << " . OK         ^ Redirect     x Error           " << endl;
	cerr << " = Duplicate  _ Unchanged    _ Empty           " << endl;
	cerr << "-----------------------------------------------" << endl;
}
