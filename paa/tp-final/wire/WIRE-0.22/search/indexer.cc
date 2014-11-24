
#include "indexer.h"
#include "feeder.h"

// 
// Name: main
//
// Description:
//   Main program for the indexer


int main( int argc, char **argv ) {
	// Init
	wire_start("search-indexer" );
	
	docid_t opt_from	= 0;
	docid_t opt_to		= 0;
	bool opt_config		= false;
	bool opt_index		= false;
	feeder_format_t opt_format	= FEEDER_FORMAT_UNDEFINED;

	while(1) {
		int option_index = 0;

		static struct option long_options[] = {
			{"help", 0, 0, 0},
			{"from", 1, 0, 0},
			{"to", 1, 0, 0},
			{"format", 1, 0, 0 },
			{"config", 0, 0, 0},
			{"index", 0, 0, 0},
			{0, 0, 0, 0}
		};

		char c = getopt_long (argc, argv, "hci",
			long_options, &option_index);

		if (c == -1)
			break;

		switch (c) {
			case 0:
				if( !strcmp( long_options[option_index].name,
							"from" ) ) {
					opt_from = atol( optarg );
					if( opt_from == 0 ) {
						indexer_usage();
					}
				} else if( !strcmp( long_options[option_index].name,
							"to" ) ) {
					opt_to = atol( optarg );
					if( opt_to == 0 ) {
						indexer_usage();
					}
				} else if( !strcmp( long_options[option_index].name,
							"format" ) ) {
					if( !strcmp( optarg, "swish-e" ) ) {
						opt_format = FEEDER_FORMAT_SWISHE;
					} else if ( !strcmp( optarg, "lucene" ) ) {
					    opt_format = FEEDER_FORMAT_LUCENE;
					} else {
						die( "Unrecognized format, only 'swish-e' and 'lucene' are currently valid" );
					}

				} else if( !strcmp( long_options[option_index].name,
							"config" ) ) {

					opt_config = true;

				} else if( !strcmp( long_options[option_index].name,
							"index" ) ) {

					opt_index = true;
					
				} else if( !strcmp( long_options[option_index].name,
							"help" ) ) {
					indexer_usage();
				}
				break;
			case 'h':
				indexer_usage();
				break;
			default:
				indexer_usage();
		}
	}

	// Check actions
	if( !(opt_config||opt_index) ) {
		cerr << "You must specify an action (--config or --index)" << endl;
		indexer_usage();
	}

	// Check format
	if( opt_format == FEEDER_FORMAT_UNDEFINED ) {
		cerr << "You must specify a file format using '--format'" << endl;
		indexer_usage();
	}

	// Create directory for index
	char dirname[MAX_STR_LEN];
	sprintf( dirname, "%s/%s", CONF_COLLECTION_BASE, COLLECTION_INDEX );
	cerr << "Index will be created in " << dirname << endl;
	createdir( dirname );

	// Create filename for configuration
	char filename_conf[MAX_STR_LEN];
	if ( opt_format == FEEDER_FORMAT_SWISHE ) {
    	sprintf( filename_conf, "%s/%s/%s", CONF_COLLECTION_BASE, COLLECTION_INDEX, SWISHE_FILE_CONFIG );
    }
    else if ( opt_format == FEEDER_FORMAT_LUCENE ) {
        sprintf( filename_conf, "%s/%s/%s", CONF_COLLECTION_BASE, COLLECTION_INDEX, LUCENE_FILE_CONFIG );
    }

	// Create configuration
	if( opt_config ) {
		// This string will contain the full path and filename_conf
		// to execute the seeder program
		char full_feeder_path[MAX_STR_LEN];
		full_feeder_path[0] = '\0';

		// Search for program in the path
		char path[MAX_STR_LEN];
		strcpy( path, getenv( "PATH" ) );
		assert( strlen(path) > 0 );

		// Prepare a statbuffer
		struct stat64 statbuf;

		char *component	= strtok( path, ":" );
		bool found		= false;
		cerr << "Searching for " << FEEDER_PROGRAM_NAME << " in current path ... ";
		while( component != NULL ) {

			// Remove trailing '/' in path
			while( (strlen(component) > 0)
			    && (component[strlen(component)-1] == '/') ) {
				component[strlen(component)-1] = '\0';

			}
				
			// Create full_feeder_path
			sprintf( full_feeder_path, "%s/%s", component, FEEDER_PROGRAM_NAME );

			// Test if found
			if( ! stat64( full_feeder_path, &(statbuf) ) ) {
				if( S_IXUSR & statbuf.st_mode ) {
					found = true;
					break;
				}
			}

			component = strtok( NULL, ":" );
		}

		if( !found ) {
			cerr << "failed" << endl;
			cerr << "Current path is " << getenv(path) << endl;
			die( "Can't find feeder executable in current PATH" );
		}

		cerr << "ok" << endl;
		cerr << "Program found: '" << full_feeder_path << "'" << endl;

		// Create configuration file
		cerr << "Creating configuration file: ";

		FILE *file_conf = fopen64( filename_conf, "w" );
		if( file_conf == NULL ) {
			perror( filename_conf );
			die( "Couldn't open configuration file for writing" );
		}
		cerr << filename_conf << " ... ";

		// Write to configuration file

        if ( opt_format == FEEDER_FORMAT_SWISHE ) {
            write_swishe_conf(file_conf, argv[0], full_feeder_path, opt_from, opt_to);
		}
		else if ( opt_format == FEEDER_FORMAT_LUCENE ) {
            write_lucene_conf(file_conf, argv[0], full_feeder_path, opt_from, opt_to);
		}
		cerr << "ok" << endl;
	}
	
	// Index
	if( opt_index ) {
	    if ( opt_format == FEEDER_FORMAT_SWISHE ) {
            execlp( SWISHE_PROGRAM,
                    "-e",					// Economy mode
                    "-c", filename_conf,	// Configuration
                    "-S", "prog",			// Method
                    NULL					// End list
            );
    
    
            // If we are here, then something wrong happened
            cerr << endl;
            cerr << "Problem while running swish-e:" << endl;
            perror( SWISHE_PROGRAM );
            wire_stop(1);
		}
		else if ( opt_format == FEEDER_FORMAT_LUCENE ) {
		    //The classpath environment variable must be set to the lucene directory from the wire program
		    execlp( LUCENE_PROGRAM,
		            "",
                    "HTMLIndexer",
                    "--config", filename_conf,	// Configuration
                    NULL					// End list
            );
            
            // If we are here, then something wrong happened
            cerr << endl;
            cerr << "Problem while running lucene:" << endl;
            perror( LUCENE_PROGRAM );
            wire_stop(1);
		}
	}

	wire_stop(0);
}

void write_lucene_conf(FILE * file_conf, char * generator, char * full_feeder_path, docid_t opt_from, docid_t opt_to) {
    // Header
    fprintf( file_conf, "# Generated by %s\n", generator );
    time_t now = time(NULL);
    fprintf( file_conf, "# Date: %s\n", ctime(&(now)) );
    
    // Feeder program
    fprintf( file_conf, "IndexFeeder %s\n", full_feeder_path );
    
    // Feeder parameters
    char feeder_params[MAX_STR_LEN] = "";
    sprintf( feeder_params, "--format lucene " );

    if( (opt_from > 0) || (opt_to > 0) ) {
        char params[MAX_STR_LEN]		= "";

        if( opt_from > 0 ) {
            sprintf( params, "--from %lu ", opt_from );
            strcat( feeder_params, params );
        }
        
        if( opt_to > 0 ) {
            sprintf( params, "--to %lu ", opt_to );
            strcat( feeder_params, params );
        }

    }
    fprintf( file_conf, "IndexFeederParams %s\n", feeder_params );
    
    //The following parameters are not used by the indexer program, but are added for future development
    // Index file
    fprintf( file_conf, "RootDir %s/%s/\n", CONF_COLLECTION_BASE, COLLECTION_INDEX );
    
     // Parser parameters
    fprintf( file_conf, "DefaultContents HTML\n" );
    fprintf( file_conf, "StoreDescription HTML <body> %d\n", CONF_INDEX_DESCRIPTIONSIZE );

    // Stopword parameters
    fprintf( file_conf, "MinWordLimit %d\n", CONF_INDEX_MINWORDLENGTH );
    fprintf( file_conf, "MaxWordLimit %d\n", CONF_INDEX_MAXWORDLENGTH );
    fprintf( file_conf, "IgnoreWords file: %s\n", CONF_INDEX_STOPWORDSFILE );

    // Properties
    fprintf( file_conf, "PropertyNamesNumeric pagerank wlrank cscore\n" );
    fprintf( file_conf, "PreSortedIndex pagerank wlrank cscore\n" );
    
    fclose( file_conf );
}

void write_swishe_conf(FILE * file_conf, char * generator, char * full_feeder_path, docid_t opt_from, docid_t opt_to) {
    // Header
    fprintf( file_conf, "# Generated by %s\n", generator );
    time_t now = time(NULL);
    fprintf( file_conf, "# Date: %s\n", ctime(&(now)) );

    // Feeder program
    fprintf( file_conf, "IndexDir %s\n", full_feeder_path );

    // Feeder parameters
    char feeder_params[MAX_STR_LEN] = "";
    sprintf( feeder_params, "--format swish-e " );

    if( (opt_from > 0) || (opt_to > 0) ) {
        char params[MAX_STR_LEN]		= "";

        if( opt_from > 0 ) {
            sprintf( params, "--from %lu ", opt_from );
            strcat( feeder_params, params );
        }
        
        if( opt_to > 0 ) {
            sprintf( params, "--to %lu ", opt_to );
            strcat( feeder_params, params );
        }

    }
    fprintf( file_conf, "SwishProgParameters %s\n", feeder_params );

    // Index file
    fprintf( file_conf, "IndexFile %s/%s/", CONF_COLLECTION_BASE, COLLECTION_INDEX );
    
    if( opt_from > 0 ) {
        fprintf( file_conf, "from_%lu_", opt_from );
    }
    if( opt_to > 0 ) {
        fprintf( file_conf, "to_%lu_", opt_to );
    }
    fprintf( file_conf, "%s\n", SWISHE_INDEX_FILE );
    
    // Parser parameters
    fprintf( file_conf, "DefaultContents HTML\n" );
    fprintf( file_conf, "StoreDescription HTML <body> %d\n", CONF_INDEX_DESCRIPTIONSIZE );

    // Stopword parameters
    fprintf( file_conf, "MinWordLimit %d\n", CONF_INDEX_MINWORDLENGTH );
    fprintf( file_conf, "MaxWordLimit %d\n", CONF_INDEX_MAXWORDLENGTH );
    fprintf( file_conf, "IgnoreWords file: %s\n", CONF_INDEX_STOPWORDSFILE );

    // Properties
    fprintf( file_conf, "PropertyNamesNumeric pagerank wlrank cscore\n" );
    fprintf( file_conf, "PreSortedIndex pagerank wlrank cscore\n" );

    fclose( file_conf );
}

//
// Name: cleanup
//
// Description: 
//   Closes files, indexes
//

void cleanup() {
}

//
// Name: indexer_usage
//
// Description:
//   Prints an usage message, then stops
//

void indexer_usage() {
	cerr << "Usage: program --format FORMAT (--config|--index) [OPTION]" << endl;
	cerr << "Indexes the collection using an external program" << endl;
	cerr << "It is usually executed in two stages:" << endl;
	cerr << "  1. Create the configuration file" << endl;
	cerr << "  2. Create the index" << endl;
	cerr << endl;
	cerr << " --format FORMAT    Indexing format, can be either swish-e or lucene" << endl;
	cerr << " --config -c        Creates the configuration file" << endl;
	cerr << " --index -i         Runs the external indexing program" << endl;
	cerr << endl;
	cerr << " --from             First docid to index, default 1" << endl;
	cerr << " --to               Last docid to index, default ndocs" << endl;
	cerr << " --help             This help message" << endl;
	cerr << endl;
	wire_stop(0);
}
