#include "test_parser.h"
#include "xmlconf-main.h"

int main( int argc, char **argv ) {
	wire_start( "testparser" );
	parser_init();

	mime_type_t	mime_type		= MIME_TEXT_HTML;
	char filename[MAX_STR_LEN]	= "";

	while(1) {
		int option_index = 0;
		
		static struct option long_options[] = {
			{"help", 0, 0, 0},
			{"html", 0, 0, 0},
			{"text", 0, 0, 0},
			{"robots-txt", 0, 0, 0},
			{"robots-rdf", 0, 0, 0},
			{0,0,0,0}
		};

		char c = getopt_long (argc, argv, "h",
				long_options, &option_index );

		if( c == -1 ) {
			if( optind >= 0 ) {
				strcpy( filename, argv[optind] );
			} else {
				test_parser_usage();
			}
			break;
		}

		switch(c) {
			case 0:
				if( !strcmp( long_options[option_index].name,
							"html" ) ) {
					mime_type	= MIME_TEXT_HTML;
				} else if( !strcmp( long_options[option_index].name,
							"text" ) ) {
					mime_type	= MIME_TEXT_PLAIN;
				} else if( !strcmp( long_options[option_index].name,
							"robots-txt" ) ) {
					mime_type	= MIME_ROBOTS_TXT;
				} else if( !strcmp( long_options[option_index].name,
							"robots-rdf" ) ) {
					mime_type	= MIME_ROBOTS_RDF;
				} else if( !strcmp( long_options[option_index].name,
							"help" ) ) {
					test_parser_usage();
				} else {
					test_parser_usage();
				}
				break;
			case 'h':
				test_parser_usage();
				break;
		}
	}

	if( strlen(filename) == 0 ) {
		test_parser_usage();
	}

	// Input file
	cerr << "Input filename            : " << filename << endl;

	// Stat input file to check size
	struct stat64 statbuf;
	lstat64( filename, &statbuf );
	off64_t len = statbuf.st_size;
	assert( len > 0 );
	assert( len < MAX_DOC_LEN );

	// Prepare buffers
	cerr << "Requesting memory         : " << len << endl;
	char *inbuf = (char *)malloc(len+1);
	assert( inbuf != NULL );
	char *outbuf = (char *)malloc(len+1);
	assert( outbuf != NULL );

	// Open output file
	cerr << "Writing links to download : " <<  FILENAME_TEST_LINKS_DOWNLOAD << endl;
	cerr << "Writing logged links      : " <<  FILENAME_TEST_LINKS_LOG << endl;
	links_download	= fopen64( FILENAME_TEST_LINKS_DOWNLOAD, "w" );
	links_log		= fopen64( FILENAME_TEST_LINKS_LOG, "w");

	// Read the input file
	cerr << "Reading                   : ";

	size_t rlen = read_file( filename, inbuf );
	assert( rlen > 0 );
	cerr << rlen << " bytes read" << endl;
	inbuf[rlen-1] = '\0';
	assert( len == rlen );

	// Prepare for parsing the file
	doc_t doc;
	doc.docid				= 1;
	doc.siteid				= 1;
	doc.mime_type			= mime_type;
	doc.raw_content_length	= len;

	cerr << "Parsing                   : ";
	off64_t size = parser_process( &doc, inbuf, outbuf );
	cerr << "strlen(out)=" << strlen(outbuf) << ", size=" << size << " bytes read" << endl;
	fclose( links_log );
	fclose( links_download );

	cerr << endl;
	cerr << "------------------- OUTPUT ---------------------" << endl;
	cerr << "=====START====="  << outbuf << "=====END=====" << endl;
	cerr << "------------- LINKS TO DOWNLOAD ----------------" << endl;
	read_file( FILENAME_TEST_LINKS_DOWNLOAD, inbuf );
	cerr << inbuf << endl;
	cerr << "---------------- LINKS TO LOG ------------------" << endl;
	read_file( FILENAME_TEST_LINKS_LOG, inbuf );
	cerr << inbuf << endl;
	cerr << "------------------ MD5 hash --------------------" << endl;

	char hash[MAX_STR_LEN];
	md5_string( outbuf, strlen(outbuf), hash );

	cerr << "[" << hash << "]" << endl;
	cerr << "------------------------------------------------" << endl;

	wire_stop(0);
}

void cleanup() {

}

//
// Name: test_parser_usage
//
// Description:
//   Shows an usage message
//

void test_parser_usage() {
	cerr << "Usage: program filename [OPTION]" << endl;
	cerr << "Tests the different parsers" << endl;
	cerr << endl;
	cerr << " filename                name of the file to parse" << endl;
	cerr << " --html                  file type is html (default)" << endl;
	cerr << " --text                  file type is plain text" << endl;
	cerr << " --robots-txt            file type is robots.txt format" << endl;
	cerr << " --robots-rdf            file type is robots.rdf format" << endl;
	cerr << " --help                  this help message" << endl;
	cerr << endl;
	wire_stop(0);
}

