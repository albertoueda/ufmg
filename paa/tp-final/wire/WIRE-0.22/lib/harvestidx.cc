
#include "harvestidx.h"

//
// Name: harvest_exists
//
// Descriptions:
//   Checks if a harvest exist
//
// Input:
//   dirname - directory to search
//   harvestid - the number of the harvest
//
// Return:
//   True if harvest exists
//

bool harvest_exists( const char *dirname, int harvestid ) {
	char filename[MAX_STR_LEN];

	// Copy filename
	sprintf( filename, HARVESTIDX_FILENAME_MAIN, dirname, harvestid );

	// Try to open
	FILE *file_main = fopen64( filename, "r" );

	if( file_main == NULL ) {
		return false;
	} else {
		fclose( file_main );
		return true;
	}
}

//
// Name: harvest_open
//
// Description:
//   Opens a harvest
//
// Input:
//   directory - the directory where all harvestidx are
//   harvestid - the number of the harvest
//   readonly - read-only mode
//
// Return
//   a harvest object
//

harvest_t *harvest_open( const char *dirname, int harvestid, bool readonly ) {
	char filename[MAX_STR_LEN];

	// Require memory
	harvest_t *harvest = (harvest_t *)malloc(sizeof(harvest_t));
	assert( harvest != NULL );

	// Copy filename
	sprintf( filename, HARVESTIDX_FILENAME_MAIN, dirname, harvestid );

	// Check file size, it should contain a single 
	// harvest_t structure
	struct stat64 statbuf;
	if( stat64( filename, &statbuf ) == 0 ) {

		// Compare to size of harvest_t
		if( statbuf.st_size != sizeof(harvest_t) ) {

			// It doesn't match, check if it's the old format
			if( statbuf.st_size != sizeof(harvest_old_t) ) {

				// The size is wrong!
				cerr << "Wrong harvest_t (expected " << sizeof(harvest_t) << ", got " << statbuf.st_size << ")" << endl;
				cerr << "Please remove " << filename << " as it might be damaged; you must cancel the remaining harvest rounds" << endl;
				die( "Inconsistency in harvest_t" );
			} else {
				// Convert old file to new format
				cerr << "[Converting old harvest_t file]" << endl;
				harvest_old_t harvest_old;
				FILE *file_main_tmp	= fopen64( filename, "r" );
				assert( file_main_tmp != NULL );
				size_t readed = fread( &(harvest_old), sizeof(harvest_old_t), 1, file_main_tmp );
				assert( readed == 1 );
				assert( harvest_old.id == harvestid );
				fclose( file_main_tmp );

				// Copy old data
				harvest_t harvest_new;
				harvest_default( &(harvest_new) );

				strcpy( harvest_new.dirname, harvest_old.dirname );
				harvest_new.id				= harvest_old.id;
				harvest_new.count			= harvest_old.count;
				harvest_new.count_ok		= harvest_old.count_ok;
				harvest_new.count_site		= harvest_old.count_site;
				harvest_new.raw_size		= harvest_old.raw_size;
				harvest_new.size			= harvest_old.size;
				harvest_new.link_total		= harvest_old.link_total;
				harvest_new.link_old_pages	= harvest_old.link_old_pages;
				harvest_new.link_new_pages	= harvest_old.link_new_pages;
				harvest_new.link_new_sites	= harvest_old.link_new_sites;
				strcpy( harvest_new.hostname, harvest_old.hostname );
				harvest_new.creationtime	= harvest_old.creationtime;
				harvest_new.begintime		= harvest_old.begintime;
				harvest_new.endtime			= harvest_old.endtime;
				harvest_new.status			= harvest_old.status;

				file_main_tmp	= fopen64( filename, "w" );
				assert( file_main_tmp != NULL );
				size_t writen = fwrite( &(harvest_new), sizeof(harvest_t), 1, file_main_tmp );
				assert( writen == 1 );
				fclose( file_main_tmp );
			}
		}
	} else {
		perror( filename );
		die( "Couldn't stat harvest main file" );
	}

	// Try to open
	FILE *file_main = fopen64( filename, "r" );
	assert( file_main != NULL );

	// Read
	size_t readed = fread( harvest, sizeof(harvest_t), 1, file_main );
	assert( readed == 1 );
	assert( harvest->id == harvestid );
	fclose( file_main );

	// Open data files
	sprintf( filename, HARVESTIDX_FILENAME_DOC, dirname, harvestid );
	// Check size of file with docs
	if( stat64( filename, &statbuf ) == 0 ) {
		// Verify file size
		if( (uint)(statbuf.st_size) != (sizeof(doc_t) * harvest->count)) {
			if( (uint)(statbuf.st_size) != (sizeof(doc_old_t) * harvest->count)) {
				die( "Inconsistency in the size of doc_t (probably needs to re-compile" );
			} else {
				metaidx_convert_old_doc_file( filename );
			}
		}

	} else {
		perror( filename );
		die( "Couldn't stat file with docs" );
	}
	harvest->file_doc = fopen64( filename, "r+" );
	assert( harvest->file_doc != NULL );

	sprintf( filename, HARVESTIDX_FILENAME_SITE, dirname, harvestid );
	harvest->file_site = fopen64( filename, "r+" );
	assert( harvest->file_site != NULL );

	sprintf( filename, HARVESTIDX_FILENAME_PATH, dirname, harvestid );
	harvest->file_path = fopen64( filename, "r+" );
	assert( harvest->file_path != NULL );

	sprintf( filename, HARVESTIDX_FILENAME_SITENAME, dirname, harvestid );
	harvest->file_sitename = fopen64( filename, "r+" );
	assert( harvest->file_sitename != NULL );

	// Nullify lists
	harvest->doc_list = NULL;
	harvest->site_list = NULL;

	// Copy some vars
	strcpy( harvest->dirname, dirname );
	harvest->readonly = readonly;

	// Return
	return harvest;
}

//
// Name: harvest_default
//
// Descriptions:
//   Fills a harvest structure with the default data
//

void harvest_default( harvest_t *harvest ) {
	assert( harvest != NULL );

	harvest->count		= 0;
	harvest->count_site = 0;
	harvest->count_ok 	= 0;
	harvest->raw_size	= 0;
	harvest->size		= 0;

	harvest->link_total	= 0;
	harvest->link_old_pages	= 0;
	harvest->link_new_pages	= 0;
	harvest->link_new_sites	= 0;

	strcpy( harvest->hostname, "" );
	harvest->creationtime = time(NULL);
	harvest->begintime	= 0;
	harvest->endtime		= 0;
	harvest->status		= STATUS_HARVEST_EMPTY;
	harvest->readonly	= false;

	harvest->speed_ok		= 0;
	harvest->links_ratio	= 0;
	harvest->bytes_in		= 0;
	harvest->bytes_out		= 0;


	// Nullify lists
	harvest->doc_list = NULL;
	harvest->site_list = NULL;
}

//
// Name: harvest_create
//
// Description:
//   Creates a harvest
//
// Input:
//   directory - the directory to create
//   harvestid - the number of the harvest
//

harvest_t *harvest_create( const char *dirname ) {
	char filename[MAX_STR_LEN];

	// Require memory
	harvest_t *harvest = (harvest_t *)malloc(sizeof(harvest_t));
	assert( harvest != NULL );

	// Get next harvestid
	int harvestid = 1;
	while(1) {
		struct stat64 buf;
		int rc;

		// Create filename and stat
		sprintf( filename, HARVESTIDX_FILENAME_MAIN, dirname, harvestid );
		rc = stat64( filename, &(buf) );

		// Check result
		if( rc != 0 && errno != ENOENT ) {

			// Unexpected error
			perror( filename );
			die( "Error while trying to stat" );
		} else if( rc == 0 ) {

			// Found, try with next
			harvestid++;
		} else {

			// Not found
			break;
		}
	}

	// Copy filename
	sprintf( filename, HARVESTIDX_FILENAME_MAIN, dirname, harvestid );

	// Try to open
	FILE *file_main = fopen64( filename, "w" );
	assert( file_main != NULL );

	// Create object
	harvest_default( harvest );

	// Dirname
	harvest->id = harvestid;
	strcpy( harvest->dirname, dirname );

	// Write object
	size_t writen = fwrite( harvest, sizeof(harvest), 1, file_main );
	assert( writen == 1 );
	fclose( file_main );

	// Open data files
	sprintf( filename, HARVESTIDX_FILENAME_DOC, dirname, harvestid );
	harvest->file_doc = fopen64( filename, "w+" );
	assert( harvest->file_doc != NULL );

	sprintf( filename, HARVESTIDX_FILENAME_SITE, dirname, harvestid );
	harvest->file_site = fopen64( filename, "w+" );
	assert( harvest->file_site != NULL );

	sprintf( filename, HARVESTIDX_FILENAME_PATH, dirname, harvestid );
	harvest->file_path = fopen64( filename, "w+" );
	assert( harvest->file_path != NULL );

	sprintf( filename, HARVESTIDX_FILENAME_SITENAME, dirname, harvestid );
	harvest->file_sitename = fopen64( filename, "w+" );
	assert( harvest->file_sitename != NULL );


	// Return
	return harvest;
}

//
// Name: harvest_save_info
//
// Description:
//   Saves the information of a harvest
//
// Input:
//   harvest - the harvest object
//

void harvest_save_info( harvest_t *harvest ) {
	assert( harvest != NULL );
	assert( !harvest->readonly );

	// Filename
	char filename[MAX_STR_LEN];
	sprintf( filename, HARVESTIDX_FILENAME_MAIN, harvest->dirname, harvest->id );

	// Try to open
	FILE *file_main = fopen64( filename, "w" );
	assert( file_main != NULL );

	// Write to disk
	size_t writen = fwrite( harvest, sizeof(harvest_t), 1, file_main );
	assert( writen == 1 );

	// Close
	fclose( file_main );
}

//
// Name: harvest_close
//
// Description:
//   Writes a harvest back to disk (except if readonly)
//
// Input:
//   harvest - the harvest object
//

void harvest_close( harvest_t *harvest ) {
	assert( harvest != NULL );

	// Close files
	fclose( harvest->file_doc );
	fclose( harvest->file_site );
	fclose( harvest->file_path );
	fclose( harvest->file_sitename );
	harvest->file_doc = NULL;
	harvest->file_site = NULL;
	harvest->file_path = NULL;
	harvest->file_sitename = NULL;


	// Check readonly mode
	if( !harvest->readonly ) {
		harvest_save_info( harvest );
	}

	// Free lists
	if( harvest->doc_list != NULL ) {
		free( harvest->doc_list );
	}
	if( harvest->site_list != NULL ) {
		free( harvest->site_list );
	}

	// Free memory
	free( harvest );
}

//
// Name: harvest_remove
//
// Description:
//   Removes files in a harvest
//
// Input:
//   dirname - the directory
//   harvestid - the harvestid
//   

void harvest_remove( const char *dirname, int harvesterid ) {
	queue<string> files;

	// Push files
	files.push( HARVESTIDX_FILENAME_MAIN );
	files.push( HARVESTIDX_FILENAME_DOC );
	files.push( HARVESTIDX_FILENAME_SITE );
	files.push( HARVESTIDX_FILENAME_PATH );
	files.push( HARVESTIDX_FILENAME_SITENAME );

	// Delete main files
	while( ! files.empty() ) {

		// Create filename
		char filename[MAX_STR_LEN];
		sprintf( filename, files.front().c_str(), dirname, harvesterid );

		// Delete file
		int rc = unlink( filename );
		if( rc != 0 && errno != ENOENT ) {
			perror( files.front().c_str() );
			die( "Couldn't unlink file" );
		}

		// Remove file from queue
		files.pop();

	}

	// Remove fetcher files
	harvest_remove_files( dirname, harvesterid );
}

void harvest_remove_files( const char *dirname, int harvesterid ) {
	assert( harvesterid > 0 );

	// Delete fetcher files
	char dir_fetcher[MAX_STR_LEN];
	sprintf( dir_fetcher, "%s/%d", dirname, harvesterid );

	// Open directory
	DIR *dir = opendir( dir_fetcher );
	if( dir != NULL ) {
		struct dirent *file;
		struct stat64 statbuf;

		// Read the directory
		while( (file = readdir(dir)) ) {
			char filename[MAX_STR_LEN];
			sprintf( filename, "%s/%s", dir_fetcher, file->d_name );
			int rc = stat64( filename, &(statbuf) );

			// Check for normal files
			if( rc == 0 ) {
				if( S_ISREG(statbuf.st_mode) ) {
					int rcunlink = unlink( filename );
					if( rcunlink != 0 && errno != ENOENT ) {
						perror( filename );
						die( "Couldn't unlink file" );
					}
				}
			}

		}
		closedir( dir );

		// Remove the directory
		int rc = rmdir( dir_fetcher );
		if( rc < 0 ) {
			perror( dir_fetcher );
			die( "Couldn't remove dir_fetcher" );
		}
	}
}

//
// Name: harvest_remove_all
//
// Description:
//   Removes all harvests
//
// Input:
//   dirname - the directory
//

void harvest_remove_all( const char *dirname ) {
	int i = 1;

	while( harvest_exists( dirname, i ) ) {
		harvest_remove( dirname, i );
		i++;
	}
}

//
//
// Name: harvest_append_doc
//
// Description:
//   Appends a document at the end of the harvest list
//
// Input:
//   harvest - the harvest object
//   doc - the document to append
//

void harvest_append_doc( harvest_t *harvest, doc_t *doc, char *path ) {
	assert( harvest != NULL );
	assert( doc != NULL );
	assert( harvest->file_doc != NULL );
	assert( harvest->file_path != NULL );
	assert( harvest->doc_list == NULL );

	int rc;

	// Write document object
	rc = fwrite( doc, sizeof(doc_t), 1, harvest->file_doc );
	assert( rc == 1 );

	// Check path: it cannot contain \n or \r,
	// otherwise, the path file will not be aligned with
	// the docs file
	int len = strlen(path);
	if( len > 0 ) {
		for( int i=0; i<len; i++ ) {
			assert( path[i] != '\n' && path[i] != '\r' );
		}

		// Write path; the paths file for a harvester is a list
		// of paths, one on each line
		rc = fwrite( path, len, 1, harvest->file_path );
		assert( rc > 0 );
	}

	rc = fwrite( "\n", 1, 1, harvest->file_path );
	assert( rc > 0 );

	// Increase document count
	harvest->count ++;
}

//
// Name: harvest_append_site
//
// Description:
//   Appends a site at the end of the harvest site list
//
// Input:
//   harvest - the harvest object
//   site - the site to append
//

void harvest_append_site( harvest_t *harvest, site_t *site, char *sitename ) {
	assert( harvest != NULL );
	assert( site != NULL );
	assert( harvest->file_site != NULL );
	assert( harvest->file_sitename != NULL );
	assert( harvest->site_list == NULL );

	int rc;

	// Write site object
	rc = fwrite( site, sizeof(site_t), 1, harvest->file_site );
	assert( rc == 1 );

	// Write sitename
	rc = fprintf( harvest->file_sitename, "%s\n", sitename );
	assert( rc > 0 );

	// increase number
	harvest->count_site ++;
}

//
// Name: harvest_read_list
//
// Description:
//   Read the lists in memory
//
// Input:
//   harvest - the harvest object
//
// Output:
//   harvest.doc_list - list of documents, sorted by docid
//   harvest.site_list - list of sites, sorted by siteid
//

void harvest_read_list( harvest_t *harvest ) {
	int rc;

	assert( harvest != NULL );
	assert( harvest->doc_list == NULL );
	assert( harvest->site_list == NULL );

	// Get memory
	harvest->doc_list = (doc_t *)malloc((harvest->count) * sizeof(doc_t));
	assert( harvest->doc_list != NULL );
	harvest->site_list = (site_t *)malloc((harvest->count_site) * sizeof(site_t));
	assert( harvest->site_list != NULL );

	// Go to the begining of the document file
	rc = fseeko( harvest->file_doc, (off64_t)0, SEEK_SET );
	assert( rc == 0 );

	// Go to the begining of the path file
	rc = fseeko( harvest->file_path, (off64_t)0, SEEK_SET );
	assert( rc == 0 );

	// Create the map
	harvest->map_path = new map<docid_t,string>;

	// Read documents
	assert( harvest->count > 0 );
	char path[MAX_STR_LEN+1];
	char c;
	int pathlen;
	for( uint i=0; i<harvest->count; i++ ) {

		// Read the document object
		rc = fread( &((harvest->doc_list)[i]), sizeof(doc_t), 1, harvest->file_doc );
		if( rc != 1 ) {
			cerr << "Readed " << i << " documents, expected " << harvest->count << endl;
			die( "Couldn't read doc list (changes in doc_t ?)" );
		}

		// Read the path
		// We don't use fscanf or fgets as there still may be spurious characters
		// in the input
		pathlen = 0;
		while( (c = fgetc( harvest->file_path )) != '\n' ) {
			path[pathlen++] = c;
			if( pathlen > MAX_STR_LEN ) {
				cerr << "A long path was found in " << HARVESTIDX_FILENAME_DOC << endl;
				die( "A long path was found" );
			}
		}
		path[pathlen] = '\0';
		
		// Store the path
		docid_t docid = ((harvest->doc_list)[i]).docid;
		(*(harvest->map_path))[docid] = path;

	}

	char *rc_fgets = fgets( path, MAX_STR_LEN, harvest->file_path );

	// Check that we've reached the end of the file
	if( rc_fgets != NULL ) {
		cerr << "There are extra lines at the end of " << HARVESTIDX_FILENAME_DOC << endl;
		die( "The list of documents was not exhausted" );
	}

	// Go to the begining of the site file
	rc = fseeko( harvest->file_site, (off64_t)0, SEEK_SET );
	assert( rc == 0 );

	// Go to the begining of the sitename file
	rc = fseeko( harvest->file_sitename, (off64_t)0, SEEK_SET );
	assert( rc == 0 );

	// Create the maps
	harvest->map_site     = new map<siteid_t,site_t>;
	harvest->map_sitename = new map<siteid_t,string>;

	// Read sites
	assert( harvest->count_site > 0 );
	for( uint i=0; i<harvest->count_site; i++ ) {

		// Read the site object
		rc = fread( &((harvest->site_list)[i]), sizeof(site_t), 1, harvest->file_site );
		if( rc != 1 ) {
			cerr << "Readed " << i << " sites, expected " << harvest->count_site << endl;
			die( "Couldn't read site list (changes in site_t ?)" );
		}

		// Read the sitename
		char sitename[MAX_STR_LEN];
		fgets( sitename, MAX_STR_LEN, harvest->file_sitename );

		assert( strlen(sitename) > 0 );

		// Remove trailing newline
		sitename[strlen(sitename)-1] = '\0';

		// Store the sitename
		siteid_t siteid = ((harvest->site_list)[i]).siteid;
		(*(harvest->map_sitename))[siteid] = sitename;

		// Store the site in the map
		(*(harvest->map_site))[siteid] = (harvest->site_list)[i];

	}
}

//
// Name: harvest_read_list_doc_only
//
// Description:
//   Read only docids (no paths, no site names)
//
// Input:
//   harvest - the harvest object
//
// Output:
//   harvest.doc_list - list of documents, sorted by docid
//

void harvest_read_list_doc_only( harvest_t *harvest ) {
	int rc;

	assert( harvest != NULL );
	assert( harvest->doc_list == NULL );

	// Get memory
	harvest->doc_list = (doc_t *)malloc((harvest->count) * sizeof(doc_t));
	assert( harvest->doc_list != NULL );

	// Go to the begining of the document file
	rc = fseeko( harvest->file_doc, (off64_t)0, SEEK_SET );
	assert( rc == 0 );

	// Read documents
	assert( harvest->count > 0 );
	for( uint i=0; i<harvest->count; i++ ) {

		// Read the document object
		rc = fread( &((harvest->doc_list)[i]), sizeof(doc_t), 1, harvest->file_doc );
		if( rc != 1 ) {
			cerr << "Readed " << i << " documents, expected " << harvest->count << endl;
			die( "Couldn't read doc list (changes in doc_t ?)" );
		}

	}
}
//
// Name: manager_compare_by_siteid
//
// Descripcion:
//   Compare by siteid function for stl
//
// Input:
//   a,b - sites
//
// Output:
//   true if a.siteid <= b.siteid
//

int harvest_compare_by_siteid( const void *a, const void *b ) {
	return( ((const site_t *)a)->siteid <= 
	        ((const site_t *)b)->siteid );
}

//
// Name: harvest_dump_status
//
// Description:
//   Shows the status of the harvest
//
// Input:
//   harvest - structure
//
void harvest_dump_status(harvest_t *harvest) {
	assert( harvest != NULL );
	cerr << " Begin status dump for harvest #" << harvest->id << endl;
	cerr << "  - status        : " << HARVEST_STATUS_STR(harvest->status) << endl;
	cerr << "  - count         : " << harvest->count << endl;
	cerr << "  - count_site    : " << harvest->count_site << endl;
	cerr << "  - count_ok      : " << harvest->count_ok << endl;
	cerr << "  - raw_size      : " << harvest->raw_size << endl;
	cerr << "  - size          : " << harvest->size << endl;
	cerr << "  - link_total    : " << harvest->link_total << endl;
	cerr << "  - link_new_sites: " << harvest->link_new_sites << endl;
	cerr << "  - link_old_pages: " << harvest->link_old_pages << endl;
	cerr << "  - link_new_pages: " << harvest->link_new_pages << endl;
	cerr << "  - newlinks/total: " << harvest->links_ratio << " (link_new_pages / link_new + link_old )" << endl;
	cerr << endl;
	cerr << "  - hostname      : " << harvest->hostname << endl;
	cerr << "  - creationtime  : " << ctime(&(harvest->creationtime));
	cerr << "  - begintime     : " << ctime(&(harvest->begintime));
	cerr << "  - endtime       : " << ctime(&(harvest->endtime));
	cerr << "  - bytes in      : " << harvest->bytes_in << endl;
	cerr << "  - bytes out     : " << harvest->bytes_out << endl;
	cerr << "  - speed_ok      : " << harvest->speed_ok << " (docs/sec)" << endl;
	cerr << " End status dump for harvest #" << harvest->id << endl;
}

//
// Name: harvest_dump_list
//
// Description:
//   Shows the list of the harvest
//
// Input:
//   harvest - structure
//
void harvest_dump_list(harvest_t *harvest) {
	assert( harvest != NULL );

	// Read list
	harvest_read_list( harvest );

	// Iterate
	for( uint i=0; i<harvest->count; i++ ) {

		// Get document
		doc_t *doc = &((harvest->doc_list)[i]);

		// Print
		cerr << doc->docid << " ";
		cerr << doc->siteid << " ";
		cerr << (*(harvest->map_sitename))[doc->siteid] << "/";
		cerr << (*(harvest->map_path))[doc->docid] << endl;
	}
}


//
// Name: harvest_analyze_harvests
//
// Description:
//   Analyzes data about the harvests
//
// Input:
//   output - descriptor
//

void harvest_analyze( metaidx_t *metaidx ) {
	assert( metaidx != NULL );
	docid_t	ndocs	= metaidx->count_doc;

	uint *harvestids = (uint *)malloc(sizeof(uint)*(ndocs + 1));

	uint nharvests = 1;
	while( harvest_exists( COLLECTION_HARVEST, nharvests ) ) {
		nharvests ++;
	}
	nharvests --;

	assert( nharvests > 0 );

	cerr << "Harvest rounds to analyze : " << nharvests << endl;

	double *sum_pagerank	= (double *)malloc(sizeof(double)*(nharvests+1) );
	assert( sum_pagerank != NULL );
	double *sum_wlrank	= (double *)malloc(sizeof(double)*(nharvests+1) );
	assert( sum_wlrank != NULL );
	double *sum_hubrank	= (double *)malloc(sizeof(double)*(nharvests+1) );
	assert( sum_hubrank != NULL );
	double *sum_authrank	= (double *)malloc(sizeof(double)*(nharvests+1) );
	assert( sum_authrank != NULL );
	docid_t *sum_in_degree	= (docid_t *)malloc(sizeof(docid_t)*(nharvests+1) );
	assert( sum_in_degree != NULL );
	uint *sum_depth	= (uint *)malloc(sizeof(uint)*(nharvests+1) );
	assert( sum_depth != NULL );
	docid_t *sum_ok	= (docid_t *)malloc(sizeof(docid_t)*(nharvests+1) );
	assert( sum_ok != NULL );

	for( docid_t docid=1; docid<=ndocs; docid++ ) {
		harvestids[docid]	= 0;
	}

	// Iterate through harvest rounds
	// to see to which round a document belong.
	// Mark the document with the last round in which it was
	// active.
	uint nharvests_div_50	= nharvests / 50;
	cerr << "Reading harvests    |--------------------------------------------------|" << endl;
	cerr << "                     ";
	uint harvest_id	= 1;
	while( harvest_exists( COLLECTION_HARVEST, harvest_id ) ) {

		if( nharvests_div_50 > 0 && harvest_id % nharvests_div_50 == 0 ) {
			cerr << ".";
		}

		// Clean accumulator variables
		sum_pagerank[harvest_id]			= 0;
		sum_wlrank[harvest_id]				= 0;
		sum_hubrank[harvest_id]				= 0;
		sum_authrank[harvest_id]			= 0;

		sum_in_degree[harvest_id]			= 0;
		sum_depth[harvest_id]				= 0;
		sum_ok[harvest_id]					= 0;

		// Open in read-only mode
		harvest_t *harvest = harvest_open( COLLECTION_HARVEST, harvest_id, true );

		if( harvest->status == STATUS_HARVEST_SEEDED  ) {

			// Read list of documents
			harvest_read_list_doc_only( harvest );

			for( uint i=0; i<harvest->count; i++ ) {
				doc_t *doc	= &((harvest->doc_list)[i]);
				harvestids[doc->docid]	= harvest_id;
			}
		}

		// Close
		harvest_close( harvest );

		// Next
		harvest_id ++;
	}
	cerr << "ok." << endl;

	// Iterate through documents
	docid_t ndocs_div_50    = ndocs / 50;
	cerr << "Analyzing docs      |--------------------------------------------------|" << endl;
	cerr << "                     ";

	

	doc_t doc;
	for( docid_t docid=1; docid<=ndocs; docid++ ) {
		doc.docid	= docid;
		if( ndocs_div_50 > 0 && doc.docid % ndocs_div_50 == 0 ) {
			cerr << ".";
		}
		metaidx_doc_retrieve( metaidx, &(doc) );

		if( HTTP_IS_OK( doc.http_status ) || HTTP_IS_REDIRECT( doc.http_status ) ) {

			sum_pagerank[harvestids[doc.docid]]		+= (double)(doc.pagerank);
			sum_wlrank[harvestids[doc.docid]]		+= (double)(doc.wlrank);
			sum_hubrank[harvestids[doc.docid]]		+= (double)(doc.hubrank);
			sum_authrank[harvestids[doc.docid]]		+= (double)(doc.authrank);

			sum_in_degree[harvestids[doc.docid]]	+= doc.in_degree;
			sum_depth[harvestids[doc.docid]]		+= (uint)(doc.depth);

			sum_ok[harvestids[doc.docid]]			++;

		}
	}
	cerr << " ok." << endl;


	//
	// Write data
	//

	char filename[MAX_STR_LEN];
	FILE *output;

	// Create output dir
	sprintf( filename, "%s/harvest", COLLECTION_ANALYSIS );
	createdir( filename );

	// Write harvest data
	sprintf( filename, "%s/harvest/harvest_analysis.csv", COLLECTION_ANALYSIS );
	output	= fopen64( filename, "w" );
	assert( output != NULL );
	harvest_dump_data_header( output );
	for( uint i=1; i<=nharvests; i++ ) {

		// Open the harvest
		harvest_t *harvest = harvest_open( COLLECTION_HARVEST, i, false );
		assert( harvest != NULL );

		// Copy the new data
		harvest->sum_pagerank		= sum_pagerank[i];
		harvest->sum_wlrank			= sum_wlrank[i];
		harvest->sum_hubrank		= sum_hubrank[i];
		harvest->sum_authrank		= sum_authrank[i];

		harvest->sum_in_degree		= sum_in_degree[i];
		harvest->sum_depth			= sum_depth[i];
		harvest->sum_ok				= sum_ok[i];

		// Re-calc some data
		harvest->speed_ok     = (double)harvest->count_ok
			/ (double)(harvest->endtime - harvest->begintime);

		harvest->links_ratio  = (double)harvest->link_new_pages
			/ (double)(harvest->link_new_pages + harvest->link_old_pages);

		// Show data
		harvest_dump_data( harvest, output );

		// Save
		harvest_close( harvest );
	}
	fclose( output );

	// Free
	free( harvestids );

	free( sum_pagerank );
	free( sum_wlrank );
	free( sum_hubrank );
	free( sum_authrank );

	free( sum_in_degree );
	free( sum_depth );
	free( sum_ok );
}

//
// Name: harvest_dump_harvests
//
// Description:
//   Dumps all the data about harvesters to the specified file
//
// Input:
//   output - descriptor
//

void harvest_dump_harvests( FILE *output ) {
	assert( output != NULL );

	uint harvest_id = 1;

	// We don't know the number of harvests, so we have to
	// check for them
	harvest_dump_data_header( output );

	while( harvest_exists( COLLECTION_HARVEST, harvest_id ) ) {
		harvest_t *harvest = harvest_open( COLLECTION_HARVEST, harvest_id, true );
		assert( harvest != NULL );

		harvest_dump_data( harvest, output );

		// Close
		harvest_close( harvest );
		harvest_id++;
	}
}

//
// Name: harvest_dump_data_header
//
// Description:
//   Shows the header for harvest_dump_data, MUST have fields
//   in the SAME order
//

void harvest_dump_data_header( FILE *output ) {
	assert( output != NULL );
	fprintf( output, "1id," );
	fprintf( output, "2count," );
	fprintf( output, "3count_ok," );
	fprintf( output, "4count_site," );
	fprintf( output, "5raw_size," );
	fprintf( output, "6size," );
	fprintf( output, "7link_total," );
	fprintf( output, "8link_old_pages," );
	fprintf( output, "9link_new_pages," );
	fprintf( output, "10link_new_sites," );
	fprintf( output, "11creationtime," );
	fprintf( output, "12begintime," );
	fprintf( output, "13endtime," );
	fprintf( output, "14status," );
	fprintf( output, "15sum_pagerank," );
	fprintf( output, "16sum_wlrank," );
	fprintf( output, "17sum_hubrank," );
	fprintf( output, "18sum_authrank," );
	fprintf( output, "19sum_in_degree," );
	fprintf( output, "20sum_depth," );
	fprintf( output, "21sum_ok," );
	fprintf( output, "22speed_ok," );
	fprintf( output, "23links_ratio," );
	fprintf( output, "24bytes_in," );
	fprintf( output, "25bytes_out," );
	fprintf( output, "26hostname\n" );
}

//
// Name: harvest_dump_data
//
// Description:
//   Dumps the status of a harvest to a text-only database, for analysis.
//
// Input:
//   harvest - structure
//
void harvest_dump_data(harvest_t *harvest, FILE *output ) {
	assert( harvest != NULL );
	assert( output != NULL );
	fprintf( output, "%d,", harvest->id );
	fprintf( output, "%d,", harvest->count );
	fprintf( output, "%d,", harvest->count_ok );
	fprintf( output, "%d,", harvest->count_site );
	fprintf( output, "%lu,", (long unsigned int)(harvest->raw_size) );
	fprintf( output, "%lu,", (long unsigned int)(harvest->size) );
	fprintf( output, "%d,", harvest->link_total );
	fprintf( output, "%d,", harvest->link_old_pages );
	fprintf( output, "%d,", harvest->link_new_pages );
	fprintf( output, "%d,", harvest->link_new_sites );
	fprintf( output, "%d,", (int)(harvest->creationtime) );
	fprintf( output, "%d,", (int)(harvest->begintime) );
	fprintf( output, "%d,", (int)(harvest->endtime) );
	fprintf( output, "%s,", HARVEST_STATUS_STR(harvest->status) );
	fprintf( output, "%e,", harvest->sum_pagerank );
	fprintf( output, "%e,", harvest->sum_wlrank );
	fprintf( output, "%e,", harvest->sum_hubrank );
	fprintf( output, "%e,", harvest->sum_authrank );
	fprintf( output, "%lu,", harvest->sum_in_degree );
	fprintf( output, "%d,", harvest->sum_depth );
	fprintf( output, "%lu,", harvest->sum_ok );
	fprintf( output, "%e,", harvest->speed_ok );
	fprintf( output, "%e,", harvest->links_ratio );
	fprintf( output, "%lu,", (long unsigned int)(harvest->bytes_in) );
	fprintf( output, "%lu,", (long unsigned int)(harvest->bytes_out) );
	fprintf( output, "%s\n", harvest->hostname );
}
