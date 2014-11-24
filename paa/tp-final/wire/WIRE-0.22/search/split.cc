
#include "split.h"

// 
// Name: main
//
// Description:
//   Main program for the splitter
//

int main( int argc, char **argv ) {
	// Init
	wire_start("split" );

	uint docid_begin = 0;
	uint docid_end   = 0;

	// Parsing options (long and short options)
	while(1) {
		int option_index = 0;
		static struct option long_options[] = {
			{ "help", 0, 0, 0 },
			{ "begin", 1, 0, 0 },
			{ "end", 1, 0, 0 }
		};

		char c = getopt_long( argc, argv, "hb:e:n", long_options, &(option_index) );

		if( c == -1 )
			break;

		switch(c) {
			case 0:
				if( !strcmp( long_options[option_index].name, "begin" ) ) {
					docid_begin = atol( optarg );
				} else if( !strcmp( long_options[option_index].name, "end" ) ) {
					docid_end = atol( optarg );
				} else if( !strcmp( long_options[option_index].name, "help" ) ) {
					split_usage();
				}
				break;
			case 'b':
				docid_begin = atol(optarg);
				break;
			case 'e':
				docid_end = atol(optarg);
				break;
			case 'h':
				split_usage();
				break;
		}
	}

	// Open url index
	cout << "Opening metaindex ...";
	metaidx = metaidx_open( COLLECTION_METADATA, true );
	assert( metaidx != NULL );

	// Open storage
	cout << "storage ... ";
	storage = storage_open( COLLECTION_TEXT, true );
	assert( storage != NULL );

	cout << "done." << endl;


	// For simplicity, we will chdir into the split directory
	cerr << "Chdir to " << COLLECTION_SPLIT << " ";
	int rc = chdir( COLLECTION_SPLIT );
	if( rc != 0 ) {
		perror( COLLECTION_SPLIT );
		die( "chdir" );
	}
	cerr << "done." << endl;

	// Will create all the directories needed first, this is faster
	cerr << "Creating directories for " << metaidx->count_site << " sites ... ";
	for( siteid_t i = 1; i <= metaidx->count_site; i++ ) {
		char filename[MAX_STR_LEN];
		sprintf( filename, "%lu", i );
		rc = mkdir( filename, 0775 );

		// Directory may exist, in that case, we don't complain
		if( rc != 0 && errno != EEXIST ) {
			perror( filename );
			die( "mkdir" );
		}

		// We report the user the percentage
		cerr << ".";
		if( i % SPLIT_PAGE_LEN == 0 ) {
			cerr << " " << (i * 100) / metaidx->count_site << "%" << endl;
		}
	}

	cerr << "done." << endl;

	// The user may not give the begin and end docid, so we
	// provide them by default.

	docid_begin = docid_begin > 0 ? docid_begin : 1;
	docid_end   = docid_end   > 0 ? docid_end   : metaidx->count_doc;

	cerr << "Copying " << docid_end - docid_begin << " documents ... ";


	// Prepare the buffer only once; char buf[MAX_DOC_LEN] is not enough,
	// fails under some circumstances
	doc_t doc;
	char *buf = (char *)malloc(sizeof(char)*MAX_DOC_LEN);
	assert( buf != NULL );
	off64_t len;

	// Copying documents that have content
	for( doc.docid = docid_begin; doc.docid <= docid_end; doc.docid++ ) {
		char filename[MAX_STR_LEN];
		metaidx_status_t metaidx_status = metaidx_doc_retrieve( metaidx, &(doc) );
		assert( metaidx_status == METAIDX_OK );
		if( doc.content_length > 0 ) {
			storage_status_t storage_status = storage_read( storage, doc.docid, buf, &(len) );

			// Filename begins with the directory
			if( storage_status == STORAGE_OK ) {
			    len = strlen(buf);
				sprintf( filename, "%lu/%lu", doc.siteid, doc.docid );

				FILE *out = fopen64( filename, "w" );
				assert( out != NULL );
				size_t result = fwrite( buf, 1, len, out );
				assert( result == len );
				fclose( out );

				cerr << ".";
			} else {

				// Not found
				cerr << "!";
			}
		} else {
			cerr << "_";
		}
		if( (doc.docid - docid_begin) % SPLIT_PAGE_LEN == 0 ) {
			cerr << " " << ((doc.docid - docid_begin) * 100) / (docid_end - docid_begin) << "%" << endl;
		}
	}
	free(buf);

	cerr << " done." << endl;
	

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
		cout << "[metaidx] ";
	}
	if( storage != NULL ) {
		storage_close( storage );
		cout << "[storage] ";
	}
}

//
// Name: split_usage
//
// Description:
//   Shows the usage message
//

void split_usage() {
	cerr << "Usage: program [OPTION]" << endl;
	cerr << "Divides the text collection in files, one subdirectory" << endl;
	cerr << "per site, and inside, one file for each document. This" << endl;
	cerr << "is to be indexed by a tool like swish-e" << endl;
	cerr << endl;
	cerr << " -b, --begin docid    start doc, default first" << endl;
	cerr << " -e, --end docid      end doc, default last" << endl;
	cerr << " -h, --help           this help" << endl;
	cerr << endl;
	wire_stop(0);
}



