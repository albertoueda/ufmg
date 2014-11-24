
#include "urlidx.h"

off64_t URLIDX_SITE_SIZE;


//
// Name: urlidx_new
//
// Description:
//   Creates a new url index in the given directory
//
// Input:
//   dirname - directory to create url index
//
// Return:
//   urlindex structure
//

urlidx_t *urlidx_new( const char *dirname ) {
	urlidx_t *u = (urlidx_t *)malloc(sizeof(urlidx_t));

	assert( u!=NULL);
	assert( CONF_OK );

	u->site_hash = (off64_t *)malloc(sizeof(off64_t)*CONF_COLLECTION_MAXSITE);
	assert( u->site_hash != NULL );

	u->path_hash = (off64_t *)malloc(sizeof(off64_t)*CONF_COLLECTION_MAXDOC);
	assert( u->path_hash != NULL );

	// Clean hash tables
	for( siteid_t i=0; i<CONF_COLLECTION_MAXSITE; i++ ) {
		u->site_hash[i] = (off64_t)0;
	}
	for( docid_t i=0; i<CONF_COLLECTION_MAXDOC; i++ ) {
		u->path_hash[i] = (off64_t)0;
	}

	// Set default values
	u->site_count = 0;
	u->path_count = 0;
	u->site_next_char = 1; // start in 1
	u->path_next_char = 1; // start in 1

	// Open files
	char filename[MAX_STR_LEN];

	// path_file
	sprintf( filename, "%s/%s", dirname, URLIDX_FILENAME_PATH );
	u->path_file = fopen64( filename, "w+" );
	assert( u->path_file != NULL );
	fwrite( "\0", 1, 1, u->path_file );   // Write 1 char (starts in position 1)

	
	// site_list
	sprintf( filename, "%s/%s", dirname, URLIDX_FILENAME_SITE_LIST );
	u->site_list = fopen64( filename, "w+" );

	// path_list
	sprintf( filename, "%s/%s", dirname, URLIDX_FILENAME_PATH_LIST );
	u->path_list = fopen64( filename, "w+" );

	// Save info
	strcpy( u->dirname, dirname );

	// Create memory area for site names
	URLIDX_SITE_SIZE = URLIDX_EXPECTED_SITENAME_SIZE * CONF_COLLECTION_MAXSITE;
	u->site = (char *)malloc(sizeof(char)*URLIDX_SITE_SIZE);
	assert( u->site != NULL );

	// Set readonlymode false
	u->readonly	= false;

	// Return
	return u;	
}

//
// Name: urlidx_open
//
// Description:
//   Opens a url index in the given directory, or
//   create a new one if necessary.
//
// Input:
//   dirname - directory in which the url index is located
//   readonly - flag for readonly mode
//
// Return:
//   urlindex structure
//

urlidx_t *urlidx_open( const char *dirname, bool readonly ) {
	// The configuration file has to be open
	assert( CONF_OK );

	// Open the main file
	char filename[MAX_STR_LEN];
	sprintf( filename, "%s/%s", dirname, URLIDX_FILENAME_ALL );
	FILE *file_all = fopen64( filename, "r" );

	if( file_all == NULL ) {
		cerr << "urlidx: creating a new index in " << dirname << endl;
		return urlidx_new( dirname );
	}

	// Malloc
	urlidx_t *u = (urlidx_t *)malloc(sizeof(urlidx_t));

	// The structure is big, so malloc can fail
	assert( u!=NULL);

	// Read the main file
	size_t count;
	count = fread( u, sizeof(urlidx_t), 1, file_all );
	assert( count == 1 );
	fclose( file_all );

	// Set readonly mode
	u->readonly	= readonly;

	// Add the dirname
	strcpy( u->dirname, dirname );

	// Open the other files
	sprintf( filename, "%s/%s", dirname, URLIDX_FILENAME_SITE_LIST );
	if( u->readonly ) {
		u->site_list = fopen64( filename, "r" );
	} else {
		u->site_list = fopen64( filename, "a+" );
	}

	if( u->site_list == NULL ) {
		perror( "Opening url index, site list" );
		die( "Failed to open url index" );
	}

	sprintf( filename, "%s/%s", dirname, URLIDX_FILENAME_PATH );
	if( u->readonly ) {
		u->path_file = fopen64( filename, "r" );
	} else {
		u->path_file = fopen64( filename, "r+" );
	}
	assert( u->path_file != NULL );

	sprintf( filename, "%s/%s", dirname, URLIDX_FILENAME_PATH_LIST );
	if( u->readonly ) {
		u->path_list = fopen64( filename, "r" );
	} else {
		u->path_list = fopen64( filename, "a+" );
	}
	assert( u->path_list != NULL );

	// Read site names
	URLIDX_SITE_SIZE = URLIDX_EXPECTED_SITENAME_SIZE * CONF_COLLECTION_MAXSITE;

	// Open file
	sprintf( filename, "%s/%s", dirname, URLIDX_FILENAME_SITE );
	FILE *site_file = fopen64( filename, "r" );

	// Get file size
	off64_t file_length;
	fseeko( site_file, 0, SEEK_END );
	file_length = ftello( site_file );
	assert( file_length > 0 );
	fseeko( site_file, 0, SEEK_SET );
	if( file_length >= URLIDX_SITE_SIZE ) {
		cerr << "The URLIDX_SITE_SIZE is too small, increase MAXSITE or EXPECTED_SITENAME_SIZE" << endl;
		exit(1);
	}

	// Allocate memory
	u->site = (char *)malloc( URLIDX_SITE_SIZE );
	assert( u->site != NULL );

	// Read
	count = fread( u->site, file_length, 1, site_file );
	assert( count == 1 );

	// Close
	fclose( site_file );

	// Read SITE hash table
	u->site_hash = (off64_t *)malloc(sizeof(off64_t)*CONF_COLLECTION_MAXSITE);
	assert( u->site_hash != NULL );
	sprintf( filename, "%s/%s", dirname, URLIDX_FILENAME_SITE_HASH );

	// Open
	FILE *file_site_hash = fopen64( filename, "r" );
	assert( file_site_hash != NULL );

	// Read
	siteid_t sites_readed = fread( u->site_hash, sizeof(off64_t), CONF_COLLECTION_MAXSITE, file_site_hash );
	assert( sites_readed == CONF_COLLECTION_MAXSITE );

	// Close
	fclose( file_site_hash );

	// Read PATH hash table
	u->path_hash = (off64_t *)malloc(sizeof(off64_t)*CONF_COLLECTION_MAXDOC);
	assert( u->path_hash != NULL );
	sprintf( filename, "%s/%s", dirname, URLIDX_FILENAME_PATH_HASH );

	// Open
	FILE *file_path_hash = fopen64( filename, "r" );
	assert( file_path_hash != NULL );

	// Read
	docid_t paths_readed = fread( u->path_hash, sizeof(off64_t), CONF_COLLECTION_MAXDOC, file_path_hash );
	assert( paths_readed == CONF_COLLECTION_MAXDOC );

	// Close
	fclose( file_path_hash );

	// Return
	return u;
}

//
// Name: urlidx_close
//
// Description:
//   Closes the url index, saving data to disk
//
// Input:
//   urlidx - the url index structure
//

void urlidx_close( urlidx_t *u ) {
	char filename[MAX_STR_LEN];
	size_t count;

	assert( CONF_OK );

	// Check what i'm going to close
	assert( u != NULL );

	// Close and nullify all the filehandlers
	fclose( u->site_list ); u->site_list = NULL;
	fclose( u->path_list ); u->path_list = NULL;
	fclose( u->path_file ); u->path_file = NULL;

	// If readonly mode, bail out, do not write anything
	if( u->readonly ) {
		free(u);
		return;
	}

	// Save  the site names 
	sprintf( filename, "%s/%s", u->dirname, URLIDX_FILENAME_SITE );

	// Open
	FILE *site_file = fopen64( filename, "w" );
	assert( site_file != NULL );

	// Write
	count = fwrite( u->site, u->site_next_char, 1, site_file );
	assert( count == 1);

	// Close
	fclose( site_file );
	free( u->site );
	u->site = NULL;

	// Save the SITE hash tables
	sprintf( filename, "%s/%s", u->dirname, URLIDX_FILENAME_SITE_HASH );

	// Open
	FILE *file_site_hash = fopen64( filename, "w" );
	assert( file_site_hash != NULL );

	// Write
	siteid_t sites_writed = fwrite( u->site_hash, sizeof(off64_t), CONF_COLLECTION_MAXSITE, file_site_hash );
	assert( sites_writed == CONF_COLLECTION_MAXSITE );

	// Close
	fclose( file_site_hash );
	free( u->site_hash );
	u->site_hash = NULL;

	// Save the PATH hash tables
	sprintf( filename, "%s/%s", u->dirname, URLIDX_FILENAME_PATH_HASH );

	// Open
	FILE *file_path_hash = fopen64( filename, "w" );
	assert( file_path_hash != NULL );

	// Write
	docid_t paths_writed = fwrite( u->path_hash, sizeof(off64_t), CONF_COLLECTION_MAXDOC, file_path_hash );
	assert( paths_writed == CONF_COLLECTION_MAXDOC );

	// Close
	fclose( file_path_hash );
	free( u->path_hash );
	u->path_hash = NULL;

	// Write the whole structure to disk
	sprintf( filename, "%s/%s", u->dirname, URLIDX_FILENAME_ALL );

	// Open
	FILE *file_all = fopen64( filename, "w" );
	assert( file_all != NULL );

	// Write
	count = fwrite( u, sizeof(urlidx_t), 1, file_all );
	assert( count == 1 );

	// Close
	fclose( file_all );

	// Nullify to avoid re-closing
	free(u);
}

//
// Name: urlidx_remove
//
// Description:
//   Deletes all files for an urlidx
//
// Input:
//   dirname - the directory to clean
//

void urlidx_remove( const char *dirname ) {
	queue<string> files;

	// Push files
	files.push( URLIDX_FILENAME_SITE_LIST );
	files.push( URLIDX_FILENAME_PATH_LIST );
	files.push( URLIDX_FILENAME_SITE );
	files.push( URLIDX_FILENAME_PATH );
	files.push( URLIDX_FILENAME_SITE_HASH );
	files.push( URLIDX_FILENAME_PATH_HASH );
	files.push( URLIDX_FILENAME_ALL );

	// Delete
	while( ! files.empty() ) {

		// Create filename
		char filename[MAX_STR_LEN];
		sprintf( filename, "%s/%s", dirname, files.front().c_str() );

		// Delete file
		int rc = unlink( filename );
		if( rc != 0 && errno != ENOENT ) {
			perror( files.front().c_str() );
		}

		// Remove file from queue
		files.pop();

	}
}

//
// Name: urlidx_dump_status
//
// Description:
//   Shows the status of the url index
//
// Input:
//   urlidx - the url index structure
//

void urlidx_dump_status( urlidx_t *u ) {
	cerr << "Status dump:" << endl;
	cerr << "- dirname            " << u->dirname << endl;
	cerr << "- site_count             " << u->site_count << endl;
	cerr << "- path_count             " << u->path_count << endl;
	cerr << "- Disk used by sitenames " << u->site_next_char << endl;
	cerr << "- Disk used by paths     " << u->path_next_char << endl;
}

//
// Name: urlidx_check_site
//
// Description:
//   Check if a sitename exists
//
// Input:
//   urlidx - the url index structure
//   site - the site name to check
//   bucket - the bucket in which this was found
//
// Return
//   a siteid if resolved
//   0 if not found
//

siteid_t urlidx_check_site( urlidx_t *u, const char *site, siteid_t *bucket ) {
	assert( u != NULL );
	assert( site != NULL );
	assert( strlen( site ) > 0 );


	// First attempt to find bucket
	(*bucket) = urlidx_hashing_site( site );

	// Linear probing
	off64_t position;
	while( (position = (u->site_hash)[(*bucket)]) != 0 ) {
		
		// See if it's the same
		if( !strcmp( (u->site) + position, site ) ) {

			// If it's, get the siteid
			char *p = (u->site) + position;
			
			// Go to the end of the string
			while( *p ) { p++; };	

			// Move one step forward
			p++;

			// Return the siteid
			return *((siteid_t *)p);
		}
		(*bucket) = ((*bucket) + 1) % CONF_COLLECTION_MAXSITE;
	}

	return (siteid_t)0;
}

//
// Name: urlidx_resolve_site
//
// Description:
//   Verify a sitename and add if necessary
//
// Input:
//   urlidx - the url index structure
//   site - the site name to check
//
// Output:
//   siteid - siteid of the existent/created register, if NULL, don't create
//   if not found.
//
// Return:
//   URLIDX_EXISTENT - the site existed
//   URLIDX_CREATED_SITE - the site was added
//   URLIDX_NOT_FOUND - the site is not known and was not created
//

urlidx_status_t urlidx_resolve_site( urlidx_t *u, const char *site, siteid_t *siteid ) {
	assert( u != NULL );
	assert( site != NULL );
	assert( strlen(site) > 0 );

	siteid_t bucket;
	siteid_t siteid_check;

	// Check if the site exists
	siteid_check	= urlidx_check_site( u, site, &(bucket) );

	if( siteid_check > 0 ) {
		if( siteid != NULL ) {
			*siteid	= siteid_check;
		}
		return URLIDX_EXISTENT;
	}

	if( siteid == NULL ) {
		return URLIDX_NOT_FOUND;
	}

	// Check for www.X.com if X.com was given,
	if( site[0] != 'w' ) {
		siteid_t	bucket_new;
		char site_new[MAX_STR_LEN];
		strcpy( site_new, "www." );
		strcat( site_new, site );
		assert( strlen(site_new) < MAX_STR_LEN );

		siteid_check = urlidx_check_site( u, site_new, &(bucket_new) );
		if( siteid_check > 0 ) {
			*siteid	= siteid_check;
			return URLIDX_EXISTENT;
		}

	// Check for X.com if www.X.com was given
	} else if( strlen(site) > 4 && !strncmp( site, "www.", 4 ) ) {

		siteid_t	bucket_new;
		siteid_check = urlidx_check_site( u, (site + 4), &(bucket_new) );
		if( siteid_check > 0 ) {
			*siteid	= siteid_check;
			return URLIDX_EXISTENT;
		}
	}

	// If I was not given a valid siteid space to return
	// a siteid, then the user don't want me to create
	// a new siteid slot for this site, so I just
	// return not found.
	if( siteid == NULL ) {
		return URLIDX_NOT_FOUND;
	}

	// Get the next siteid
	*siteid = ++(u->site_count);

	if( u->site_count > CONF_COLLECTION_MAXSITE ) {
		die( "Maximum number of sites exceeded, increase in configuration file"  );
	}

	// Put in bucket
	(u->site_hash)[bucket] = (u->site_next_char);

	// Save in list of sites
	fwrite( &(u->site_next_char), sizeof(off64_t), 1, u->site_list );

	// Store string
	memcpy( u->site + u->site_next_char, site, strlen(site) + 1 );

	// Store siteid
	memcpy( u->site + u->site_next_char + strlen(site) + 1, siteid, sizeof(siteid_t) );

	// Move char pointer
	u->site_next_char = u->site_next_char + strlen(site) + 1 + sizeof(siteid_t);
	if( u->site_next_char > URLIDX_SITE_SIZE ) {
		cerr << "Site names memory area full!" << endl;
		cerr << "Increase CONF_COLLECTION_MAXSITE or EXPECTED_SITENAME_SIZE" << endl;
		return(URLIDX_ERROR);
	}

	// Return
	return URLIDX_CREATED_SITE;
}

// 
// Name: urlidx_resolve_path
//
// Description:
//   Check if path is saved for a siteid, and store it if not
//
// Input:
//   urlidx - the url index structure
//   siteid - the siteid in which this path was found
//   path - the path; it MUST NOT begin with a '/'
//
// Output:
//   docid - the corresponding document id
//
//
// Return:
//   URLIDX_EXISTENT - the path existed in that site
//   URLIDX_CREATED_PATH - the path was added
//

urlidx_status_t  urlidx_resolve_path( urlidx_t *u, siteid_t siteid, const char *inpath, docid_t *docid ) {
	// Check the path
	assert( inpath[0] != '/' );
	assert( !strchr(inpath,'\r') );
	assert( !strchr(inpath,'\n') );

	// On disk, the path will be a bit larger
	char path[URLIDX_PATH_LEN];

	// Copy inpath to path. Append the siteid at the start.

	int  pathlen = sizeof(siteid_t) + strlen(inpath);
	memcpy( path, &siteid, sizeof(siteid) );
	memcpy( path + sizeof(siteid_t), inpath, strlen(inpath) + 1 );

	// Try to find it in the hash table
	docid_t bucket = urlidx_hashing_path( path, pathlen );

	// Linear probing
	while(1) {
		off64_t position;
		char found[URLIDX_PATH_LEN];
	   
		// Read the position of this path in disk
		position = u->path_hash[bucket];
		if( position == 0 ) {
			break;
		}

		// Read from disk (dunno where the end is, so read a chunk)
		fseeko( u->path_file, position, SEEK_SET );
		fread( found, URLIDX_PATH_LEN, 1, u->path_file );

		// Compare the extracted with the disk
		if( !memcmp( found, path, pathlen ) ) {

			// If it's, save the docid
			*docid = *((docid_t *)(found + pathlen + 1));

			// Return
			return URLIDX_EXISTENT;
		}
		bucket = (bucket + 1) % CONF_COLLECTION_MAXDOC;
	}

	// It's a new element, insert

	// Get docid. Note that this structure has its own
	// count of object, and this could be inconsistent with
	// the metaidx if something nasty happens with it.

	// The idea with the metaidx is that whenever it's asked
	// to store a docid, if this docid is larger than its current
	// number of documents, it increases its number of documents
	*docid = ++(u->path_count);

	// Check
	if( u->path_count >= CONF_COLLECTION_MAXDOC ) {
		die( "Maximum number of docs exceeded, increase in configuration file"  );
	}

	// Put in bucket
	(u->path_hash)[bucket] = (u->path_next_char);

	// Save in list of docs
	fwrite( &(u->path_next_char), sizeof(off64_t), 1, u->path_list );

	// Store string
	fseeko( u->path_file, u->path_next_char, SEEK_SET );
	fwrite( path, pathlen + 1, 1, u->path_file );

	// Store docid
	fwrite( docid, sizeof(docid_t), 1, u->path_file );

	// Move char pointer
	u->path_next_char = u->path_next_char + pathlen + 1 + sizeof(docid_t);

	// Return
	return URLIDX_CREATED_PATH;
}

//
// Name: urlidx_resolve_url
//
// Description:
//   Gets the docid for a "sitename/path" string
//
// Input:
//   urlidx - url index structure
//   url - string, in the form "sitename/path"
//
// Output:
//   siteid - siteid for sitename
//   docid - docid for path
//
// Return:
//   URLIDX_CREATED_SITE - created site and path entry
//   URLIDX_CREATED_PATH - site existed, created path
//   URLIDX_OK - everything is ok
//

urlidx_status_t urlidx_resolve_url( urlidx_t *u, char *url, siteid_t *siteid, docid_t *docid ) {
	char site[MAX_STR_LEN];
	char path[MAX_STR_LEN];

	// Copy the site name
	int i = 0;
	for( i=0; url[i] != '/' && url[i] != '\0'; i++ ) {
		site[i] = url[i];
	}
	site[i] = '\0';

	// Check if no '/'
	assert( url[i] != '\0' );

	// Skip the '/'
	i++;

	// Copy the path
	int j = 0;
	for( ; url[i] != '\0'; i++ ) {
		path[j++] = url[i];	
	}
	path[j] = '\0';

	// Resolve
	if( urlidx_resolve_site(u, site, siteid ) == URLIDX_CREATED_SITE ) {
		urlidx_resolve_path( u, *siteid, path, docid );
		return URLIDX_CREATED_SITE;
	} else {
		return urlidx_resolve_path( u, *siteid, path, docid );
	}
}

/* 
	urlidx_site_by_siteid
	Get the name of a site based on its id
	TODO: This function must fail in an error condition.
*/

void urlidx_site_by_siteid( urlidx_t *u, siteid_t siteid, char *sitename ) {

	// Seek
	fseeko( u->site_list, (siteid - 1) * sizeof(off64_t), SEEK_SET );

	// Read offset
	off64_t offset;
	fread( &offset, sizeof(off64_t), 1, u->site_list );

	// Copy
	assert( strlen( (u->site) + offset ) < MAX_STR_LEN );
	strcpy( sitename, (u->site) + offset );

	// Put at end
	fseeko( u->site_list, 0, SEEK_END );
}

/*
	urlidx_url_by_docid( u, docid, url )
	Returns the url of a docid in "url" in the form "site/path".
	This is important! because we don't need to locate the
    siteid in the metaidx to retrieve it. the siteid is stored
    here also.
*/

void urlidx_url_by_docid( urlidx_t *u, docid_t docid, char *url ) {

	// Seek
	fseeko( u->path_list, (docid - 1) * sizeof(off64_t), SEEK_SET );

	// Read offset
	off64_t offset;
	fread( &offset, sizeof(off64_t), 1, u->path_list );

	// Put at end
	fseeko( u->path_list, 0, SEEK_END );

	// Read the path
	char path[MAX_STR_LEN + 1 + sizeof(docid_t)];
	fseeko( u->path_file, offset, SEEK_SET );
	fread( path, MAX_STR_LEN + 1 + sizeof(docid_t), 1, u->path_file );

	// Get the siteid and skip it
	siteid_t siteid;
	siteid = *((siteid_t *)path);

	// Get the Site Name
	urlidx_site_by_siteid( u, siteid, url );

	// Append a slash
	strcat( url, "/" );

	// Append the path
	strcat( url, (path + sizeof(siteid_t)) );
	
}

//
// Name: urlidx_siteid_by_docid
//
// Description:
//   Returns the stored siteid for a document
//
// Input:
//   urlidx - the structure
//   docid - the document id
//
// Returns:
//   the siteid of the docid
//

siteid_t urlidx_siteid_by_docid( urlidx_t *u, docid_t docid ) {
	assert( u != NULL );

	// Seek
	fseeko( u->path_list, (docid - 1) * sizeof(off64_t), SEEK_SET );

	// Read offset
	off64_t offset;
	fread( &offset, sizeof(off64_t), 1, u->path_list );

	// Put at end
	fseeko( u->path_list, 0, SEEK_END );

	// Read the path
	char path[MAX_STR_LEN + 1 + sizeof(docid_t)];
	fseeko( u->path_file, offset, SEEK_SET );
	fread( path, MAX_STR_LEN + 1 + sizeof(docid_t), 1, u->path_file );

	// Get the siteid
	siteid_t siteid;
	siteid = *((siteid_t *)path);

	return siteid;

}

//
// urlidx_path_by_docid
// Returns the path of a URL
//

void urlidx_path_by_docid( urlidx_t *u, docid_t docid, char *path ) {
	// Seek
	fseeko( u->path_list, (docid - 1) * sizeof(off64_t), SEEK_SET );

	// Read offset
	off64_t offset;
	fread( &offset, sizeof(off64_t), 1, u->path_list );

	// Read the path
	char buf[MAX_STR_LEN + 1 + sizeof(docid_t)];
	fseeko( u->path_file, offset, SEEK_SET );
	fread( buf, MAX_STR_LEN + 1 + sizeof(docid_t), 1, u->path_file );

	// Put at end
	fseeko( u->path_list, 0, SEEK_END );

	// Return
	assert( strlen((buf + sizeof(siteid_t))) < MAX_STR_LEN );
	strcpy( path, (buf + sizeof(siteid_t)));
}

//
// Name: urlidx_is_homepage
//
// Description:
//   Used to quickly check if a url points to a home page
//
// Return:
//   1 if path = ''
//   0 if not

bool urlidx_is_homepage( urlidx_t *u, docid_t docid ) {

	// Seek
	fseeko( u->path_list, (docid - 1) * sizeof(off64_t), SEEK_SET );

	// Read offset
	off64_t offset_start;
	off64_t offset_end;
	fread( &offset_start, sizeof(off64_t), 1, u->path_list );

	// Read next offset
	size_t rc = fread( &offset_end, sizeof(off64_t), 1, u->path_list );
	if( rc != 1 ) {
		// This is the last path in the file
		// probably it's not a homepage (checking it will mean
		// reading the file, and we don't want to).
		return false;
	}

	// Check if it's too small, just '/'
	if( (offset_end - offset_start) == ( (sizeof( siteid_t ) * 2) + 1) ) {
		// It contains just the siteid, then
		// this is a home page
		return true;
	} else {
		// This is not a homepage
		return false;
	}
}
// Hashing Functions

siteid_t urlidx_hashing_site( const char *text ) {
	siteid_t val;
	for( val=0; *text; text++ ) {
		val = 131 * val + *text;
	}
	return( val % CONF_COLLECTION_MAXSITE );
}

docid_t urlidx_hashing_path( const char *text, int size ) {
	docid_t val;
	int count = 0;
	for( val=0; count++ <= size; text++ ) {
		val = 131 * val + *text;
	}
	return( val % CONF_COLLECTION_MAXDOC );
}

//
// Name: urlidx_canonicalize_path
//
// Description:
//   Put a path in canonical form; eliminate all '/./'
//   and go up in the path if a '/../' is found.
//   Result doesn't have leading '/', BUT represents an absolute path
//
// Input:
//   source - Original path
//
// Output:
//   dest - Result path
//

void urlidx_canonicalize_path( char *source, char *dest) {
	// Iterate for each component of the path
	char component[MAX_STR_LEN];
	uint slash_count = 0;
	uint slash_position[MAX_STR_LEN];
	uint outpos 	= 0;
	uint comppos 	= 0;
	uint inpos		= 0;

	// Check special case
	if( source == NULL ) {
		dest = NULL;
		return;
	}

	// Copy source

	// Test memory
	for( uint i = 0; i<MAX_STR_LEN; i++ ) {
		dest[i] = '\0';
		component[i] = '\0';
		slash_position[i] = 0;
	}


	for( inpos = 0; inpos <= strlen(source); inpos++ ) {
		if( source[inpos] == '/' || source[inpos] == '\0' ) {

			// Close the component
			component[comppos] = '\0';

			// Check for '..'
			if( component[0] == '.' && component[1] == '.'
			 && component[2] == '\0' ) {
				if( slash_count == 0 ) {
					// Ignore, this path begins with '..'
				} else {
					outpos = slash_position[--slash_count];
				}
				comppos = 0;

			// Check for '.'
			} else if( component[0] == '.' && component[1] == '\0' ) {

				// Ignore
				comppos = 0;

			// It's something else, copy
			} else {

				slash_position[slash_count++] = outpos;

				if( outpos != 0 ) {
					dest[outpos++] = '/';
				}

				for( uint i = 0; i < strlen(component); i++ ) {
					if( outpos < MAX_STR_LEN-1 ) {
						dest[outpos++] = component[i];
					}
				}

				comppos = 0;

			}

		} else {
			if( comppos < MAX_STR_LEN-1 ) {
				component[comppos++] = source[inpos];
			}
		}
	}

	// Write the final '\0'
	dest[outpos] = '\0';

	// Return
	return;
}


//
// Name: urlidx_is_dynamic
//
// Description:
//   Checks if a url is dynamic; checks for cgi parameter separator ('?')
//   Then for extensions, then for cgi- (cgi-bin, cgi-local).
//
// Input:
//   extensions_dynamic - hash table with the known dynamic extensions
//   url - the url
// 
// Return:
//   true iff the url is looks like a dynamic one
//

bool urlidx_is_dynamic( perfhash_t *extensions_dynamic, char *url ) {
	assert( extensions_dynamic != NULL );
	assert( url != NULL );

	// Check for '?' in the url
	if( strchr( url, '?' ) != NULL ) {
		return true;
	}

	// Check for ';' in the url
	if( strchr( url, ';' ) != NULL ) {
		return true;
	}

	// Check for known dynamic extensions
	char lowercase_extension[MAX_STR_LEN];
	urlidx_get_lowercase_extension( url, lowercase_extension );

	if( strlen( lowercase_extension ) > 0 ) {
		// Search
		if( perfhash_check( extensions_dynamic, lowercase_extension) ) {
			return true;
		}
	}

	// Heuristic: check for known directories

	if(    strstr( url, "cgi-bin" ) != NULL
		|| strstr( url, "cgi-local" ) != NULL ){

		return true;
	}

	// It looks like a static url

	return false;
}

//
// Name: urlidx_parse_complete_url
//
// Description:
//   Splits an url in its parts.
//
// Input:
//   original_url - The original url to be converted
//
// Output:
//   dest_protocol - The parsed protocol
//   dest_sitename - The parsed site name
//   dest_path     - The parsed path
//
// Return:
//   true - Everything is ok
//   false - original_url was malformed
//

bool urlidx_parse_complete_url( char *url, char *protocol, char *sitename, char *path ) {
	int src_pos = 0;
	int dest_pos = 0;

	// Check if url is empty
	if( url[0] == '\0' ) {
		return false;
	}

	// Copy protocol
	while( url[src_pos] != ':' && url[src_pos] != '\0' ) {
		protocol[dest_pos++] = url[src_pos++];
	}

	// End protocol string
	protocol[dest_pos] = '\0';

	// Check if there is something more besides the protocol
	if( url[src_pos] == '\0' ) {
		return false;
	}

	// Move two chars right, checking that they are slashes
	src_pos++;
	if( url[src_pos] != '/' || url[src_pos] == '\0' ) {
		return false;
	}
	src_pos++;
	if( url[src_pos] != '/' || url[src_pos] == '\0' ) {
		return false;
	}

	// Copy the sitename
	dest_pos = 0;
	src_pos++;
	while( url[src_pos] != '/' && url[src_pos] != '\0' ) {
		sitename[dest_pos++] = url[src_pos++];
	}
	
	// End sitename string
	sitename[dest_pos] = '\0';

	// Check if the path is empty

	if( url[src_pos] == '\0' ) {
		path[0] = '\0';
		return true;
	}

	// Move past the slash
	src_pos++;

	if( url[src_pos] == '/' ) {
		// Path begins with double slash, malformed
		return false;
	}

	// Copy the path
	dest_pos = 0;
	while( url[src_pos] != '\0' ) {
		path[dest_pos++] = url[src_pos++];
	}
	path[dest_pos] = '\0';

	return true;
}

//
// Name: urlidx_relative_path_to_absolute
//
// Description:
//   Converts a path to absolute
//
// Input:
//   src_path - source path
//   url - destination of the link
//
// Output:
//   path - the resulting path
//

void urlidx_relative_path_to_absolute( char *src_path, char *url, char *path ) {
	char complete_path[MAX_STR_LEN];
	uint outpos = 0;

	// If given an absolute path, return immediatly
	if( url[0] == '/' ) {
		assert( strlen(url) < MAX_STR_LEN );
		strcpy( path, url );
		return;
	}

	// We have to avoid appending double '?'
	bool seen_question_mark = false;

	// Copy the source url, up to the last slash
	int last_slash = 0;
	for(outpos=0; outpos<strlen(src_path); outpos++ ) {
		if( src_path[outpos] == '?' ) {

			// We try to avoid double question marks
			if( seen_question_mark ) {
				break;
			} else {
				seen_question_mark = true;
			}
		} else if( src_path[outpos] == '#' ) {

			// We don't want to copy hash urls
			break;
		} else if( src_path[outpos] == '/' && !seen_question_mark) {
			last_slash = outpos;
		}

		// Copy
		complete_path[outpos] = src_path[outpos];
	}
	outpos = last_slash;
	complete_path[outpos++] = '/';
	seen_question_mark = false;
	// Append the given URL
	uint j=0;
	while(j<strlen(url)) {
		if( url[j] == '?' ) {
			if( seen_question_mark ) {
				break;
			} else {
				seen_question_mark = true;
			}
		}
		if( outpos < MAX_STR_LEN-1 ) {
			complete_path[outpos++] = url[j];
		}
		j++;
	}
	complete_path[outpos] = '\0';

	// Canonicalize
	urlidx_canonicalize_path( complete_path, path );
}

//
// Name: urlidx_remove_variable
//
// Description:
//   Removes a variable from an URL, useful for deleting
//   session-ids in URLs. Variables can be CGI variables,
//   or portions of the URL separated by ';', e.g.:
//    a/b.cgi?c=1&d=2&PHPSESSID=000000  -> a/b.cgi?c=1&d=2&
//    a/b.html;jsessionid=000000        -> a/b.html
//  
//  Input:
//    url - the original url
//    varname - the variable name
//
//  Output:
//    url - the corrected url
//

#define isalnum_or_underscore(x) (isalnum(x)||x=='_')

void urlidx_remove_variable( char *url, const char *varname ) {
	assert( url != NULL );
	assert( varname != NULL );

	int varlen	= strlen( varname );
	assert( varlen > 0 );

	// Easy test
	if( ! strstr(url, varname) ) {
		return;
	}

	// Deleting is very difficult and slow, as a lot
	// of patterns can occur, including the variable
	// at the beginning, end, middle of the CGI portion,
	// or even before, separated by a ';', so we
	// take the easy path.

	// Try to find this pattern in the URL
	char *start_ptr 	= NULL;
	char *end_ptr		= NULL;

	while( (start_ptr = strstr( url, varname ) ) ) {

		// Check if we are at the beginning of the URL, in this
		// case, we can't be matching OK
		end_ptr	= start_ptr + varlen;

		// Move the start a bit to catch the var as a suffix
		while( start_ptr > url && isalnum_or_underscore(*(start_ptr - 1)) ) {
			start_ptr --;
		}

		if( (*end_ptr) == '=' ) {
			end_ptr++;

			// Now we have the character that start the value
			// of this var, we have to search for the end of it.
			while( (*end_ptr) != '\0' && isalnum_or_underscore( (*end_ptr) ) ) {
				end_ptr ++;
			}

			// Check if we are also at the end of the string
			while( (*end_ptr) != '\0' ) {
				(*(start_ptr++)) = (*(end_ptr++));
			}
			(*start_ptr) = '\0';
		}
		// Advance the url
		url ++;
	}

}


//
// Name: urlidx_remove_sessionids_heuristic
//
// Description:
//   Heuristic to detect other common sessionids in the url
//  

void urlidx_remove_sessionids_heuristic( char *path ) {
	assert( path != NULL );

	// Remove s=XXXX..... from .php files (this is an heuristic,
	// because many php files use 's=XXXX....' as a sessionid, so
	// I will be careful and only remove this at the beginning
	if( char *position = strstr( path, "php?s=" ) ) {

		// Check if we have at least 4 hex digit in the variable
		// value
		if( strlen(position) > 12 &&
			isxdigit(position[6]) && isxdigit(position[7]) &&
			isxdigit(position[8]) && isxdigit(position[9]) &&
			isxdigit(position[10]) && isxdigit(position[11]) ) {

			urlidx_remove_variable( path, "s" );

		}
	}

	if( char *position = strstr( path, "sid=" ) ) {

		// Check if we have at least 6 hex digit in the variable
		// value
		if( strlen(position) > 12 &&
			isxdigit(position[4]) && isxdigit(position[5]) &&
			isxdigit(position[6]) && isxdigit(position[7]) &&
			isxdigit(position[8]) && isxdigit(position[9]) &&
			isxdigit(position[10]) && isxdigit(position[11]) ) {

			urlidx_remove_variable( path, "sid" );

		}
	}
}


//
// Name: urlidx_sanitize_url
//
// Description:
//   Performs a series of replacement in the URL
//

void urlidx_sanitize_url( char *url ) {

	// Replace all '&amp;' by '&'
	replace_all( url, "&amp;", "&" );

	// Replace '&&' by '&'
	replace_all( url, "&&", "&" );

	// Fix the url, e.g.: a.html;sessid=x?b=1 -> a.html;?b=1 -> a.html?b=1
	// Replace ';?' by '?'
	replace_all( url, ";?", "?" );

	// Replace '?&' by '?'
	replace_all( url, "?&", "?" );

	// Replace double slash
	replace_all( url, "//", "/" );

	// Fix the url, remove trailing ';', '&' or '?'
	while( strlen(url) > 0 && (url[strlen(url)-1] == ';' || url[strlen(url)-1] == '&' || url[strlen(url)-1] == '?') ) {
		url[strlen(url)-1] = '\0';
	}
}

//
// Name: urlidx_get_lowercase_extension
//
// Description:
//   Gets the lowercased extension of a string, e.g.:
//    example.html?x=1  -> html
//    www.example.CoM   -> com
//
// Input:
//   examined - the string to be examined
//
// Output:
//   extension - the resulting extension, the user must request memory
//               for this
//

void urlidx_get_lowercase_extension( char *examined, char *extension ) {
	// Make sure we will return something in the extension
	extension[0] = '\0';

	// Go to the end of the string
	int inpos	= strlen(examined) - 1;

	// If the string is empty, or ends in a dot
	if( inpos == -1 || examined[inpos] == '.' ) {
		// There is no extension
		return;
	}

	// Go backwards until a dot is found
	inpos--;
	while( inpos > 0 ) {
		if( examined[inpos] == '.' ) {
			// We have located the last dot, move one char
			// to the right
			inpos++;

			// Copy, until non-alphanumeric characters appear or we
			// reach the end of the string
			int	outpos	= 0;
			while( examined[inpos] != '\0'
				&& isalnum_or_underscore(examined[inpos]) 
				&& outpos < MAX_STR_LEN ) {

				extension[outpos++]	= tolower( examined[inpos++] );
			}

			// Put a null at the end
			extension[outpos] = '\0';
			return;
		}
		inpos--;
	}

	// We didn't find a dot
	return;
}
