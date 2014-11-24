 
#include "seeder.h"

//
// Name: main
//
// Description:
//   Main cycle for the seeder program
//
// Input:
//   argv[1] - filename to read; the file must have one url per line,
//             possibly prepended by a number to indicate its depth
//

char opt_start_urls[MAX_STR_LEN]	= "";

int main( int argc, char **argv ) {
	// Init the program
	wire_start( "seeder" );

	// Parse options
	bool opt_force_start = false;
	int opt_harvester_id = 0;
	
	while(1) {
		int option_index = 0;

		static struct option long_options[] = {
			{"help", 0, 0, 0},
			{"start", 1, 0, 0},
			{"force", 1, 0, 0},
			{0, 0, 0, 0}
		};

		char c = getopt_long (argc, argv, "hsf:",
				long_options, &option_index );
		
		if( c == -1)
			break;

		switch (c) {
			case 0:
				if( !strcmp( long_options[option_index].name,
							"start" ) ) {
					opt_force_start = true;
					strcpy( opt_start_urls, optarg );
				} else if( !strcmp( long_options[option_index].name, "force" ) ) {
					opt_harvester_id = atol( optarg );
				} else if( !strcmp( long_options[option_index].name,
							"help" ) ) {
					seeder_usage();
				}
				break;
			case 's':
				opt_force_start = true;
				break;
			case 'f':
				opt_harvester_id = atol( optarg );
				break;
		}
	}

	// Initialize maps
	seeder_init_maps();

	// Open url index
	seeder_open_indexes();

	// Check if the metaidx is empty
	if( metaidx->count_doc == 0 && ( !opt_force_start ) ) {
		die( "Metaindex is empty, use --start to load starting urls" );
	}

	if( opt_force_start ) {

		seeder_process_harvest(NULL);
		syslog( LOG_NOTICE, "seeder readed starting urls" );

	} else {

		// Iterate through harvests
		for( int i=1; harvest_exists(COLLECTION_HARVEST, i ); i++ ) {
			harvest_t *harvester = harvest_open( COLLECTION_HARVEST, i, true );
			assert( harvester != NULL );

			// Report
			cerr << "Checking harvester #" << harvester->id << " ... ";

			// Check status
			if( harvester->status == STATUS_HARVEST_GATHERED ||
			    harvester->id == opt_harvester_id ) {
				cerr << "ok" << ( harvester->id == opt_harvester_id ? " [forced]" : "") << endl;

				cerr << "Reopening in write mode ... ";
				harvest_close( harvester );
				harvester = harvest_open( COLLECTION_HARVEST, i, false );

				// Process
				seeder_process_harvest(harvester);

				// Add info about the links
				harvester->speed_ok		= (double)harvester->count_ok
					            / (double)(harvester->endtime - harvester->begintime);

				harvester->links_ratio	= (double)harvester->link_new_pages
					                / (double)(harvester->link_new_pages + harvester->link_old_pages);

				// Dump status
				harvest_dump_status( harvester );

				// Log
				syslog( LOG_NOTICE, "seeder readed links from harvest %d",
						harvester->id );
			} else {
				cerr << "skiped, status=" << HARVEST_STATUS_STR(harvester->status) << endl;
			}

			// Close
			harvest_close( harvester );

		}
	}

	metaidx_dump_status( metaidx );

	// End
	wire_stop(0);
}

//
// Name: seeder_process_harvest
//
// Description:
//   Extracts unseen urls from the links of a harvest
//
// Input:
//   harvester - the harvester to be checked, or null if
//               we must use the start urls.
//

void seeder_process_harvest( harvest_t *harvest ) {
	// Files
	FILE *input_file;
	FILE *output_file;
	char input_filename[MAX_STR_LEN];
	char output_filename[MAX_STR_LEN];

	if( harvest != NULL ) {
		assert( harvest->status == STATUS_HARVEST_GATHERED ||
			harvest->status == STATUS_HARVEST_SEEDED );
	}

	// Create filenames
	if( harvest != NULL ) {
		sprintf( input_filename, "%s/%d%s", COLLECTION_LINK, harvest->id, FILENAME_LINKS_DOWNLOAD );
		sprintf( output_filename, "%s/%d%s", COLLECTION_LINK, harvest->id, FILENAME_RESOLVED_LINKS );
	} else {
		assert( strlen( opt_start_urls ) > 0 );
		sprintf( input_filename, "%s", opt_start_urls );
		sprintf( output_filename, "/dev/null" );
	}

	// Open input file
	cerr << "Opening '" << input_filename << "' ...";
	input_file = fopen64( input_filename, "r" );
	if( !input_file ) {
		perror( input_filename );
		die( "opening input file" );
	}
	cerr << "done." << endl;

	// Open output file
	output_file = fopen64( output_filename, "w" );

	if( !output_file ) {
		perror( output_filename );
		die( "opening output file" );
	}

	// Show legend
	seeder_show_legend();

	// Line counter, for reporting
	int line_number = 0;
	char *line;
	line = (char *)malloc(sizeof(char)*MAX_STR_LEN*3);
	assert( line != NULL );

	// Outlink_list
	uint adjacency_list_length = 0;
	out_link_t adjacency_list[LINK_MAX_OUTDEGREE];
	

	// Source document
	doc_t	src_doc;
	src_doc.docid = (docid_t)0;
	src_doc.depth = 0;
	char src_path[MAX_STR_LEN];
	docid_t src_docid;

	// Siteid cache
	site_t site_cache;
	site_cache.siteid = 0;
	size_t linelen = 0;

	// Readed url
	char url[MAX_STR_LEN];
	char caption[MAX_STR_LEN];
	char path_buffer[MAX_STR_LEN];

	//Readed link info
	int rel_pos;
	int tag;
	int caption_length;

	// Read lines from input file
	while( getline( &line, &linelen, input_file ) != -1 ) {
		// Report
		if( (line_number++ % SEEDER_PAGE_WIDTH) == 0 ) {
			cerr << endl << line_number << " ";
		}

		// Note in harvest
		if( harvest != NULL ) {
			harvest->link_total ++;
		}

		// Format of input lines: (docid link caption)

		url[0] = '\0';
		caption[0] = '\0';
		src_docid = 0;

		if( ! (
			   (sscanf( line, "%lu %d %d %s %[^\n]", (docid_t *)&(src_docid), &rel_pos, &tag, url, caption ) == 5 ) 
			|| (sscanf( line, "%lu %d %d %s", (docid_t *)&(src_docid), &rel_pos, &tag, url ) == 4 ) 
			|| (sscanf( line, "%s %s", url, caption ) == 2 ) 
			|| (sscanf( line, "%s", url ) == 1 ) 
			 )) {

			// Malformed
			cerr << "!";
			continue;
		}
		caption_length = strlen(caption);

		// Check length of everything
		if( strlen(url) >= MAX_STR_LEN || strlen(caption) >= MAX_STR_LEN ) {
			cerr << "!";
			continue;
		}

		// Check URL for strange characters, all the
		// URL at least has to be printable
		if( has_nonprintable_characters(url) ) {
			cerr << "!";
			continue;
		}

		// Check if the source docid changed
		if( src_docid != src_doc.docid ) {

			// It's a different docid, save current adjacency list
			// if the list has elements.
			if( adjacency_list_length > 0 ) {

				// It has elements, save
				seeder_save_links( &(src_doc), adjacency_list, adjacency_list_length );

				// Releasing memory used by the adjacency list
				//for(uint i=0; i<adjacency_list_length; i++)
				//	free(adjacency_list[i]);

				// Clean adjacency list
				adjacency_list_length = 0;
			}

			// Retrieve the new src_doc
			src_doc.docid = src_docid;

			// Check if it's 0
			if( src_doc.docid != (docid_t)0 ) {

				// It's a real docid, not a 0
				metaidx_status_t rc = metaidx_doc_retrieve( metaidx, &(src_doc) );

				if( rc != METAIDX_OK ) {
					cerr << "DocID " << src_docid << " not found!" << endl;
					cerr << "Offending line: " << line << endl;
					die( "link from unknown document seen" );
				}

				// Retrieve the new src_doc path
				urlidx_path_by_docid( urlidx, src_doc.docid, src_path );
				assert( src_path != NULL );

				// Sometimes there is an inconsistency
				// (can't find it yet) that puts a 
				// leading '/' on the path_file
				if( src_path[0] == '/' ) {
					cerr << "!";
					continue;
				}

			} else {

				// It's a 0 docid
				src_path[0] = '\0';

			}

		}

		// Check depth
		if( ( src_doc.docid > 0 ) &&
			(src_doc.depth >= ( src_doc.is_dynamic
				  ? CONF_MANAGER_MAXDEPTH_DYNAMIC 
				  : CONF_MANAGER_MAXDEPTH_STATIC )) ) {

			cerr << "H";
			continue;
		}

		// Check if it's not a duplicate
		if( ( src_doc.docid > 0 ) && ( src_doc.duplicate_of != (docid_t)0 ) ) 
		{
			// We don't include pages from duplicate documents
			cerr << "C";
			continue;
		}


		// Resolve the link
		docid_t dest = seeder_resolve_link( harvest, &(src_doc), src_path, url, caption, path_buffer, &site_cache );

		// See if it was resolved
		if( (src_doc.docid > (docid_t)0) && (dest > (docid_t)0) ) {
			// Write result
			if( caption[0] != '\0' ) {
				fprintf( output_file, "%lu %s\n", dest, caption );
			}

			// Append to adjacency list; we will check if the
			// link already exists, or if its a self-link
			if( adjacency_list_length < (LINK_MAX_OUTDEGREE - 1) ) {

				if( src_doc.docid != dest ) {
					// This is not a self link
					// See if this is a repeated link
					bool is_repeated	= false;
					for( uint i=0; i<adjacency_list_length; i++ ) {
						if( adjacency_list[i].dest == dest ) {
							is_repeated	= true;
						}
					}

					if( ! is_repeated ) {
						// This is not a repeated link
						adjacency_list[adjacency_list_length].dest = dest;
						adjacency_list[adjacency_list_length].rel_pos = (char)rel_pos;
						adjacency_list[adjacency_list_length].tag = (char)tag;
						adjacency_list[adjacency_list_length].anchor_length = caption_length;
						adjacency_list_length++;
					}
				}


			}
		}

	} // Next line

	// If there are still elements to save, save them
	if( adjacency_list_length > 0 ) {
		seeder_save_links( &(src_doc), adjacency_list, adjacency_list_length );
		adjacency_list_length = 0;
	}

	// Report number of lines parsed
	cerr << " " <<  --line_number << " done." << endl;

	free(line);

	// Mark as seeded
	if( harvest != NULL ) {
		harvest->status = STATUS_HARVEST_SEEDED;
	}

	// Close
	fclose( input_file );
	fclose( output_file );

	// We could discard here 'input_filename' using unlink(), but we will need it for the analysis program :-(
}

//
// Name: seeder_init_maps
// 
// Description:
//   Initialize a series of maps used by the seeder parser
//   These maps are:
//    accept_protocol - Protocols accepted (ie: html)
//    extensions_ignores - Known non-text extensions (ie: jpg)
//

void seeder_init_maps() {
	cerr << "Initializing ... ";

	cerr << "[accept_protocol] ";
	accept_protocol.check_matches = true;
	perfhash_create( &(accept_protocol), CONF_SEEDER_LINK_ACCEPT_PROTOCOL );

	cerr << "[extensions_ignore] ";
	extensions_ignore.check_matches = true;
	perfhash_create( &(extensions_ignore), CONF_SEEDER_LINK_EXTENSIONS_IGNORE );

	cerr << "[dynamic extension] ";
	extensions_dynamic.check_matches = true;
	perfhash_create( &(extensions_dynamic), CONF_SEEDER_LINK_DYNAMIC_EXTENSION );

	cerr << "[domain suffixes] ";
	domain_suffixes = string_list_create( CONF_SEEDER_LINK_ACCEPT_DOMAIN_SUFFIXES );

	cerr << "[reject patterns] ";
	tokenizeToRegex( CONF_SEEDER_REJECTPATTERNS, &reject_patterns );

	cerr << "[sessionid variables] ";
	char sessionids_cpy[MAX_STR_LEN];
	assert( strlen(CONF_SEEDER_SESSIONIDS) < MAX_STR_LEN );
	strcpy( sessionids_cpy, CONF_SEEDER_SESSIONIDS );
	tokenizeToRegex( sessionids_cpy, &sessionid_patterns );
	sessionid_variables = tokenizeToTable( CONF_SEEDER_SESSIONIDS, &(sessionid_variables_count) );

	cerr << "done." << endl;
}

//
// Name: seeder_show_legend
//
// Description:
//   Prints the legend of the seeder to stderr
//

void seeder_show_legend() {
	cerr << "------------------ LEGEND ----------------------" << endl;
	cerr << "ACCEPT:   . new document    + new site          "	<< endl;
	cerr << "REJECT:   _ seen            ! malformed         " << endl;
	cerr << "          P Pattern         D Domain            "		<< endl;
	cerr << "          E Extension       H protocol not Http "	<< endl;
	cerr << "          H Depth           C link from Copy    "  << endl;
	cerr << "          M too Many pages                      " << endl;
	cerr << "------------------------------------------------" << endl;
}

//
// Name: seeder_resolve_link
//
// Description:
//   Resolves a link to a document id
//
// Input:
//   harvest - the harvester where the link appears
//   src_doc - source document
//   url - string of the link
//   caption - description of the link
//   path - memory area to save the path
//

docid_t seeder_resolve_link( harvest_t *harvest, doc_t *src_doc, char *src_path, char *url, char *caption, char *path, site_t *site ) {
	assert( path != NULL );

	// Check if we have room for new documents
	if( urlidx->path_count >=
	   (docid_t)((float)CONF_COLLECTION_MAXDOC*URLIDX_MAX_OCCUPANCY)) {
		cerr << "M";
		return (docid_t)0;
	}

	// Urlindex variables
	urlidx_status_t rc;

	// Doc and siteid
	// For the sites we use a cache of one element (the last site)
	docid_t docid;
	bool is_new_site = false;
	siteid_t siteid;

	// Check if the url includes the '%' character, urls are stored unescaped
	if( strchr( url, '%' ) ) {
		unescape_url( url );
	}

	// Delete newlines, these can be escaped using, e.g.: %0A,
	// so they won't be detected by the gatherer
	if( char *ptr = strchr( url, '\n' ) ) {
		(*ptr) = '\0';
	}
	if( char *ptr = strchr( url, '\r' ) ) {
		(*ptr) = '\0';
	}

	assert( ! (strchr( url, '\n' ) || strchr( url, '\r' )) );

	// Check if the url has protocol
	if( ! strchr( url, ':' ) ) {

		if( src_doc->docid == 0 ) {
			cerr << endl;
			cerr << "*** Warning! Relative url without source docid: '" << url << "'" << endl;

			// This can be more serious; abort here
			die( "The seeder is reading a file that has errors" );
		}

		// It doesn't have protocol, it's a local url
		// from the same site
		siteid = src_doc->siteid;

		// Check if the link starts with a '/'
		if( url[0] == '/' ) {

			// It's an absolute url, copy as-is
			assert( strlen(url) < MAX_STR_LEN );
			strcpy( path, url );

		} else {

			// Relative URL, this have to be converted
			urlidx_relative_path_to_absolute( src_path, url, path );
		}

	} else {
		// Remote url
		// It's a url that has protocol

		// Parser variables
		char protocol[MAX_STR_LEN];
		char sitename[MAX_STR_LEN];

		// Parse the input url
		if( !urlidx_parse_complete_url( url, protocol, sitename, path ) ) {

			// Malformed
			cerr << "!";
			return (docid_t)0;
		}

		// Check the protocol
		if( ! perfhash_check( &(accept_protocol), protocol) ) {

			// Protocol not accepted
			cerr << "P";
			return (docid_t)0;
		}

		// Check for sitenames that are too short
		if( strlen(sitename) < 3 ) {
			cerr << "!";
			return (docid_t)0;
		}

		// Convert the sitename to lowercase
		for( uint i = 0; i < strlen(sitename); i++ ) {
			sitename[i] = tolower( sitename[i] );
			if( !isalnum(sitename[i]) && sitename[i] != '.'
					&& sitename[i] != '-' && sitename[i] != '_' ) {

				// Site name not acceptable
				cerr << "!";
				return (docid_t)0;
			}
		}

		// Check if the site has a normal tld
		char top_level_domain[MAX_STR_LEN];
		urlidx_get_lowercase_extension(sitename, top_level_domain);
		// Check if it has a TLD
		if( strlen(top_level_domain) == 0 ) {
			// Skip: no TLD
		   cerr << "!";
		   return (docid_t)0;
		}

		// Why don't we check for the TLD here?
		// Because the site may not match the tld (for instance, if .none
		// was given), but the linked site may be in the list of starting seeds!
		// And in that case we want to save the link anyways
 
		// Check if the site is not known
		rc = urlidx_resolve_site( urlidx, sitename, NULL );
		if( rc == URLIDX_NOT_FOUND ) {

			// It is a new site
			assert( strlen(sitename) > 0 );
			if( harvest != NULL ) {
				// If harvest == NULL then we're reading the starting
				// URLs, in that case we don't look at the domain_suffixes,
				// we accept everything
				// In this case the harvest is not null, let's see if
				// the site has an acceptable pattern
				if( ! string_list_suffix( domain_suffixes, sitename ) ) {
					// No, it's outside our domain suffixes
					cerr << "D";
					return (docid_t)0;
				}
			}

			// Check if there are too many sites
			if( urlidx->site_count >=
			   (siteid_t)((float)CONF_COLLECTION_MAXSITE*URLIDX_MAX_OCCUPANCY)) {
				cerr << "M";
				return (docid_t)0;
			} else if( urlidx->path_count >=
			   (docid_t)((float)(CONF_COLLECTION_MAXDOC-2)*URLIDX_MAX_OCCUPANCY)) {
				// A new site could require more documents
				// to be added, check this condition here
				cerr << "M";
				return (docid_t)0;
			}
		}

		// Get the siteid, or create the site
		rc = urlidx_resolve_site( urlidx, sitename, &(siteid) );

		if( rc == URLIDX_CREATED_SITE ) {
			assert( siteid > 0 );

			// It's a new site
			is_new_site = true;

			// Add metadata for this new site
			site_t newsite;
			metaidx_site_default( &(newsite) );
			newsite.siteid = siteid;
			newsite.count_doc  = 0;

			// Add IP information if possible
			if( strlen(caption) > 10 ) { // at least IP=1.1.1.1
				if( caption[0] == 'I' 
				 && caption[1] == 'P'
				 && caption[2] == '=' ) {
					int rc = inet_aton( (char *)(caption+3), &(newsite.addr) );
					if( rc == 0 ) {
						cerr << "(wrong IP)";
					} else {
						time_t now = time(NULL);
						newsite.last_resolved = now;
					}
				}
			}
			metaidx_site_store( metaidx, &(newsite) );

			// This is the first time this site is seen, check if it's
			// necessary to add other URLs.
			
			// Add home page
			if( CONF_SEEDER_ADDHOMEPAGE == 1 && strlen(path) != 0 ) {

				// Create new document
				doc_t doc;
				metaidx_doc_default( &(doc) );

				// "/" was added, get docid
				rc = urlidx_resolve_path( urlidx, newsite.siteid, "", &(doc.docid) );
				assert( rc == URLIDX_CREATED_PATH );
				assert( doc.docid > src_doc->docid );

				// Save metadata
				doc.siteid = newsite.siteid;
				doc.depth  = 1;
				metaidx_doc_store( metaidx, &(doc) );

				// Count this document
				newsite.count_doc	++;
				metaidx_site_store( metaidx, &(newsite) );

			}

			// Add robots.txt
			if( CONF_SEEDER_ADD_ROBOTSTXT == 1
				&& strcmp(path, FILENAME_ROBOTS_TXT) ) {

				// Create new document
				doc_t doc;
				metaidx_doc_default( &(doc) );

				// "/" was added, get docid
				rc = urlidx_resolve_path( urlidx, newsite.siteid, FILENAME_ROBOTS_TXT, &(doc.docid) );
				assert( rc == URLIDX_CREATED_PATH );
				assert( doc.docid > src_doc->docid );

				// Save metadata
				doc.siteid = newsite.siteid;
				doc.depth  = 1;
				doc.mime_type	= MIME_ROBOTS_TXT;
				metaidx_doc_store( metaidx, &(doc) );

				// Mark this document as the robots.txt file
				newsite.docid_robots_txt	= doc.docid;

				// Count this document
				newsite.count_doc	++;
				metaidx_site_store( metaidx, &(newsite) );

			}
		
			// Add robots.rdf
			if( CONF_SEEDER_ADD_ROBOTSRDF == 1
				&& strcmp(path, FILENAME_ROBOTS_RDF) ) {

				// Create new document
				doc_t doc;
				metaidx_doc_default( &(doc) );

				// "/" was added, get docid
				rc = urlidx_resolve_path( urlidx, newsite.siteid, FILENAME_ROBOTS_RDF, &(doc.docid) );
				assert( rc == URLIDX_CREATED_PATH );
				assert( doc.docid > src_doc->docid );

				// Save metadata
				doc.siteid		= newsite.siteid;
				doc.mime_type	= MIME_ROBOTS_RDF;
				doc.depth  = 1;
				metaidx_doc_store( metaidx, &(doc) );

				// Mark this document as the robots.txt file
				newsite.docid_robots_rdf	= doc.docid;

				// Count this document
				newsite.count_doc	++;
				metaidx_site_store( metaidx, &(newsite) );

			}
		
		}

	}

	// In this point we have a path and a siteid
	assert( siteid > 0 );
	assert( siteid < CONF_COLLECTION_MAXSITE );
	assert( path != NULL );

	// Retrieve the site. We have to do this here to
	// check if the website has too many urls already;
	// because after we have called urlidx resolve path,
	// we MUST store a document in the metaidx to be
	// consistent.
	metaidx_status_t rc_metaidx;

	if( siteid == site->siteid ) {
		// It is the same as the last one, we use the cache
		rc_metaidx = METAIDX_OK;

	} else {
		// It is a new one
		site->siteid = siteid;
		rc_metaidx = metaidx_site_retrieve( metaidx, site );
		assert( rc_metaidx == METAIDX_OK );

	}

	// Check how many documents has this site
	// we do this before starting parsing the path to save time
	if( site->count_doc >= CONF_SEEDER_MAX_URLS_PER_SITE ) {
		cerr << "M";
		return (docid_t)0;
	}

	// Remove (potential) leading slash from path
	if( path[0] == '/' ) {

		// Copy to auxiliary var
		char pathcpy[MAX_STR_LEN];
		assert( strlen(path) < MAX_STR_LEN );
		strcpy( pathcpy, path );

		// Skip slashes
		uint last_slash = 0;
		while( last_slash < strlen(url) &&
			url[last_slash] == '/' ) {
			last_slash++;
		}

		// copy back
		assert( last_slash > 0 );
		assert( strlen(pathcpy + last_slash) < MAX_STR_LEN );
		strcpy( path, pathcpy + last_slash );
	}

	// Remove trailing slash from exclusion paths
	if( !strcmp(caption,LINK_CAPTION_FORBIDDEN) ) {
		if( strlen(path) > 0 && path[strlen(path)-1] == '/' ) {
			path[strlen(path)-1] = '\0';
		}
	}

	// Reject certains paths
	if( regexec( &reject_patterns, path, 0, NULL, 0 ) == 0 ) {
		cerr << "P";
		return (docid_t)0;
	}

	// Remove sessionids in URLs
	if( regexec( &sessionid_patterns, path, 0, NULL, 0 ) == 0 ) {
		for( int i=0; i<sessionid_variables_count; i++ ) {
			urlidx_remove_variable( path, sessionid_variables[i] );
		}
	}

	// Remove sessionids, heuristic
	urlidx_remove_sessionids_heuristic( path );

	// Sanitize the URL
	urlidx_sanitize_url( path );

	// Resolve
	assert( path[0] != '/' );
	rc = urlidx_resolve_path( urlidx, siteid, path, &(docid) );

	// New URL
	if( rc == URLIDX_CREATED_PATH ) {
		assert( docid > src_doc->docid );

		// Increase siteid
		site->count_doc++;
		assert( site->count_doc > 0 );

		// Store site
		assert( site->siteid > 0 );
		assert( site->siteid < CONF_COLLECTION_MAXSITE );
		metaidx_site_store( metaidx, site );

		// Create a new record
		doc_t doc;
		metaidx_doc_default( &(doc) );

		// Data
		doc.is_dynamic = urlidx_is_dynamic( &(extensions_dynamic), path );
		doc.docid  = docid;		// Returned by urlidx resolve_path
		doc.siteid = siteid;
		doc.depth  = src_doc->depth + 1;

		// Check robots.txt information
		// Exclusion paths are reported by the gatherer
		// as a link with a special caption = LINK_CAPTION_FORBIDDEN
		// e.g.:
		//    cgi-bin/ <FORBIDDEN>
		// means that all the content of that directory must be
		// kept off-limits.

		if( !strcmp(caption,LINK_CAPTION_FORBIDDEN) ) {

			assert( src_doc->mime_type == MIME_ROBOTS_TXT );

			// This is an exclusion path, that was
			// readed from a robots.txt file
			// Mark this document as excluded
			doc.status			= STATUS_DOC_EXCLUSION;
			site->has_valid_robots_txt	= true;

		} else if( site->has_valid_robots_txt && seeder_is_excluded( &(doc), path ) ) {
			// This document must be excluded, because
			// its a sub-directory of an exclusion path
			doc.status = STATUS_DOC_EXCLUSION;
		}

		// Store
		metaidx_doc_store( metaidx, &(doc) );

		if( is_new_site ) {
			cerr << "+";

			if( harvest != NULL ) {
				harvest->link_new_sites ++;
			}
		} else {
			cerr << ".";
		}

		if( harvest != NULL ) {
			harvest->link_new_pages ++;
		}

		// Copy docid
		docid = doc.docid;

	} else if( rc == URLIDX_EXISTENT ) {

		// Suppose we have A->B, and B exists.
		// Here we could update the depth of B, if it have
		// become smaller (i.e. we had find a shortest route),
		// BUT in that case we should
		// also update the depth of the pages that are
		// pointed by B, so we prefer not to do this.

		// The analysis program has an option for fixing depths

		// Report as 'seen'
		cerr << "_";

		// Maybe this was an exclusion path for a URL we've
		// already seen
		if( !strcmp(caption,LINK_CAPTION_FORBIDDEN) ) {

			assert( src_doc->mime_type == MIME_ROBOTS_TXT );

			// This is an exclusion path, that was
			// readed from a robots.txt file, but the URL
			// has been seen before
			doc_t doc;
			doc.docid	= docid;
			metaidx_doc_retrieve( metaidx, &(doc) );
			assert( doc.docid == docid );

			// Mark this document as excluded
			doc.status			= STATUS_DOC_EXCLUSION;
			site->has_valid_robots_txt	= true;
			metaidx_doc_store( metaidx, &(doc) );
		}

		if( harvest != NULL ) {
			harvest->link_old_pages ++;
		}

	} else {
		die( "Unexpected response from urlidx resolver" );
	}

	// Check robots.rdf information, in this case, we use
	// a special caption for last modified date.
	// e.g.:
	//    news/new_item.html <LAST-MODIFIED> 1079473183
	// In which the second number is the last modified date
	// informed in the robots.txt file
	if( strlen(caption) > strlen(LINK_CAPTION_LAST_MODIFIED)
	 && !strncmp(caption,LINK_CAPTION_LAST_MODIFIED,strlen(LINK_CAPTION_LAST_MODIFIED)) ) {
		doc_t doc;
		doc.docid	= docid;

		metaidx_doc_retrieve( metaidx, &(doc) );

		// Advance special caption "<LAST-MODIFIED>"
		caption += strlen(LINK_CAPTION_LAST_MODIFIED);

		// Advance whitespace
		while(isspace(*caption)) {
			caption++;
		}

		// Copy date
		doc.last_modified	= atol(caption);

		// Save
		metaidx_doc_store( metaidx, &(doc) );
	}



	return docid;
}

//
// Name: seeder_save_links
//
// Description:
//   Saves all of the urls from a document
//
// Input:
//   src_doc - source document
//   adjacency_list - list of outgoing links
//   adjacency_list_length - length of adjacency_list
//   

void seeder_save_links( doc_t *src_doc, out_link_t adjacency_list[], int adjacency_list_length ) {
	// Save
	linkidx_status_t rc = linkidx_store( linkidx, src_doc->docid, adjacency_list, adjacency_list_length );
	if( rc != LINKIDX_OK ) {
		cerr << "[Failed to store links from " << src_doc->docid << "]" << endl;
	}

	return;
}

//
// Name: seeder_is_excluded
//
// Description:
//   Checks if a url is excluded, called only for sites
//   that have exclusion (via a robots.txt file)
//
// Input:
//   doc - the document object
//   path - the path on that site
// 
// Return:
//   true iff the path must be rejected
//

bool seeder_is_excluded( doc_t *doc, char *path ) {
	assert( doc->docid > 0 );
	assert( path != NULL );
	assert( path[0] != '/' );

	// If this is a exclusion path mentioned in the 
	// robots.txt file, then it's certainly excluded
	if( doc->status == STATUS_DOC_EXCLUSION ) {
		return true;
	}

	// We will copy the path
	char path_cpy[MAX_STR_LEN];
	bool first_try	= true;

	uint i = 0;
	while( i<strlen(path) ) {

		// Check if this is the first try
		if( first_try ) {

			// If this is the first try, we will consider
			// the possibility of '/' being an exclusion path
			strcpy( path_cpy, "" );
			first_try	= false;

		} else {
			while( path[i] != '/' && i < strlen(path) ) {
				path_cpy[i] = path[i];
				i++;
			}
			if( i == strlen(path) ) {
				break;
			}
			path_cpy[i] = '\0';
		}

		// Check the partial path
		doc_t partial_doc;
		docid_t partial_docid;
		urlidx_status_t rc = urlidx_resolve_path( urlidx,
				doc->siteid, path_cpy, &(partial_docid) );

		if( rc == URLIDX_CREATED_PATH ) {
			metaidx_doc_default( &(partial_doc) );
			partial_doc.docid  = partial_docid;
			partial_doc.siteid = doc->siteid;
			partial_doc.depth  = doc->depth > 1 ? doc->depth - 1 : 1;

			// Every time you get a new docid from urlidx resolve_path,
			// YOU MUST store a document in the database
			metaidx_doc_store( metaidx, &(partial_doc) );

			site_t site;
			site.siteid = doc->siteid;
			metaidx_site_retrieve( metaidx, &(site) );
			site.count_doc ++;
			metaidx_site_store( metaidx, &(site) );

		} else if( rc == URLIDX_EXISTENT ) {
			partial_doc.docid = partial_docid;
			metaidx_doc_retrieve( metaidx, &(partial_doc) );

			if( partial_doc.status == STATUS_DOC_EXCLUSION ) {
				return true;
			}
		} else {
			cerr << "Wrong answer from urlidx_resolve_url: " << (int)rc << endl;

		}

		if( i > 0 ) {
			path_cpy[i] = '/';
			i++;
		}
	}

	return false;
}

//
// Name: seeder_open_indexes
//
// Description:
//   Opens all the indexes needed
//   

void seeder_open_indexes() {
	cerr << "Open indexes ... ";

	cerr << "[metaidx] ";
	metaidx = metaidx_open( COLLECTION_METADATA, false );
	assert( metaidx != NULL );

	cerr << "[linkidx] ";
	linkidx = linkidx_open( COLLECTION_LINK, false );
	assert( linkidx != NULL );

	cerr << "[urlidx] ";
	urlidx = urlidx_open( COLLECTION_URL, false );
	assert( urlidx != NULL );

	cerr << "done." << endl;
}

//
// Name: cleanup
//
// Description: 
//   Closes files and clean everything
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
	regfree( &reject_patterns );
	regfree( &sessionid_patterns );
}

//
// Name: seeder_usage
//
// Description:
//   Prints the usage notice
//

void seeder_usage() {
	cerr << "Usage: program [OPTION]" << endl;
	cerr << "Reads start urls or read urls found by the gatherer" << endl;
	cerr << endl;
	cerr << " -s, --start <filename>  read start urls from filename" << endl;
	cerr << " -f, --force id          force a harvest id, even if its already seeded" << endl;
	cerr << " --help                  this help message" << endl;
	cerr << endl;
	wire_stop(0);
}

