#include "extract.h"

// 
// Name: main
//
// Description:
//   Main program for the extract
//

int main( int argc, char **argv ) {
	wire_start("extract" );

	// Parse options
	bool opt_dump_doc			= false; bool opt_url_depth		= false;
									     bool opt_url			= false;
	bool opt_dump_links			= false;
	bool opt_dump_logged_links	= false; uint opt_depth		   = 0;
										 bool opt_with_source_doc	= false;
										 bool opt_with_caption		= false;
	bool opt_dump_site			= false;
	bool opt_dump_seeds		= false;
	bool opt_dump_sitelinks		= false;
	bool opt_dump_harvest		= false;
	bool opt_dump_docs_harvest	= false; uint opt_harvester_id = 0;
	bool opt_dump_text			= false;
	bool opt_dump_texts			= false;
	docid_t opt_from			= 0;
	docid_t opt_to				= 0;
	bool opt_dump_sketch            = false;
	bool opt_dump_sketchall         = false;
	char filename_docids[MAX_STR_LEN];
	uint opt_dump_sketch_docid = 0;
	while(1) {
		int option_index = 0;

		static struct option long_options[] = {
			{"help", 0, 0, 0},
			{"harvest", 0, 0, 0},
			{"docs-harvest", 1, 0, 0},
			{"logged-links", 0, 0, 0},
			{"depth", 1, 0, 0},
			{"url-depth", 0, 0, 0},
			{"url", 0, 0, 0},
			{"with-source-doc", 0, 0, 0},
			{"with-caption", 0, 0, 0},
			{"links", 0, 0, 0},
			{"site", 0, 0, 0},
			{"seeds", 0, 0, 0},
			{"sketch", 1, 0, 0},
			{"sketches", 0, 0, 0},
			{"sitelinks", 0, 0, 0},
			{"doc", 0, 0, 0},
			{"text", 1, 0, 0},
			{"texts", 0, 0, 0},
			{"from", 1, 0, 0},
			{"to", 1, 0, 0},
			{0, 0, 0, 0}
		};

		char c = getopt_long (argc, argv, "htdls",
			long_options, &option_index);

		if (c == -1)
			break;

		switch (c) {
			case 0:
				if( !strcmp( long_options[option_index].name, "harvest" ) ) {
					opt_dump_harvest = true;
				} else if( !strcmp( long_options[option_index].name, "docs-harvest" ) ) {
					opt_dump_docs_harvest	= true;
					opt_harvester_id		= atol( optarg );
				} else if( !strcmp( long_options[option_index].name, "logged-links" ) ) {
					opt_dump_logged_links = true;
				} else if( !strcmp( long_options[option_index].name, "depth" ) ) {
					opt_depth		= atoi( optarg );
				} else if( !strcmp( long_options[option_index].name, "url" ) ) {
					opt_url			= true;
				} else if( !strcmp( long_options[option_index].name, "url-depth" ) ) {
					opt_url_depth		= true;
				} else if( !strcmp( long_options[option_index].name, "with-source-doc" ) ) {
					opt_with_source_doc	= true;
				} else if( !strcmp( long_options[option_index].name, "with-caption" ) ) {
					opt_with_caption	= true;
				} else if( !strcmp( long_options[option_index].name, "links" ) ) {
					opt_dump_links = true;
				} else if( !strcmp( long_options[option_index].name, "site" ) ) {
					opt_dump_site = true;
				} else if( !strcmp( long_options[option_index].name, "seeds" ) ) {
					opt_dump_seeds = true;
				} else if( !strcmp( long_options[option_index].name, "sketch" ) ) {
				        opt_dump_sketch = true;
					opt_dump_sketch_docid = atol(optarg);
				} else if( !strcmp( long_options[option_index].name, "sketches" ) ) {
				        opt_dump_sketchall = true;
				} else if( !strcmp( long_options[option_index].name, "sitelinks" ) ) {
					opt_dump_sitelinks = true;
				} else if( !strcmp( long_options[option_index].name, "doc" ) ) {
					opt_dump_doc = true;
				} else if( !strcmp( long_options[option_index].name, "text" ) ){
					opt_dump_text = true;
					strcpy( filename_docids, optarg );
				} else if( !strcmp( long_options[option_index].name, "texts" ) ){
					opt_dump_texts = true;
				} else if( !strcmp( long_options[option_index].name, "from" ) ) {
					opt_from = atol(optarg);
				} else if( !strcmp( long_options[option_index].name, "to" ) ) {
					opt_to = atol(optarg);
				} else if( !strcmp( long_options[option_index].name,
							"help" ) ) {
					extract_usage();
				}
				break;
			case 'l':
				opt_dump_links = true;
				break;
			case 's':
				opt_dump_site = true;
				break;
			case 'd':
				opt_dump_doc = true;
				break;
		}
	}

	if( !(opt_dump_doc||opt_dump_site||opt_dump_harvest||opt_dump_docs_harvest||opt_dump_links||opt_dump_logged_links||opt_dump_text||opt_dump_texts||opt_dump_sitelinks||opt_dump_seeds||opt_dump_sketch||opt_dump_sketchall) ) {
		extract_usage();
	}


	// Harvest information
	if( opt_dump_harvest ) {
		harvest_dump_harvests( stdout );
	}

	// Harvest lists
	if( opt_dump_docs_harvest ) {

		assert( opt_harvester_id > 0 );
		harvest_t *harvest = harvest_open( COLLECTION_HARVEST, opt_harvester_id, true );
		assert( harvest != NULL );
		cerr << "Harvest " << opt_harvester_id << " should have " << harvest->count << " documents" << endl;

		// This function is defined in harvestidx
		harvest_dump_list( harvest );
		harvest_close( harvest );
	}

	// If needed, open metaidx
	if( opt_dump_site || opt_dump_doc || opt_dump_links || opt_dump_logged_links || opt_dump_sitelinks || opt_dump_seeds || opt_dump_sketch || opt_dump_sketchall || opt_dump_texts ) {
		cerr << "Opening metaidx ...";
		metaidx = metaidx_open( COLLECTION_METADATA, true );
		assert( metaidx != NULL );
		cerr << "done." << endl;
	}

	// If needed, open urlidx
	if( opt_dump_site || opt_dump_logged_links || opt_dump_text || opt_dump_texts || opt_dump_seeds || opt_url_depth || opt_url || opt_dump_sketch || opt_dump_sketchall ) {

		cerr << "Opening urlidx ... ";
		urlidx = urlidx_open( COLLECTION_URL, true );
		assert( urlidx != NULL );
		cerr << "done." << endl;

	}

	// Sites
	if( opt_dump_site ) {
		metaidx_dump_sitelist( metaidx, urlidx, stdout );
	}

	// Seeds
	if( opt_dump_seeds ) {
		char sitename[MAX_STR_LEN];
		site_t site;
		docid_t ndocs	= 0;

		// Create an array of zeros, we will compare this against
		// the structure
		char zeros[sizeof(struct in_addr)];
		memset( zeros, 0, sizeof(struct in_addr) );

		for( site.siteid = 1; site.siteid <= metaidx->count_site; site.siteid ++ ) {
			metaidx_site_retrieve( metaidx, &(site) );
			if( site.count_doc_ok > 0 ) {
				urlidx_site_by_siteid( urlidx, site.siteid, sitename );
				cout << "http://" << sitename << "/";

				// Check if the site has a valid IP
				if( memcmp( (void *)zeros, (void *)(&(site.addr)), sizeof(struct in_addr) ) != 0 ) {				
					cout << " IP=" << inet_ntoa(site.addr);
				}
				cout << endl;

				ndocs++;
			}
		}

		if( ndocs == 0 ) {

			cerr << "*** Warning: no sites to export. Perhaps you must run: " << endl;
			cerr << "    'wire-info-analysis --site-statistics' first" << endl;
		}
	}

	// Sitelinks
	if( opt_dump_sitelinks ) {
		sitelinkidx_t *sitelinkidx = sitelink_load( COLLECTION_SITELINK );
		sitelink_dump_structure( sitelinkidx );
		free( sitelinkidx );
	}

	// Documents
	if( opt_dump_doc ) {

		doc_t doc;

		// Show header
		if( ! (opt_url_depth||opt_url) ) {
			metaidx_dump_doc_header( stdout );
		}

		// Iterate through documents
		char url[MAX_STR_LEN]	= "";
		docid_t from = ( opt_from > 0 ? opt_from : 1 );
		docid_t to   = ( opt_to   > 0 ? opt_to   : metaidx->count_doc );

		for( doc.docid = from; doc.docid <= to; doc.docid++ ) {
			if( ! opt_url ) {
				metaidx_doc_retrieve( metaidx, &(doc) );
			}

			if( opt_url_depth ) {
				// Show only url and depth
				urlidx_url_by_docid( urlidx, doc.docid, url );

				cout << doc.depth << " " << url << endl;

			} else if( opt_url ) {
				urlidx_url_by_docid( urlidx, doc.docid, url );

				cout << doc.docid << " " << url << endl;

			} else {
				// Check depth
				if( opt_depth == 0 || opt_depth == doc.depth ) {
					metaidx_dump_doc( &(doc), stdout );
				}
			}
		}
	}

	// Text
	if( opt_dump_text || opt_dump_texts ) {
		char url[MAX_STR_LEN];
		char *buf;
		buf = (char *)malloc(sizeof(char)*MAX_DOC_LEN);
		assert( buf != NULL );

		storage = storage_open( COLLECTION_TEXT, true );

		if( opt_dump_text ) {
			ifstream input_file;
			input_file.open( filename_docids );
			assert( input_file.is_open() );
			char line[MAX_STR_LEN];
			while( ! input_file.eof() && input_file.getline(line,MAX_STR_LEN) ) {
				docid_t docid = atol(line);
				extract_dump_document( docid, url, buf );
			}
			input_file.close();
		}

		if( opt_dump_texts ) {
			docid_t from = ( opt_from > 0 ? opt_from : 1 );
			docid_t to   = ( opt_to   > 0 ? opt_to   : metaidx->count_doc );
			doc_t doc;
			for( docid_t docid = from; docid <= to; docid++ ) {
				doc.docid = docid;
				metaidx_doc_retrieve( metaidx, &(doc) );
				assert( doc.docid == docid );
				if( doc.status == STATUS_DOC_GATHERED && ( HTTP_IS_OK( doc.http_status ) ) ) {
					extract_dump_document( docid, url, buf );
				}
			}
		}
}
	  
	// Sketch (only if supported)
	    
	if( (opt_dump_sketchall || opt_dump_sketch) && CONF_GATHERER_USESKETCHES) {  
	  char url[MAX_STR_LEN];
	  irudiko_sketch_t sketch;
	  
	  storage = storage_open( COLLECTION_TEXT, true );
	  docid_t ndocs = metaidx->count_doc;
	  
	  /* Managing both cases (--sketch <id>, and --sketches) */
	  docid_t curdocid = (opt_dump_sketchall)?(opt_from > 0 ? opt_from : 1):opt_dump_sketch_docid;
	  docid_t docid_end = (opt_dump_sketchall)?(opt_to > 0 ? opt_to : ndocs):opt_dump_sketch_docid;
	  
	  /* Loop */
	  for(; curdocid <= docid_end; ++curdocid) {
	    urlidx_url_by_docid( urlidx, curdocid, url );
	    doc_t doc;
	    doc.docid = curdocid;
	    metaidx_doc_retrieve( metaidx, &(doc) );
	    
	    /* Is a valid document? */
	    if( doc.docid == curdocid && doc.status == STATUS_DOC_GATHERED && ( HTTP_IS_OK( doc.http_status ) ) ) {
	      storage_read_sketch( storage, curdocid, &sketch );

	      /* This is to ensure only significant sketches are returned */
	      bool is_empty_sketch = true;
	      for(uint h = 0; h < IRUDIKO_SKETCHSIZE; ++h)
		if (sketch.sketch[h]!=0) { is_empty_sketch=false; break; }
	      
	      if(!is_empty_sketch) {
		/* Print docinfo + sketch */
		cout << "<!-- DOCID: " << curdocid << " URL: " << url << " -->" << endl;
		cout << "SKETCH: " << sketch.sketch[0];
		for(uint h = 1; h < IRUDIKO_SKETCHSIZE; ++h)
		  cout << ", " << sketch.sketch[h];
		cout << "." << endl;
	      }
	    }
	  }
	}
	
	// Links
	if( opt_dump_links ) {
                linkidx = linkidx_open( COLLECTION_LINK, true );
                docid_t ndocs = metaidx->count_doc;
                docid_t docid;

                char *ok = (char *)malloc(sizeof(char)*(ndocs+1));
                assert( ok != NULL );
                cerr << "Marking ok documents" << endl;
                doc_t doc;
                for( docid = 1; docid <= ndocs; docid++ ) {
                        doc.docid = docid;
                        metaidx_doc_retrieve( metaidx, &(doc) );
                        if( doc.docid == docid && doc.status == STATUS_DOC_GATHERED && ( HTTP_IS_OK( doc.http_status ) || HTTP_IS_REDIRECT( doc.http_status ) ) ) {
                                ok[docid] = 'X';
                        } else {
                                ok[docid] = '\0'; /* Null means omit */
                        }
                }
                cerr << "done" << endl;

		for( docid = 1; docid <= ndocs; docid++ ) {
			if( ok[docid] != '\0' ) {
				linkidx_dump_links_checking( linkidx, docid, ok );
			}
		}
		free(ok);

	}

	// Logged links
	if( opt_dump_logged_links ) {
		char input_filename[MAX_STR_LEN];
		char *line;
		line = (char *)malloc(sizeof(char)*MAX_STR_LEN*3);
		assert( line != NULL );

		int harvestid 	= 1;

		while(1) {
			ifstream input_file;
			sprintf( input_filename, "%s/%d%s", COLLECTION_LINK, harvestid, FILENAME_LINKS_LOG );
			cerr << "Opening links from harvest #" << harvestid << " ... ";
			input_file.open( input_filename );
			if( !input_file.is_open() ) {

				// Check for errors
				if( errno != ENOENT ) {
					perror( input_filename );
					die( "Couldn't open file" );
				}	

				// Ok, end
				cerr << "not found (ok)" << endl;
				break;
			}
			cerr << "ok" << endl;

			// Process file
			docid_t src_docid	= 0;
			doc_t src_doc;
			src_doc.docid		= 0;
			int rel_pos			= 0;
			int tag				= 0;
			int caption_length	= 0;
			char url[MAX_STR_LEN];
			char caption[MAX_STR_LEN];
			char src_path[MAX_STR_LEN];
			

			while( ! input_file.eof() && input_file.getline(line,MAX_STR_LEN*2) ) {

				if( !
				   (sscanf( line, "%lu %d %d %s %[^\n]", (docid_t *)&(src_docid), &rel_pos, &tag, url, caption ) == 6 )
				   ) {
					assert( src_docid != 0 );
					caption_length = strlen(caption);
					assert( strlen(url) != 0 );

					// Check if the source docid has changed
					if( src_docid != src_doc.docid ) {
						src_doc.docid	= src_docid;
						metaidx_doc_retrieve( metaidx, &(src_doc) );


						// Save time by skipping documents at a 
						// different depth
						if( opt_depth == 0 || src_doc.depth == opt_depth ) {
							urlidx_url_by_docid( urlidx, src_doc.docid, src_path );
						}
					}

					// Check depth
					if( opt_depth == 0 || src_doc.depth == opt_depth ) {

						// Now we have a src_doc and a src_path
						assert( src_path != NULL );

						// The only think left is to resolve the link
						if( strchr( url, '%' ) ) {
							unescape_url( url );
						}

						if( opt_with_source_doc ) {
							cout << src_docid << " http://" << src_path << " -> ";
						}

						// Check if url has protocol
						if( ! strchr( url, ':' ) ) {

							char absolute_path[MAX_STR_LEN];

							char source_path_cpy[MAX_STR_LEN];
							strcpy( source_path_cpy, src_path );

							// Relative url, fix
							 urlidx_relative_path_to_absolute( source_path_cpy, url, absolute_path );
							 cout << "http://" << absolute_path;


						} else {

							// Complete url, show
							cout << url;

						}

						if( opt_with_caption ) {
							cout << " " << caption;
							caption[0] = '\0';
						}

						cout << endl;
					}

				} else {
						cerr << "* UNRECOGNIZED * " << line << endl;
				}
			}
			
			// Next filename
			input_file.close();
			harvestid++;
		}

	}

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
	if( storage != NULL ) {
		storage_close( storage );
		cerr << "[storage] ";
	}
}


void extract_dump_document( docid_t docid, char *url, char *buf ) {
	off64_t size;
	urlidx_url_by_docid( urlidx, docid, url );
	storage_read( storage, docid, buf, &(size) );
	if( size > 0 ) {
		cout << "<!-- DOCID:" << docid << " SIZE:" << size << " " << url << " -->" << endl;
		cout << buf << endl;
	}
}

//
// Name: extract_usage
//
// Description:
//   Prints an usage message, then stops
//

void extract_usage() {
	cerr << "Usage: program [OPTION]" << endl;
	cerr << "Dumps the contents of the indices and structures" << endl;
	cerr << endl;
	cerr << " --doc, -d            dump doc information (slow!)" << endl;
	cerr << "    --from            first doc to show (default 1)" << endl;
	cerr << "    --to              last doc to show (default last in collection)" << endl;
	cerr << "    --url             dump list of urls (slow!)" << endl;
	cerr << "    --depth <n>       only show urls of docs at that depth" << endl;
	cerr << "    --url-depth       only show url and depth" << endl;
	cerr << " --text <filename>    dump content, filename contains docids, one per line" << endl;
	cerr << " --texts              dump content of documents specified by --from and --to" << endl;
	cerr << "    --from            first doc to show (default 1)" << endl;
	cerr << "    --to              last doc to show (default last in collection)" << endl;
	cerr << " --sketch <docid>     shows Irudiko sketch for document <docid>" << endl;
	cerr << " --sketches           shows Irudiko sketch for documents" << endl;
	cerr << "    --from            first doc to show (default 1)" << endl;
	cerr << "    --to              last doc to show (default last in collection)" << endl;
	cerr << endl;
	cerr << " --site, -s           dump site information" << endl;
	cerr << " --seeds              dump seeds for next collection" << endl;
	cerr << endl;
	cerr << " --links, -l          dump links information (slow!)" << endl;
	cerr << " --sitelinks          dumps the structure of site links" << endl;
	cerr << " --logged-links       dumps urls of logged documents and statistics about" << endl;
	cerr << "                       the filename extensions (slow!)" << endl;
	cerr << "    --depth <n>       only dump urls of logged documents with links" << endl;
	cerr << "                      from documents at depth <n>" << endl;
	cerr << "    --with-source-doc include the original url" << endl;
	cerr << "    --with-caption    include the textual caption" << endl;
	cerr << endl;
	cerr << " --harvest            dump information about harvests" << endl;
	cerr << " --docs-harvest <num> shows the url of the documents assigned" << endl;
	cerr << "                      to harvest number <num>" << endl;
	cerr << " --help, -h           this help message" << endl;
	cerr << endl;
	wire_stop(0);
}
