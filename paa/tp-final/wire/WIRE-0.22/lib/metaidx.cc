
#include "metaidx.h"

//
// Name: metaidx_open
//
// Description:
//   Opens a metaidx, creates it if necessary
//
// Input:
//   dirname - directory to create metaidx
//   readonly - readonly flag
//
// Return:
//   the metaidx object 
//

void metaidx_convert_old_site_file( char *infile ) {
	char outfile[MAX_STR_LEN];
	sprintf( outfile, "%s.tmp", infile );

	cerr << "Converting site file ... ";
	// Create file
	FILE *in = fopen64( infile, "r" );
	assert( in != NULL );
	FILE *out = fopen64( outfile, "w" );
	assert( out != NULL );

	site_old_t site_old;
	site_t site_new;

	metaidx_site_default( &(site_new) );

	for(siteid_t i=0; i<CONF_COLLECTION_MAXSITE; i++ ) {
		fread( &site_old, sizeof(site_old_t), 1, in );
		site_new.siteid				= site_old.siteid;
		site_new.count_doc			= site_old.count_doc;
		site_new.count_error		= site_old.count_error;
		site_new.last_visit			= site_old.last_visit;
		site_new.last_resolved		= site_old.last_resolved;
		memcpy( &(site_new.addr), &(site_old.addr), sizeof(struct in_addr) );
		site_new.raw_content_length	= site_old.raw_content_length;
		site_new.harvest_id			= site_old.harvest_id;
		site_new.has_valid_robots_txt		= site_old.has_exclusions;
		site_new.status				= site_old.status;

		site_new.count_doc_ok		= site_old.count_doc_ok;
		site_new.count_doc_gathered	= site_old.count_doc_gathered;
		site_new.count_doc_new		= site_old.count_doc_new;
		site_new.count_doc_assigned	= site_old.count_doc_assigned;
		site_new.count_doc_static	= site_old.count_doc_static;
		site_new.count_doc_dynamic	= site_old.count_doc_dynamic;
		site_new.siterank			= site_old.siterank;
		site_new.age_oldest_page	= site_old.age_oldest_page;
		site_new.age_newest_page	= site_old.age_newest_page;
		site_new.age_average_page	= site_old.age_average_page;
		site_new.in_degree			= site_old.in_degree;
		site_new.max_depth			= site_old.max_depth;
		site_new.component			= site_old.component;
		site_new.internal_links		= site_old.internal_links;
		site_new.out_degree			= site_old.out_degree;
		site_new.count_doc_gathered			= site_old.count_doc_gathered;
		site_new.count_doc_new			= site_old.count_doc_new;
		site_new.count_doc_assigned			= site_old.count_doc_assigned;
		site_new.count_doc_ignored			= site_old.count_doc_ignored;

		site_new.sum_pagerank		= site_old.sum_pagerank;
		site_new.sum_hubrank		= site_old.sum_hubrank;
		site_new.sum_authrank		= site_old.sum_authrank;

		fwrite( &site_new, sizeof(site_t), 1, out );
	}
	cerr << "done" << endl;

	fclose( in );
	fclose( out );

	rename( outfile, infile );

}

void metaidx_convert_old_doc_file( char *infile ) {
	char outfile[MAX_STR_LEN];
	sprintf( outfile, "%s.tmp", infile );

	cerr << "Converting doc file ... ";
	// Create file
	FILE *in = fopen64( infile, "r" );
	assert( in != NULL );
	FILE *out = fopen64( outfile, "w" );
	assert( out != NULL );

	doc_old_t doc_old;
	doc_t doc_new;

	metaidx_doc_default( &(doc_new) );

	while( fread( &doc_old, sizeof(doc_old_t), 1, in ) > 0 ) {

		doc_new.docid				= doc_old.docid;
		doc_new.siteid				= doc_old.siteid;
		doc_new.status       	  	= doc_old.status;
		doc_new.mime_type			= doc_old.mime_type;
		doc_new.http_status        	= doc_old.http_status;

		doc_new.effective_speed    	= doc_old.effective_speed;
		doc_new.latency				= 0;
		doc_new.latency_connect		= 0;

		doc_new.number_visits      	= doc_old.number_visits;
		doc_new.number_visits_changed    = doc_old.number_visits_changed;

		doc_new.time_unchanged     	= doc_old.time_unchanged;
		doc_new.first_visit        	= doc_old.first_visit;
		doc_new.last_visit			= doc_old.last_visit;
		doc_new.last_modified      	= doc_old.last_modified;

		doc_new.raw_content_length 	= doc_old.raw_content_length;
		doc_new.content_length     	= doc_old.content_length;
		doc_new.hash_value			= doc_old.hash_value;
		doc_new.duplicate_of		= doc_old.duplicate_of;

		doc_new.depth              	= doc_old.depth;
		doc_new.is_dynamic			= doc_old.is_dynamic;

		doc_new.in_degree			= doc_old.in_degree;
		doc_new.out_degree			= doc_old.out_degree;

		doc_new.pagerank			= doc_old.pagerank;
		doc_new.wlrank            	= doc_old.wlrank;
		doc_new.hubrank           	= doc_old.hubrank;
		doc_new.authrank          	= doc_old.authrank;

		doc_new.freshness			= doc_old.freshness;
		doc_new.current_score		= doc_old.current_score;
		doc_new.future_score		= doc_old.future_score;

		fwrite( &doc_new, sizeof(doc_t), 1, out );
	}
	cerr << "done" << endl;

	fclose( in );
	fclose( out );

	rename( outfile, infile );

}

metaidx_t *metaidx_open( const char *dirname, bool readonly ) {
	char filename[MAX_STR_LEN];
	struct stat64 statbuf;

	// Request memory for the structure
	metaidx_t *m = (metaidx_t *)malloc(sizeof(metaidx_t));
	assert( m != NULL );

	// Copy directory name
	assert( strlen(dirname) < MAX_STR_LEN );
	strcpy( m->dirname, dirname );
	m->readonly = readonly;

	// Create or open site filename
	sprintf( filename, "%s/%s", dirname, METAIDX_FILENAME_SITE );
	if( stat64( filename, &statbuf ) == 0 ) {
		// Verify file size
		if( (unsigned long)(statbuf.st_size) != (sizeof(site_t) * CONF_COLLECTION_MAXSITE) ) {
			if( (unsigned long)(statbuf.st_size) != (sizeof(site_old_t) * CONF_COLLECTION_MAXSITE) ) {
				cerr << "Inconsistency in site_t:" << endl;
				cerr << "- Sizeof(site_t)       : " << sizeof(site_t) << endl;
				cerr << "- Sizeof(site_old_t)   : " << sizeof(site_old_t) << endl;
				cerr << "- Maxsites             : " << CONF_COLLECTION_MAXSITE << endl;
				cerr << "- Size of current file : " << (unsigned long)(statbuf.st_size) << endl;
				cerr << "- Expected size        : " << sizeof(site_t) * CONF_COLLECTION_MAXSITE << endl;
				die( "Inconsistency in site_t" );
			} else {
				metaidx_convert_old_site_file( filename );
			}
		}

		if( m->readonly ) {
			m->file_site = fopen64( filename, "r" );
		} else {
			// Open read/write
			m->file_site = fopen64( filename, "r+" );
		}

		if( m->file_site == NULL ) {
			perror( filename );
			die( "opening metaidx" );
		}

		// Read the header site
		int rc = fseeko( m->file_site, (off64_t)0, SEEK_SET );
		assert( rc == 0 );

		site_t site_header;
		rc = fread( &(site_header), sizeof(site_t), 1, m->file_site );

		// Read data from it
		m->count_site = site_header.siteid;


	} else {

		// Check readonly
		if( m->readonly ) {
			die( "Couldn't create index: readonly mode enabled" );
		}

		// Create file
		cerr << "Creating site file for " << CONF_COLLECTION_MAXSITE << " sites ... ";
		m->file_site = fopen64( filename, "w+" );
		site_t site;
		site.siteid = 0;
		for(siteid_t i=0; i<CONF_COLLECTION_MAXSITE; i++ ) {
			fwrite( &site, sizeof(site_t), 1, m->file_site );
		}
		cerr << "done." << endl;

		// Initialize
		m->count_site = 0;
	}

	if( m->file_site == NULL ) {
		perror( filename );
	}
	assert( m->file_site != NULL );

	// Create or open doc filename
	sprintf( filename, "%s/%s", dirname, METAIDX_FILENAME_DOC );

	// Check if file exists
	if( stat64( filename, &statbuf ) == 0 ) {

		// Verify file size
		if( (unsigned long)(statbuf.st_size) != (sizeof(doc_t) * CONF_COLLECTION_MAXDOC)) {
			if( (unsigned long)(statbuf.st_size) != (sizeof(doc_old_t) * CONF_COLLECTION_MAXDOC)) {
				cerr << "The size of the file of metadata of documents is wrong" << endl;
				cerr << "Probably you changed maxdoc in the configuration" << endl;
				cerr << "Or the version of WIRE is wrong" << endl;
				die( "Inconsistency in the size of doc_t" );
			} else {
				metaidx_convert_old_doc_file( filename );
			}
		}

		if( m->readonly ) {
			m->file_doc = fopen64( filename, "r" );
		} else {
			// Open read/write
			m->file_doc = fopen64( filename, "r+" );
		}
		assert( m->file_doc != NULL );

		// Read the header document
		int rc = fseeko( m->file_doc, (off64_t)0, SEEK_SET );
		assert( rc == 0 );

		doc_t doc_header;
		rc = fread( &(doc_header), sizeof(doc_t), 1, m->file_doc );
		
		// Read data from it
		m->count_doc = doc_header.docid;


	} else {

		// Check readonly
		if( m->readonly ) {
			die( "Couldn't create index: readonly mode enabled" );
		}

		// Create file
		cerr << "Creating doc file for " << CONF_COLLECTION_MAXDOC << " documents ... ";
		m->file_doc = fopen64( filename, "w+" );
		doc_t doc;
		doc.docid = 0;
		for(docid_t i=0; i<CONF_COLLECTION_MAXDOC; i++ ) {
			fwrite( &doc, sizeof(doc_t), 1, m->file_doc );
		}
		cerr << "done." << endl;

		// Initialize data
		m->count_doc = 0;
	}

	if( m->file_doc == NULL ) {
		perror( filename );
	}
	assert( m->file_doc != NULL );

	return m;
}

//
// Name: metaidx_close
//
// Description:
//   Closes a structure. Writes data to disk.
//
// Input:
//   metaidx - the metaidx structure
//
// Return: 
//   status code
//

metaidx_status_t metaidx_close( metaidx_t *m ) {
	assert( m->file_site != NULL );
	assert( m->file_doc != NULL );

	if( !m->readonly ) {
		int rc;
		doc_t doc_header;
		site_t site_header;

		// Generate data for headers
		doc_header.docid   = m->count_doc;
		site_header.siteid = m->count_site;

		// Write headers
		rc = fseeko( m->file_doc, (off64_t)0, SEEK_SET );
		assert( rc == 0 );
		rc = fwrite( &(doc_header), sizeof(doc_t), 1, m->file_doc );
		assert( rc == 1 );

		rc = fseeko( m->file_site, (off64_t)0, SEEK_SET );
		assert( rc == 0 );
		rc = fwrite( &(site_header), sizeof(site_t), 1, m->file_site );
		assert( rc == 1 );

		// Test if there is corruption
		if( m->count_doc > 1 ) {
			doc_t test_document;
			test_document.docid = (docid_t)1;
			int rc = metaidx_doc_retrieve( m, &(test_document) );
			if( rc != METAIDX_OK ) {
				die( "Metaidx corruption: at closing time, the metaidx has more than 1 document but document 1 can not be read!" );
			}
			if( test_document.docid != (docid_t)1 ) {
				die( "Metaidx corruption: at closing time, document 1 was read but had the wrong docid!" );
			}
		}
	}

	// Close files
	fclose( m->file_site );
	fclose( m->file_doc );

	// Free
	free(m);

	return METAIDX_OK;
}

//
// Name: metaidx_remove
//
// Description:
//   Removes files in a metaidx
//
// Input:
//   dirname - the directory
//   

void metaidx_remove( const char *dirname ) {
	queue<string> files;

	// Push files
	files.push( METAIDX_FILENAME_SITE );
	files.push( METAIDX_FILENAME_DOC );

	// Delete
	while( ! files.empty() ) {

		// Create filename
		char filename[MAX_STR_LEN];
		sprintf( filename, "%s/%s", dirname, files.front().c_str() );

		// Delete file
		int rc = unlink( filename );
		if( rc != 0 && errno != ENOENT ) {
			perror( files.front().c_str() );
			die( "Couldn't unlink file" );
		}

		// Remove file from queue
		files.pop();

	}
}

//

//
// Name: metaidx_site_store
//
// Description:
//   Stores information about a site
//
// Input:
//    site - the site to be stored
// 
// Return:
//    status code
//


metaidx_status_t metaidx_site_store( metaidx_t *m, site_t *site ) {
	assert( site->siteid > 0 );
	assert( site->siteid < CONF_COLLECTION_MAXSITE );
	assert( !m->readonly );
	int rc;

	// Seek
	rc = fseeko( m->file_site, (off64_t)(sizeof(site_t) * site->siteid), SEEK_SET );
	assert( rc == 0 );

	// Write
	rc = fwrite( site, sizeof(site_t), 1, m->file_site );
	assert( rc == 1 );

	// Update count
	if( site->siteid > m->count_site ) {
		m->count_site = site->siteid;
	}

	return METAIDX_OK;
}


// 
// Name: metaidx_site_retrieve
//
// Description:
//   Retrieves information about a site
//
// Input:
//   metaidx - metaindex holding the information
//   site.siteid - site id of the retrieved site
//
// Output:
//   site - site
//
// Return:
//

metaidx_status_t metaidx_site_retrieve( metaidx_t *m, site_t *site ) {
	assert( site->siteid > 0 );
	int rc;

	// Check number of sites
	if( site->siteid > m->count_site ) {
		return METAIDX_EOF;
	}

	// Seek
	rc = fseeko( m->file_site, (off64_t)(sizeof(site_t) * site->siteid), SEEK_SET );
	assert( rc == 0 );

	// Read
	rc = fread( site, sizeof(site_t), 1, m->file_site );
	assert( rc == 1 );

	if( site->siteid == 0 ) {
		return METAIDX_EOF;
	} else {
		return METAIDX_OK;
	}
}

//
// Name: metaidx_doc_store
//
// Description:
//   Stores information about a document
//
// Input:
//    metaidx - the metaidx structure
//    doc - the document to be stored
// 
// Return:
//    status code
//

metaidx_status_t metaidx_doc_store( metaidx_t *m, doc_t *doc ) {
	assert( doc->docid > 0 );
	assert( doc->siteid > 0 );
	assert( doc->docid < CONF_COLLECTION_MAXDOC );
	assert( doc->siteid < CONF_COLLECTION_MAXSITE );
	assert( !m->readonly );
	int rc;

	// Seek
	off64_t address = ((off64_t)sizeof(doc_t)) * (doc->docid);
	rc = fseeko( m->file_doc, address, SEEK_SET );

	if( rc != 0 ) {
		perror( "fseeko in metadata file of documents" );
		cerr << "Docid was " << doc->docid << endl;
		cerr << "Seek position was " << sizeof(doc_t) * doc->docid << endl;
		die( "fseeko" );
	}

	// Write
	rc = fwrite( doc, sizeof(doc_t), 1, m->file_doc );
	if( rc != 1 ) {
		perror( "fwrite in metadata file of documents" );
		die( "fwrite" );
	}

	// Update document count
	if( doc->docid > m->count_doc ) {
		m->count_doc = doc->docid;
	}

	return METAIDX_OK;
}

// 
// Name: metaidx_doc_retrieve
//
// Description:
//   Retrieves information about a document
//
// Input:
//   metaidx - metaindex holding the information
//   doc.docid - document id of the retrieved document
//
// Output:
//   doc - document
//
// Return:
//   METAIDX_K - success
//   METAIDX_EOF - docid out of range
//

metaidx_status_t metaidx_doc_retrieve( metaidx_t *m, doc_t *doc ) {
	assert( doc->docid > 0 );
	int rc;

	// See if it's inside the range
	if( m->count_doc < doc->docid ) {
		return METAIDX_EOF;
	}

	// Seek
	rc = fseeko( m->file_doc, (off64_t)(sizeof(doc_t) * doc->docid), SEEK_SET );
	if( rc != 0 ) {
		perror( "metaidx_doc_retrieve fseeko" );
		cerr << "Docid is " << doc->docid << endl;
		cerr << "Position is " << (off64_t)(sizeof(doc_t) * doc->docid) << endl;
		die( "fseeko" );
	}

	// Read
	rc = fread( doc, sizeof(doc_t), 1, m->file_doc );
	if( rc != 1 ) {
		perror( "metaidx_doc_retrieve fread" );
		cerr << "** docid " << doc->docid << " **";
		die( "Seek, but couldn't read" );
	}

	if( doc->docid == 0 ) {
		return METAIDX_EOF;
	} else {
		return METAIDX_OK;
	}
}

// 
// Name: metaidx_dump_doc_status
//
// Description:
//   Prints the status of a document
//
// Input:
//   doc - document
//

void metaidx_dump_doc_status( doc_t *doc ) {
	// Main variables
	cerr << "docid              " << doc->docid << endl;
	cerr << "siteid             " << doc->siteid << endl;
	cerr << "status             " << DOC_STATUS_STR(doc->status) << endl;
	cerr << "mime_type          " << MIME_TYPE_STR(doc->mime_type) << endl;

	// Harvester
	cerr << "http_status        " << doc->http_status
		<< " (" << HTTP_STR(doc->http_status) << ": "
		<<	HTTP_IS_STR(doc->http_status) << ")" << endl;
	cerr << "raw_content_length " << doc->raw_content_length << endl;
	cerr << "effective_speed    " << doc->effective_speed << " bytes/sec" << endl;
	cerr << "latency            " << doc->latency << " sec" << endl;
	cerr << "latency_connect    " << doc->latency_connect << " sec" << endl;
	cerr << "number_visits      " << doc->number_visits << endl;
	cerr << "number_visits_changed " << doc->number_visits_changed << endl;
	cerr << "time_unchanged     " << doc->time_unchanged << " sec" << endl;
	cerr << "first_visit        " << ctime(&(doc->first_visit));
	cerr << "last_visit         " << ctime(&(doc->last_visit));
	cerr << "last_modified      " << ctime(&(doc->last_modified));

	// Gatherer
	cerr << "content_length     " << doc->content_length << endl;
	cerr << "hash_value         " << doc->hash_value << endl;
	cerr << "duplicate_of       " << doc->duplicate_of << endl;

	// Seeder
	cerr << "depth              " << doc->depth << endl;
	cerr << "is_dynamic         " <<(doc->is_dynamic?"yes":"no")<< endl;

	// Manager
	cerr << "in_degree          " << doc->in_degree << endl;
	cerr << "pagerank           " << doc->pagerank << endl;
	cerr << "wlrank             " << doc->wlrank << endl;
	cerr << "hubrank            " << doc->hubrank << endl;
	cerr << "authrank           " << doc->authrank << endl;
	cerr << "freshness          " << doc->freshness << endl;
	cerr << "current_score      " << doc->current_score << endl;
	cerr << "future_score       " << doc->future_score << endl;
}

// 
// Name: metaidx_dump_doc_header
//
// Description:
//   Prints a line with the header data
//   for metaidx_dum_doc
//
// Input:
//   out - the output file handler
//

void metaidx_dump_doc_header( FILE *out ) {
	assert( out != NULL );
	
	// The order is very important. It must be consistent with
	// metaidx_dump_doc

	fprintf( out, "1docid," );
	fprintf( out, "2siteid," );
	fprintf( out, "3status," );
	fprintf( out, "4mime_type," );
	fprintf( out, "5http_status," );
	fprintf( out, "6raw_content_length," );
	fprintf( out, "7effective_speed," );
	fprintf( out, "8latency," );
	fprintf( out, "9latency_connect," );
	fprintf( out, "10number_visits," );
	fprintf( out, "11number_visits_changed," );
	fprintf( out, "12time_unchanged," );
	fprintf( out, "13first_visit," );
	fprintf( out, "14last_visit," );
	fprintf( out, "15last_modified," );
	fprintf( out, "15content_length," );
	fprintf( out, "17hash_value," );
	fprintf( out, "18duplicate_of," );
	fprintf( out, "19depth," );
	fprintf( out, "20is_dynamic," );
	fprintf( out, "21in_degree," );
	fprintf( out, "22out_degree," );
	fprintf( out, "23pagerank," );
	fprintf( out, "24wlrank," );
	fprintf( out, "25hubrank," );
	fprintf( out, "26authrank," );
	fprintf( out, "27freshness," );
	fprintf( out, "28current_score," );
	fprintf( out, "29future_score\n" );
}
// 
// Name: metaidx_dump_doc
//
// Description:
//   Prints a line with the data about a document
//
// Input:
//   doc - document
//   out - the output file handler
//

void metaidx_dump_doc( doc_t *doc, FILE *out ) {
	assert( doc != NULL );
	assert( out != NULL );

	// The order is very important. It must be consistent with
	// metaidx_dump_doc_header

	fprintf( out, "%lu,",  	doc->docid );
	fprintf( out, "%lu,",  	doc->siteid );
	fprintf( out, "%d,",  	(int)(doc->status) );
	fprintf( out, "%d,",  	(int)(doc->mime_type) );
	fprintf( out, "%d,",  	doc->http_status );
	fprintf( out, "%lu,",  	(long unsigned int)(doc->raw_content_length) );
	fprintf( out, "%f,",  	doc->effective_speed );
	fprintf( out, "%f,",  	doc->latency );
	fprintf( out, "%f,",  	doc->latency_connect );
	fprintf( out, "%d,",  	doc->number_visits );
	fprintf( out, "%d,",  	doc->number_visits_changed );
	fprintf( out, "%d,",  	(int)(doc->time_unchanged) );
	fprintf( out, "%d,",  	(int)(doc->first_visit) );
	fprintf( out, "%d,",  	(int)(doc->last_visit) );
	fprintf( out, "%d,",  	(int)(doc->last_modified) );
	fprintf( out, "%lu,",  	(long unsigned int)(doc->content_length) );
	fprintf( out, "%lu,",  	doc->hash_value );
	fprintf( out, "%lu,",  	doc->duplicate_of );
	fprintf( out, "%d,",  	doc->depth );
	fprintf( out, "%d,",  	doc->is_dynamic );
	fprintf( out, "%d,",  	doc->in_degree );
	fprintf( out, "%d,",  	doc->out_degree );
	fprintf( out, "%e,",  	doc->pagerank );
	fprintf( out, "%e,",  	doc->wlrank );
	fprintf( out, "%e,",  	doc->hubrank );
	fprintf( out, "%e,",  	doc->authrank );
	fprintf( out, "%e,",  	doc->freshness );
	fprintf( out, "%e,",  	doc->current_score );
	fprintf( out, "%e\n",  	doc->future_score );
}

// 
// Name: metaidx_dump_doc_short_status
//
// Description:
//   Prints the status of a document in one line
//
// Input:
//   doc - document
//

void metaidx_dump_doc_short_status( doc_t *doc ) {
	// Status
	cerr << DOC_STATUS_STR(doc->status) << " ";
	cerr << MIME_TYPE_STR(doc->mime_type) << " ";
	cerr << HTTP_STR(doc->http_status) << ":"
		<<	HTTP_IS_STR(doc->http_status) << " ";

	// Scores
	cerr << "in " << doc->in_degree << " ";
	cerr << "out " << doc->out_degree << " ";
	cerr << "pr " << doc->pagerank;
	cerr << "wl " << doc->wlrank;
}

// 
// Name: metaidx_dump_site_status
//
// Description:
//   Prints the status of a site
//
// Input:
//   site
//

void metaidx_dump_site_status( site_t *site ) {
	cerr << "siteid             " << site->siteid << endl;
	cerr << "status             " << SITE_STATUS_STR(site->status) << endl;
	cerr << "addr               " << inet_ntoa(site->addr) << endl;

	cerr << endl;
	cerr << "count doc          " << site->count_doc << endl;
	cerr << "count doc static   " << site->count_doc_static << endl;
	cerr << "count doc dynamic  " << site->count_doc_dynamic << endl;
	cerr << "count doc ok       " << site->count_doc_ok << endl;
	cerr << "count error        " << site->count_error << endl;

	cerr << endl;
	cerr << "harvester id       " << site->harvest_id << endl;
	cerr << "last visited       " << ctime(&(site->last_visit));
	cerr << "last resolved      " << ctime(&(site->last_resolved));
	cerr << "resolver latency   " << site->resolver_latency << " sec" << endl;


	cerr << endl;
	cerr << "last checked r.txt " << ctime(&(site->last_checked_robots_txt));
	cerr << "last checked r.rdf " << ctime(&(site->last_checked_robots_rdf));
	cerr << "valid robots txt   " << (site->has_valid_robots_txt
		? "yes" : "no") << endl;
	cerr << "valid robots rdf   " << (site->has_valid_robots_rdf
		? "yes" : "no") << endl;

	cerr << endl;
	cerr << "raw content length " << site->raw_content_length << endl;
	cerr << "bytes in           " << site->bytes_in << endl;
	cerr << "bytes out          " << site->bytes_out << endl;

	cerr << endl;
	cerr << "age oldest page    " << site->age_oldest_page << " (" << (site->age_oldest_page/(60*60*24)) << " days)" << endl;
	cerr << "age newest page    " << site->age_newest_page << " (" << (site->age_newest_page/(60*60*24)) << " days)" << endl;
	cerr << "age average page   " << site->age_average_page << " (" << (site->age_average_page/(60*60*24)) << " days)" << endl;

	cerr << endl;
	cerr << "siterank           " << site->siterank << endl;
	cerr << "in degree          " << site->in_degree << endl;
	cerr << "out degree     "		<< site->out_degree << endl;
	cerr << "internal links     " << site->internal_links << endl;
	cerr << "max depth          " << site->max_depth << endl;
	cerr << "component          " << COMPONENT_STR(site->component) << endl;

}

//
// Name: metaidx_dump_sitelist
//
// Description:
//   Writes the site metadata to a file
//
// Input:
//   metaidx - the metadata index
//   urlidx - an urlidx to extract site names
//

void metaidx_dump_sitelist( metaidx_t *metaidx, urlidx_t *urlidx, FILE *out ) {
	assert( metaidx != NULL );
	assert( urlidx != NULL );
	assert( out != NULL );

	site_t site;
	char sitename[MAX_STR_LEN];
	metaidx_dump_site_header( out );
	for( site.siteid = 1; site.siteid <= metaidx->count_site; site.siteid++ ) {
		metaidx_site_retrieve( metaidx, &(site) );
		
		// The sitename is stored in the urlidx
		urlidx_site_by_siteid( urlidx, site.siteid, sitename );
		assert( sitename != NULL );

		metaidx_dump_site( &(site), sitename, out );
	}
}

//
// Name: metaidx_dump_site_header
//
// Description:
//   Shows the header for metaidx_dump_site, MUST have fields
//   in the SAME order
//   

void metaidx_dump_site_header( FILE *out ) {
	// The order must be consistent with the documentation
	fprintf( out, "1siteid," );
	fprintf( out, "2count_doc," );
	fprintf( out, "3count_error," );
	fprintf( out, "4last_visit," );
	fprintf( out, "5last_resolved," );
	fprintf( out, "6raw_content_length," );
	fprintf( out, "7harvest_id," );
	fprintf( out, "8has_valid_robots_txt," );
	fprintf( out, "9count_doc_ok," );
	fprintf( out, "10count_doc_gathered," );
	fprintf( out, "11count_doc_new," );
	fprintf( out, "12count_doc_assigned," );
	fprintf( out, "13count_doc_static," );
	fprintf( out, "14count_doc_dynamic," );
	fprintf( out, "15count_doc_ignored," );
	fprintf( out, "16siterank," );
	fprintf( out, "17age_oldest_page," );
	fprintf( out, "18age_newest_page," );
	fprintf( out, "19age_average_page," );
	fprintf( out, "20in_degree," );
	fprintf( out, "21out_degree," );
	fprintf( out, "22sum_pagerank," );
	fprintf( out, "23sum_hubrank," );
	fprintf( out, "24sum_authrank," );
	fprintf( out, "25internal_links," );
	fprintf( out, "26max_depth," );
	fprintf( out, "27component," );
	fprintf( out, "28bytes_in," );
	fprintf( out, "29bytes_out," );
	fprintf( out, "30sitename" );
	fprintf( out, "\n" );
}


//
// Name: metaidx_dump_site
//
// Description:
//   Dumps information about a site to a relational database
//
// Input:
//   site
//

void metaidx_dump_site( site_t *site, char *sitename, FILE *out ) {
	assert( site != NULL );
	assert( sitename != NULL );
	assert( out != NULL );

	// The order must be consistent with the documentation
 fprintf( out, "%lu,", site->siteid  ); 
 fprintf( out, "%d,", site->count_doc  ); 
 fprintf( out, "%d,", site->count_error  ); 
 fprintf( out, "%d,", (int)(site->last_visit)  ); 
 fprintf( out, "%d,", (int)(site->last_resolved)  ); 
 fprintf( out, "%lu,", (long unsigned int)(site->raw_content_length) ); 
 fprintf( out, "%d,", site->harvest_id  ); 
 fprintf( out, "%d,", site->has_valid_robots_txt  ); 
 fprintf( out, "%d,", site->count_doc_ok  );
 fprintf( out, "%d,", site->count_doc_gathered  );
 fprintf( out, "%d,", site->count_doc_new  );
 fprintf( out, "%d,", site->count_doc_assigned  );
 fprintf( out, "%d,", site->count_doc_static  );
 fprintf( out, "%d,", site->count_doc_dynamic  );
 fprintf( out, "%d,", site->count_doc_ignored  );
 fprintf( out, "%e,", site->siterank  );
 fprintf( out, "%d,", (int)(site->age_oldest_page) );
 fprintf( out, "%d,", (int)(site->age_newest_page) );
 fprintf( out, "%d,", (int)(site->age_average_page) );
 fprintf( out, "%lu,", site->in_degree  );
 fprintf( out, "%lu,", site->out_degree  );
 fprintf( out, "%e,", site->sum_pagerank  );
 fprintf( out, "%e,", site->sum_hubrank  );
 fprintf( out, "%e,", site->sum_authrank  );
 fprintf( out, "%lu,", site->internal_links  );
 fprintf( out, "%d,", site->max_depth  );
 fprintf( out, "%s,", COMPONENT_STR(site->component) );
 fprintf( out, "%lu,", (unsigned long int)(site->bytes_in) );
 fprintf( out, "%lu,", (unsigned long int)(site->bytes_out) );
 fprintf( out, "%s\n", sitename );
}

//
// Name: metaidx_doc_default
//
// Description:
//   Fills all the default data about a doc
//
// Input:
//   doc - empty doc object
//
// Output:
//   doc - fill doc object
//

void metaidx_doc_default( doc_t *doc ) {
	doc->docid			= (docid_t)0;
	// Siteid is not changed
	doc->status         = STATUS_DOC_NEW;
	doc->mime_type		= MIME_UNKNOWN;
	doc->http_status        = 0;

	doc->effective_speed    = 0;
	doc->latency			= 0;
	doc->latency_connect	= 0;

	doc->number_visits      = 0;
   	doc->number_visits_changed    = 0;

	doc->time_unchanged     = 0;
	doc->first_visit        = 0;
	doc->last_visit			= 0;
	doc->last_modified      = 0;

	doc->raw_content_length = 0;
	doc->content_length     = 0;
	doc->hash_value			= 0;
	doc->duplicate_of		= 0;

	doc->depth              = (depth_t)1;
	doc->is_dynamic			= false;

	doc->in_degree			= 0;
	doc->out_degree			= 0;

	doc->pagerank			= (pagerank_t)0;
	doc->wlrank            = (wlrank_t)0;
	doc->hubrank            = (hubrank_t)0;
	doc->authrank           = (authrank_t)0;

	doc->freshness			= 0;
	doc->current_score		= 0;
	doc->future_score		= 0;
}

//
// Name: metaidx_site_default
//
// Description:
//   Fills all the default data about a site
//
// Input:
//   site - empty site object
//
// Output:
//   site - fill site object
//

void metaidx_site_default( site_t *site ) {

	site->siteid			= (siteid_t)0;
	site->status		= STATUS_SITE_NEW;
	site->harvest_id	= 0;


	site->count_doc				= 0;
	site->count_error		= 0;

	site->count_doc_static		= 0;
	site->count_doc_dynamic		= 0;
	site->count_doc_gathered	= 0;
	site->count_doc_new			= 0;
	site->count_doc_assigned	= 0;
	site->count_doc_ignored		= 0;

	site->age_oldest_page		= (time_t)0;
	site->age_newest_page		= (time_t)0;
	site->age_average_page		= (time_t)0;
	site->raw_content_length= (off64_t)0;

	inet_aton( "0.0.0.0", &(site->addr) );

	site->last_visit		= (time_t)0;
	site->last_resolved 	= (time_t)0;
	site->last_checked_robots_txt 	= (time_t)0;
	site->last_checked_robots_rdf 	= (time_t)0;

	site->docid_robots_txt		= 0;
	site->docid_robots_rdf		= 0;

	site->has_valid_robots_txt	= true;
	site->has_valid_robots_rdf	= true;

	site->in_degree			= 0;
	site->out_degree			= 0;
	site->component			= COMPONENT_UNDEF;
	site->siterank			= (pagerank_t)0;
	site->internal_links	= 0;


	site->sum_pagerank		= 0;
	site->sum_wlrank		= 0;
	site->sum_hubrank		= 0;
	site->sum_authrank		= 0;

	site->bytes_in			= 0;
	site->bytes_out			= 0;
}

//
// Name: metaidx_mime_type
//
// Description:
//   Converts a string to a mime_type_t number, we will base
//   1) in the path, e.g.: "robots.txt" has a special mime-type
//   2) in the mime-type header returned by the web server
//
// Input:
//   path - path of the file downloaded
//   mime_type - string containing (ie.: "text/htm")
// 
// Return:
//   mime_type_t enumerated type integer


mime_type_t metaidx_mime_type( char *path, char *mime_str ) {
	int len;

	// Check for special filenames
	if( !strcmp( path, FILENAME_ROBOTS_TXT ) ) {
		return MIME_ROBOTS_TXT;
	} else if( !strcmp( path, FILENAME_ROBOTS_RDF ) ) {
		return MIME_ROBOTS_RDF;
	}

	// Check if mime_str exists
	if( mime_str == NULL ) {
		return MIME_UNKNOWN;
	}
   
	// Check prefixes
    len	= strlen(mime_str);

	if( len >= 8 && !strncasecmp( mime_str, "text/htm", 8 ) ) {
		return MIME_TEXT_HTML;

	} else if( len >= 9 && !strncasecmp( mime_str, "multipart", 9 ) ) {
		return MIME_TEXT_HTML;

	} else if( len >= 10 && !strncasecmp( mime_str, "text/devil", 10 ) ) {
		return MIME_TEXT_HTML;
		
	} else if( len >= 9 && !strncasecmp( mime_str, "text/plai", 9 ) ) {
		return MIME_TEXT_PLAIN;
		
	} else if( len >= 23 && !strncasecmp( mime_str, "application/x-shockwave", 23 ) ) {
	   return MIME_APPLICATION_FLASH;	

	} else if( len >= 11 && !strncasecmp( mime_str, "application", 11 ) ) {
		return MIME_APPLICATION;

	} else if( len >= 5 && !strncasecmp( mime_str, "audio", 5 ) ) {
		return MIME_AUDIO;

	} else if( len >= 5 && !strncasecmp( mime_str, "image", 5 ) ) {
		return MIME_IMAGE;

	} else if( len >= 5 && !strncasecmp( mime_str, "video", 5 ) ) {
		return MIME_VIDEO;

	} else if( len >= 12 && !strncasecmp( mime_str, "text/vnd.wap", 12 ) ) {
		return MIME_TEXT_WAP;

	} else if( len >= 8 && !strncasecmp( mime_str, "text/rtf", 8 ) ) {
		return MIME_TEXT_RTF;

	} else if( len >= 8 && !strncasecmp( mime_str, "text/xml", 8 ) ) {
		return MIME_TEXT_XML;

	} else if( len >= 10 && strncasecmp( mime_str, "text/x-tex", 10 ) ) {
		return MIME_TEXT_TEX;

	} else if( len >= 11 && strncasecmp( mime_str, "text/x-chdr", 11 ) ) {
		return MIME_TEXT_CHDR;

	} else {
		cerr << "[MIME '" << mime_str << "' UNKNOWN]";
		return MIME_UNKNOWN;
	}
}

//
// Name: metaidx_dump_status
// 
// Description:
//   Dumps the status of the metaidx
//

void metaidx_dump_status( metaidx_t *metaidx ) {
	cerr << "Begin status dump for metaidx" << endl;
	cerr << "- dirname         " << metaidx->dirname << endl;
	cerr << "- count_doc       " << metaidx->count_doc << endl;
	cerr << "- count_site      " << metaidx->count_site << endl;
	cerr << "- bytes per doc   " << sizeof(doc_t) << endl;
	cerr << "- bytes per site  " << sizeof(site_t) << endl;
	cerr << "End status dump for metaidx" << endl;
}

