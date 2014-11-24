#include "storage.h"

//
// Name: storage_prepare_sequential_read
//
// Description:
//   Prepares the storage file for sequential reading
//   THIS WORKS only if there are no deletions
//
// Input:
//   st - the storage object
//

storage_status_t storage_prepare_sequential_read( storage_t *st ) {
	assert( st != NULL );

	// Look for the associated storage record
	if( lseek( st->index_file, (off64_t)(1 * sizeof(storage_record_t)), SEEK_SET) < (off64_t)0 ) {
		perror( "storage_read: couldn't seek to the begining of index" );
		die( "couldn't seek on the index" );
	}

	// Go to the content file address for that document
	if( lseek( st->content_file, (off64_t)0, SEEK_SET ) != (off64_t)0 ) {
		perror( "storage_read: couldn't seek to the begining of content file" );
		die( "seek in content file" );
	}

	return STORAGE_OK;
}
//
// Name: storage_read
// 
// Description:
//   Reads a document from a storage
//
// Input:
//   st - storage object
//   docid - document id to retrieve
//
// Output:
//   buf - the readed object
//   len - the length
//
// Return:
//   status
// 

storage_status_t storage_read( storage_t *st, docid_t docid, char *buf, off64_t *len ) {
	storage_record_t rec;

	assert( st != NULL );
	assert( docid > 0 );

	// Ensure a consistent buffer on output
	buf[0] = '\0';

	// Check if the storage file is large enough
	if( docid >= st->max_docid ) {
		return STORAGE_NOT_FOUND;
	}

	// Look for the associated storage record
	if( lseek( st->index_file, (off64_t)(docid * sizeof(storage_record_t)), SEEK_SET) < (off64_t)0 ) {
		perror( "storage_read: couldn't seek on index" );
		die( "couldn't seek on index" );
	}

	// Read the storage record
	if( read( st->index_file, &(rec), sizeof(storage_record_t)) != sizeof(storage_record_t)) {
		cerr << "Failed when reading docid " << docid << endl;
		perror( "storage_read: couldn't read from index" );
		die( "couldn't read from index" );
	}

	// Check record size
	if( rec.size == -1 ) {
		// Empty record
		return STORAGE_NOT_FOUND;

	};
	assert( rec.size + 1 <= MAX_DOC_LEN );

	// Go to the content file address for that document
	if( lseek( st->content_file, rec.offset, SEEK_SET ) != rec.offset ) {
		perror( "storage_read: couldn't seek in content file" );
		die( "seek in content file" );
	}

	// Read the content into the buffer
	if( read( st->content_file, buf, rec.size ) != rec.size ) {
		perror( "storage_read: failed to read content file" );
		die( "read content file" );
	}

	// Copy the size
	*len = rec.size;

	// Append a '\0' at the end of the readed content
	buf[rec.size] = '\0';
	
	return STORAGE_OK;
}

//
// Name: storage_write
//
// Description:
//   Writes a document to storage
//
// Input:
//   st - the storage object
//   docid - document id to write
//   buf - content of the document
//   size - number of bytes to write
//   hash - the old hash value (if it's null, then we don't care about text dup)
//
// Output:
//   hash - the hash value calculated while writing
//   duplicate_of - if it's a duplicate, return docid of duplicate source
//
// Return:
//   status

storage_status_t storage_write( storage_t *st, docid_t docid, char *buf, off64_t size, doc_hash_t *hash, docid_t *duplicate_of ) {
	assert( st != NULL );
	assert( st->duplicates_file > 0 );
	assert( docid > 0 );
	assert( size > 0 );
	assert( ! st->readonly );
	off64_t rc;

	if( duplicate_of != NULL ) {
		(*duplicate_of) = (docid_t)0;
	}

	// Check if it's necessary to grow the index
	if( docid >= st->max_docid ) {
		_storage_grow( st, docid );
	}

	// If hash is null, then the we don't care about text duplicates
	if( hash != NULL ) {

		// Calculate hash function
		doc_hash_t newhash = _storage_hash( size, buf );

		// Check if the doc was already written
		if( storage_exists(st,docid) == STORAGE_OK ) {

			bool has_changed = true;

			// Check if the doc has changed
			if( newhash == *hash ) {
				
				// Same hash value, check content
				has_changed = false;

				// Load the old document
				char *tmpbuf = (char *)malloc(sizeof(char)*MAX_DOC_LEN);
				off64_t tmplen;
				assert( tmpbuf != NULL );
				storage_status_t strc =
					storage_read( st, docid, tmpbuf, &(tmplen) );
				assert( strc == STORAGE_OK );

				// Compare sizes
				if( tmplen == size ) {

					// Compare content
					for( off64_t i=0; i<tmplen; i++ ) {
						if( tmpbuf[i] != buf[i] ) {
							has_changed = true;
							break;
						}
					}
				} else {
					has_changed = true;
				}

				free( tmpbuf );

				if( ! has_changed ) {

					return STORAGE_UNCHANGED;
				}
			}

			// It has changed
			
			// Delete old record from hash table
			rc = lseek( st->duplicates_file,
					*hash * sizeof(docid_t), SEEK_SET );
			if( rc == (off64_t)-1 ) {
				cerr << "[Error while deleting record from duplicates hash]" << endl;
			}
			docid_t zero_docid = 0;
			rc = write( st->duplicates_file,
					&(zero_docid), sizeof(docid_t) );
			if( rc != sizeof(docid_t) ) {
				cerr << "[Error while writing a 0 in the hash of duplicates]" << endl;
			}

			// Delete content from storage file
			storage_delete( st, docid );

		}

		// It's a new document, or a document that has changed

		// Note: this may be the source for a mirror (ie.: many pages
		// are copies of this one, and this page changed).
		// Hence, // those pages are no longer mirrors of this. 
		// To account for this case, mirrors should be visited,
		// with low priority.

		// Save hash value
		(*hash) = newhash;

		// Check hash table for duplicates
		rc = lseek( st->duplicates_file, newhash*sizeof(docid_t),
				SEEK_SET );
		if( rc < (off64_t)0 ) {
			cerr << "[Couldn't seek to  " << newhash << "]";
			perror( "lseek" );
			die( "lseek" );
		}
		docid_t duplicate_docid = 0;

		rc = read( st->duplicates_file, &(duplicate_docid), sizeof(docid_t) );
		assert( rc == sizeof(docid_t) );

		// Ask for memory to load temporary files and compare
		char *tmpbuf = (char *)malloc(sizeof(char)*MAX_DOC_LEN);
		assert( tmpbuf != NULL );

		while( duplicate_docid != (docid_t)0
				&& duplicate_docid != docid ) {

			// Check if it's duplicate by content
			bool is_duplicate = true;

			// Read potential duplicate
			off64_t tmplen;
			storage_status_t strc =
				storage_read( st, duplicate_docid, tmpbuf, &(tmplen) );

			if( strc != STORAGE_OK ) {
				cerr << "[Failed to read document " << duplicate_docid << " while checking duplicates of " << docid << "]" << endl;
				is_duplicate = false;
				break;

			} else {

				// Compare sizes
				if( tmplen == size ) {

					// Compare content
					for( off64_t i=0; i<tmplen; i++ ) {
						if( tmpbuf[i] != buf[i] ) {
							is_duplicate = false;
							break;
						}
					}
				} else {
					is_duplicate = false;
				}

				if( is_duplicate ) {
					(*duplicate_of) = duplicate_docid;
					free( tmpbuf );
					return STORAGE_DUPLICATE;
				}

				// Linear probe
				newhash = (newhash + 1)
					% (CONF_COLLECTION_MAXDOC * STORAGE_DUPLICATE_HASH_FACTOR );

				rc = lseek( st->duplicates_file, newhash*sizeof(docid_t),
						SEEK_SET );
				assert( rc == (off64_t)(newhash*sizeof(docid_t)) );
				rc = read( st->duplicates_file, &(duplicate_docid), sizeof(docid_t) );
				assert( rc == sizeof(docid_t) );
			}
		}

		free(tmpbuf);

		// It's not a duplicate, store it in the hash table
		rc = lseek( st->duplicates_file, newhash * sizeof(docid_t),
				SEEK_SET );
		assert( rc == (newhash*sizeof(docid_t)) );
		rc = write( st->duplicates_file, &(docid), sizeof(docid_t) );
		assert( rc == sizeof(docid_t) );
	}

	// Look for an free place in the free list
	// First-fit
	storage_free_list_t *current = st->free_list;
	while( current->rec.size < size ) {
		current = current->next;
		if( current == NULL ) {
			cerr << "storage_write: couldn't find a free place in the file" << endl;
			die( "no free places" );
		}

	}

	// Seek in disk that place in the content file
	if( lseek( st->content_file, current->rec.offset, SEEK_SET ) != current->rec.offset ) {
		perror( "storage_write: couldn't seek in content file" );
		die( "seek content file" );
	}

	// Write to disk
	if( write( st->content_file, buf, size ) != size ) {
		if( errno == EFBIG ) {
			die( "File too big! Either the filesystem or the operating system don't support large files: try 1) moving to another filesystem 2) upgrading kernel" );
		} else {
			perror( "storage_write: couldn't write to content file" );
			die( "write content file" );
		}
	}

	// Update the index file

	storage_record_t rec;
	rec.offset = current->rec.offset;
	rec.size   = size;

	_storage_write_index_record( st, docid, &(rec) );

	// Update the free list
	assert( current->rec.size >= size );
	if( current->rec.size == size ) {

		// Delete the free block size

		// Check if start of list
		if( current == st->free_list ) {

			// Delete first element of list
			st->free_list = st->free_list->next;
			free(current);

		} else {

			// Delete another element of list
			storage_free_list_t *ptr = st->free_list;
			while( ptr->next != current ) {
				ptr = ptr->next;
			}
			ptr->next = ptr->next->next;
			free(current);
		}

	} else {

		// Update the free block size
		current->rec.size -= size;
		current->rec.offset += size;
	}
	return STORAGE_OK;
}

//
// Name: storage_create
//
// Description:
//   Creates and return a storage object
//
// Input:
//   basedir - directory
//   check_duplicates - if it's necessary to create a duplicate check file
//
// Return:
//   the created storage object
//

storage_t *storage_create( const char *basedir, bool check_duplicates ) {
	assert( basedir != NULL );

	// Filename
	char filename[MAX_STR_LEN];

	// Create structure
	storage_t *st = (storage_t *)malloc(sizeof(storage_t));
	assert( st != NULL );

	// Set base parameters
	strcpy( st->basedir, basedir );
	st->readonly	= false;
	st->max_docid	= 0;
	st->free_list = NULL;

	// Nullify files
	st->content_file	= 0;
	st->index_file		= 0;
	st->sketch_file         = 0;
	// Create index
	sprintf( filename, "%s/%s", st->basedir, STORAGE_FILENAME_INDEX );
	st->index_file = open( filename, O_RDWR|O_LARGEFILE|O_CREAT|O_TRUNC, S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);

	// Check sizeof(offset type)
	assert( sizeof(off64_t) == 8 );

	if( st->index_file <= 0 ) {
		perror( "storage_create: couldn't create index" );
		die( "create index" );
	}

	// Grow
	_storage_grow( st, (docid_t)1 );

	// Create sketchfile
	sprintf( filename, "%s/%s", st->basedir, STORAGE_FILENAME_SKETCH );
	st->sketch_file = open( filename, O_RDWR|O_LARGEFILE|O_CREAT|O_TRUNC, S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);

	if( st->sketch_file <= 0 ) {
		perror( "storage_create: couldn't create sketchfile" );
		die( "create sketchfile" );
	}

	// Create the content file
	sprintf( filename, "%s/%s", st->basedir, STORAGE_FILENAME_CONTENT );
	st->content_file = open( filename, O_RDWR|O_LARGEFILE|O_CREAT|O_TRUNC, S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);
	assert( st->content_file > 0 );

	// Create the duplicates file (this is slow)
	// The duplicates file is an in-disk hash table, initially
	// set to zero
	sprintf( filename, "%s/%s", st->basedir, STORAGE_FILENAME_DUPLICATES );
	st->duplicates_file = open( filename, O_RDWR|O_LARGEFILE|O_CREAT|O_TRUNC,
	S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);
	if( st->duplicates_file <= 0 ) {
		perror( "storage_create duplicate file" );
		die( "couldn't create duplicates file" );
	}

	// Iterate to fill the hash table with zero
	if( check_duplicates ) {
		docid_t zero_docid = 0;
		for( doc_hash_t i = 0; i <= CONF_COLLECTION_MAXDOC * STORAGE_DUPLICATE_HASH_FACTOR; i++ ) {
			size_t written = 
				write( st->duplicates_file, &(zero_docid), sizeof(docid_t) );
			assert( written == sizeof(docid_t) );
		}
	}

	// Create the free pointers structure
	st->free_list = (storage_free_list_t *)malloc(sizeof(storage_free_list_t));
	assert( st->free_list != NULL );
	st->free_list->rec.offset = 0;
	st->free_list->rec.size   = (off64_t)1 << 62;  // max offset
	st->free_list->next 		= NULL;

	// Return
	return st;
}

//
// Name: storage_open
//
// Description:
//   Opens the storage in the specified directory,
//   creates if necessary
//
// Input:
//   basedir - directory
//
// Return:
//   the opened storage object
//

storage_t *storage_open( const char *basedir, bool readonly ) {
	assert( basedir != NULL );

	char filename[MAX_STR_LEN];

	// Try to open the main file
	sprintf( filename, "%s/%s", basedir, STORAGE_FILENAME_MAIN );
	FILE *file_main = fopen64( filename, "r" );
	if( file_main == NULL ) {

		if( readonly ) {
			die( "Storage does not exist. Can't create file in readonly mode" );
		}

		// Create storage
		cerr << "(storage_create) ";
		return storage_create( basedir, true );
	}

	// Get memory for the storage object
	storage_t *st = (storage_t *)malloc(sizeof(storage_t));
	assert( st != NULL );

	// Read from disk
	if( fread( st, sizeof(storage_t), 1, file_main ) < 1 ) {
		die( "couldn't read main file for storage" );
	}

	// Save basedir for consistency
	strcpy( st->basedir, basedir );

	// Save readonly mode
	st->readonly	= readonly;

	// Open the index
	sprintf( filename, "%s/%s", st->basedir, STORAGE_FILENAME_INDEX );

	if( st->readonly ) {
		st->index_file = open( filename, O_RDONLY|O_LARGEFILE );
	} else {
		st->index_file = open( filename, O_RDWR|O_LARGEFILE|O_CREAT, S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);
	}

	if( st->index_file <= 0 ) {
		perror( "storage_open: couldn't open index" );
		die("storage open");
	}

	off64_t indexsize = lseek( st->index_file, (off64_t)0, SEEK_END );
	assert( indexsize > 0 );

	// Open the sketchfie
	sprintf( filename, "%s/%s", st->basedir, STORAGE_FILENAME_SKETCH );

	if( st->readonly ) {
		st->sketch_file = open( filename, O_RDONLY|O_LARGEFILE );
	} else {
		st->sketch_file = open( filename, O_RDWR|O_LARGEFILE|O_CREAT, S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);
	}

	if( st->sketch_file <= 0 ) {
		perror( "storage_open: couldn't open sketchfile" );
		die("storage open");
	}

	// Open the free pointers file
	sprintf( filename, "%s/%s", st->basedir, STORAGE_FILENAME_FREE );
	int free_file;
	
	if( st->readonly ) {
		free_file = open( filename, O_RDONLY|O_LARGEFILE );
	} else {
		free_file = open( filename, O_RDWR|O_LARGEFILE|O_CREAT, S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);
	}

	if( free_file <= 0 ) {
		perror( "storage_open: couldn't open free file" );
		die("open free file");
	}

	// Check the free pointers file size
	off64_t rc = lseek(free_file, (off64_t)0, SEEK_SET );
	assert( rc == 0 );

	// Read the free pointers file
	storage_record_t freerec;
	st->free_list = NULL;
	storage_free_list_t *current = NULL;
	while( read( free_file, &(freerec), sizeof(freerec)) == sizeof(freerec) ) {
		storage_free_list_t *item = (storage_free_list_t *)malloc(sizeof(storage_free_list_t));
		assert( item != NULL );
		item->rec.offset = freerec.offset;
		item->rec.size	= freerec.size;
		item->next	= NULL;
		if( current == NULL ) {
			st->free_list = item;
		} else {
			current->next = item;
		}
		current = item;
	}
	close( free_file );


	// Open the content file
	sprintf( filename, "%s/%s", st->basedir, STORAGE_FILENAME_CONTENT );
	if( st->readonly ) { 
		st->content_file = open( filename, O_RDONLY|O_LARGEFILE );
	} else {
		st->content_file = open( filename, O_RDWR|O_LARGEFILE, S_IREAD|S_IWRITE|S_IRGRP|S_IROTH );
	}
	if( st->content_file <= 0 ) {
		perror( "storage_open: couldn't open content file" );
		die("open content file");
	}

	// Try to re-create free pointers file if it was lost
	if( st->free_list == NULL ) {
		struct stat64 buf;
		fstat64( st->content_file, &(buf) );

		cerr << "***********************************************" << endl;
		cerr << "WARNING! Had to create again free pointers file" << endl;
		cerr << "         directory " << st->basedir << endl;
		cerr << "**********************************************" << endl;
		storage_free_list_t *item = (storage_free_list_t *)malloc(sizeof(storage_free_list_t));
		assert( item != NULL );

		// Create a blank record
		item->rec.offset	= 0;
		item->rec.size   	= (off64_t)1 << 62;  // max offset

		// Modify record to mark all the data entered
		// as not-free
		item->rec.offset 	+= buf.st_size;
		item->rec.size		-= buf.st_size;

		// Store the item
		item->next	= NULL;
		st->free_list	= item;
	}

	// Open the duplicates file
	sprintf( filename, "%s/%s", st->basedir, STORAGE_FILENAME_DUPLICATES );

	if( st->readonly ) {
		st->duplicates_file = open( filename, O_RDONLY|O_LARGEFILE);
	} else {
		st->duplicates_file = open( filename, O_RDWR|O_LARGEFILE);
	}

	if( st->duplicates_file <= 0 ) {
		perror( "storage_open: couldn't open dup detect file" );
		die( "open dup detect file" );
	}

	// Returns the readed object
	return st;
}

//
// Name: storage_close
//
// Description:
//   Closes the storage in basedir, frees memory
//
// Input:
//   st - the storage object
// 

void storage_close( storage_t *st ) {
	assert( st != NULL );

	int rc;
	char filename[MAX_STR_LEN];

	// Close index st
	rc = close( st->index_file );
	assert( rc == 0 );

	// Close sketch st
	rc = close( st->sketch_file );
	assert( rc == 0 );

	// Close content st
	rc = close( st->content_file );
	assert( rc == 0 );

	// Close duplicates st
	rc = close( st->duplicates_file );
	assert( rc == 0 );

	// If readonly mode, bail out
	if( st->readonly ) {
		free( st );
		return;
	}

	// Open free_file
	int free_file;
	sprintf( filename, "%s/%s", st->basedir, STORAGE_FILENAME_FREE );
	free_file = open( filename, O_RDWR|O_LARGEFILE|O_TRUNC|O_CREAT, S_IREAD|S_IWRITE|S_IRGRP|S_IROTH );
	assert( free_file > 0 );

	// Write free_file
	storage_free_list_t *current = st->free_list;
	storage_free_list_t *last;
	
	while( current != NULL ) {
		if( write( free_file, &(current->rec), sizeof(storage_record_t)) != sizeof(storage_record_t) ) {
			perror( "storage_close: couldn't write to free st" );
			die("storage_close: write free file");
		}
		last = current;
		current = current->next;
		free(last);
	}
	// Close free and duplicates file
	rc = close( free_file );
	assert( rc == 0 );

	// Write main file to disk

	// Nullify all pointers
	st->index_file		= 0;
	st->content_file	= 0;
	st->sketch_file         = 0;
	st->free_list		= NULL;

	// Open main file
	sprintf( filename, "%s/%s", st->basedir, STORAGE_FILENAME_MAIN ); 
	FILE *file_main = fopen64( filename, "w" );
	assert( file_main != NULL );

	if( fwrite( st, sizeof(storage_t), 1, file_main ) < 1 ) {
		die( "Couldn't write file_main" );
	}

	fclose( file_main );

	// Free
	free(st);

	// Nullify
	st = NULL;
}

//
// Name: storage_remove
//
// Description:
//   Removes files in a storage
//
// Input:
//   dirname - the directory
//   

void storage_remove( const char *dirname ) {
	queue<string> files;

	// Push files
	files.push( STORAGE_FILENAME_MAIN );
	files.push( STORAGE_FILENAME_INDEX );
	files.push( STORAGE_FILENAME_CONTENT );
	files.push( STORAGE_FILENAME_SKETCH );
	files.push( STORAGE_FILENAME_FREE );
	files.push( STORAGE_FILENAME_DUPLICATES );

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
// Name: storage_dump_status
//
// Description:
//   Dumps the structure status to stderr
//
// Input:
//   st - the storage object
//

void storage_dump_status( storage_t *st ) {
	cerr << "* Status report of storage in " << st->basedir << endl;
	off64_t total = 0;
	int nfree = 0;
	off64_t maxsize = 0;
	off64_t fragmented = 0;
	storage_free_list_t *current = st->free_list;
	off64_t lastoffset = 0;
	while( current != NULL ) {
		if( current->rec.size > maxsize ) {
			fragmented += maxsize;
			maxsize = current->rec.size;
		} else {
			fragmented += current->rec.size;
		}
		total += current->rec.size;
		nfree++;
		lastoffset = current->rec.offset;
		current = current->next;
	}
	cerr << "Disk space used  : " << lastoffset << endl;
	if( lastoffset > 0 ) {
		cerr << "Fragmented       : " << fragmented << " (" << (int)((fragmented*100/lastoffset)) << "%) in " << nfree << " blocks" << endl;
	}
	cerr << "Available size   : " << total << endl;
	cerr << "max_docid        : " << st->max_docid << endl;
}

//
// Name: storage_exists
//
// Description:
//   Returns the status of existente of an object
//
// Input:
//   st - the storage object
//   docid - the document id requested
//
// Returns:
//   status
//

storage_status_t storage_exists( storage_t *st, docid_t docid ) {
	storage_record_t rec;

	// Verify that the docid is covered
	assert( st != NULL );
	if( docid > st->max_docid ) {

		// Grow
		_storage_grow( st, docid );
	}

	// Load the index record
	_storage_read_index_record( st, docid, &(rec));

	// See if the record contains any information
	if( rec.size == -1 ) {
		return STORAGE_NOT_FOUND;
	} else {
		return STORAGE_OK;
	}
}

//
// Name: storage_delete
//
// Description:
//   Deletes an object from the storage
//
// Input:
//   st - the storage object
//   docid - the document id to delete
//
// Return:
//   status
//

storage_status_t storage_delete( storage_t *st, docid_t docid ) {
	if( storage_exists(st, docid) != STORAGE_OK ) {
		return STORAGE_NOT_FOUND;
	}
	
	// Load the current index record
	storage_record_t rec;
	_storage_read_index_record( st, docid, &(rec));
	assert( rec.size != -1 );

	// Create a new free record for that file
	storage_free_list_t *newfree =  (storage_free_list_t *)malloc(sizeof(storage_free_list_t));
	assert( newfree != NULL );
	newfree->rec.offset 	= rec.offset;
	newfree->rec.size	= rec.size;

	// Search for the place for this new free record

	storage_free_list_t *current	= st->free_list;
	assert( current != NULL );

	if( current->rec.offset >= newfree->rec.offset ) {
		// Must insert in the beginning
		st->free_list	 = newfree;
		newfree->next	 = current;

		// Merge if necessary (new + next)
		if( (newfree->rec.offset + newfree->rec.size) == current->rec.offset ) {
			newfree->rec.size += current->rec.size;
			newfree->next	= current->next;
			free(current);
		}
	} else {

		// Must insert somewhere else
		while( current->next != NULL ) {
			if( current->next->rec.offset >= newfree->rec.offset ) {
				break;
			} else {
				current = current->next;
			}
		}
		if( current->next == NULL ) {
			cerr << "Inserting at the end of the free list ? Free list is corrupted." << endl;
			die( "Free list corrupted" );
		}
		newfree->next	= current->next;
		current->next	= newfree;

		// Merge if necessary (new + previous)
		if( (current->rec.offset + current->rec.size) == newfree->rec.offset ) {
			current->rec.size += newfree->rec.size;
			current->next	= newfree->next;
			free(newfree);
			newfree = current; // so you can test and merge forward
		}

		// Merge if necessary (new + next)
		if( newfree->next != NULL ) {
			storage_free_list_t *next = newfree->next;

			if( (newfree->rec.offset + newfree->rec.size) == next->rec.offset ) {
				newfree->rec.size += next->rec.size;
				newfree->next	   = next->next;
				free(next);
			}

		}

	}

	// Mark in the index as deleted.
	rec.size = -1;
	_storage_write_index_record( st, docid, &(rec));
	
	return STORAGE_OK;
}

//
// Name: _storage_read_index_record
//
// Description:
//   Reads an index record from index_file
//
// Input:
//   st - the storage object
//   docid - the record number
//
// Output:
//   rec - the readed record
//

void _storage_read_index_record( storage_t *st, docid_t docid, storage_record_t *rec ) {
	_storage_seek_index_record( st, docid );
	assert( st->index_file > 0 );

	// Read the storage record
	if( read( st->index_file, rec, sizeof(storage_record_t)) != sizeof(storage_record_t)) {
		perror( "_storage_read_index_record: couldn't read from index" );
		die("read from index");
	}
}

//
// Name: _storage_write_index_record
//
// Description:
//   Writes an index record to index_file
//
// Input:
//   st - the storage object
//   docid - the record number
//   rec - the record to be written
//

void _storage_write_index_record( storage_t *st, docid_t docid, storage_record_t *rec ) {
	_storage_seek_index_record( st, docid );

	if( write( st->index_file, rec, sizeof(storage_record_t)) != sizeof(storage_record_t)) {
		perror( "_storage_write_index_record: couldn't write to index" );
		die("write to index");
	}
}

//
// Name: _storage_seek_index_record
//
// Description:
//   Seeks the position of an index record in index_file
//
// Input:
//   st - the storage object
//   docid - the record number
//

void _storage_seek_index_record( storage_t *st, docid_t docid ) {
	if( lseek( st->index_file, (off64_t)(docid * sizeof(storage_record_t)), SEEK_SET) < (off64_t)0 ) {
		perror( "_storage_seek_index_record: couldn't seek on index" );
		die("seek on index");
	}
}

//
// Name: _storage_grow
//
// Description:
//   Grows the index to allow at least docid
//
// Input:
//   st - the storage object
//   docid - the docid to allocate

void _storage_grow( storage_t *st, docid_t docid ) {
	cerr << "[index grow, current=" << st->max_docid << ", needed=" << docid;

	// Empty record
	storage_record_t rec;
	rec.offset = (~0);
	rec.size = (~0);

	// Go to the end of the index
	off64_t indexsize = lseek( st->index_file, (off64_t)0, SEEK_END );
	if( indexsize < 0 ) {
		die( "lseek in index" );
	}

	// Check max_docid
	if( st->max_docid != (indexsize/sizeof(storage_record_t)) ) {
		cerr << "st->max_docid=" << st->max_docid << ", " << indexsize << ", " << indexsize/sizeof(storage_record_t) << endl;
		die( "st->max_docid inconsistent" );
	}

	// See how much it's necessary to grow
	docid_t new_max = st->max_docid;
	if( new_max == 0 )
	{
		new_max = 1;
	}
	while( new_max <= docid ) {
		new_max = new_max * STORAGE_GROW_FACTOR;
	}
	cerr << " to " << new_max;


	// Grow
	for(docid_t i = st->max_docid; i<new_max; i++ ) {
		if( write(st->index_file,&(rec),sizeof(storage_record_t)) < 0 ) {
			perror( "couldn't write to index file" );
			die( "write to index file" );
		}
	}

	// Update max_docid
	st->max_docid = new_max;

	cerr << " done] ";
}

//
// Name: _storage_hash
//
// Description:
//   Calculates a hash function for a document
//
// Input:
//   length - the document length
//   content - the document content
// 
// Return:
//   the hash funcion value, between 0 and
//   CONF_COLLECTION_MAXDOC * STORAGE_DUPLICATE_HASH_FACTOR
//

doc_hash_t _storage_hash( off64_t length, char *content ) {
	doc_hash_t hash_value;
	off64_t count = 0;
	for( hash_value = 0; count++ <= length; content++ ) {
		hash_value = 131 * hash_value + (*content);
	}
	return( hash_value 
			% (CONF_COLLECTION_MAXDOC * STORAGE_DUPLICATE_HASH_FACTOR) );
}


// StorageWriteSketch

storage_status_t storage_write_sketch( storage_t *st, docid_t docid, irudiko_sketch_t *sketch) {
        assert(st!=NULL);
	assert(docid > 0);
	assert(sketch!=NULL);
	assert(!st->readonly);
	/*	
	cout << "SKETCH: " << (*sketch).sketch[0];
	for(uint h = 1; h < IRUDIKO_SKETCHSIZE; ++h)
	  cout << ", " << (*sketch).sketch[h];
	cout << "." << endl;
	*/
	if(docid >= st->max_docid) { }

	if( lseek( st->sketch_file, (off64_t)(docid * sizeof(irudiko_sketch_t)), SEEK_SET) < (off64_t)0 ) {
		perror( "storage_write_sketch: couldn't seek on sketch" );
		die("seek on sketch");
	}
	if( write( st->sketch_file, sketch, sizeof(irudiko_sketch_t)) != sizeof(irudiko_sketch_t)) {
		perror( "storage_write_sketch: couldn't write to sketch" );
		die("write to sketch");
	}
	
	return STORAGE_OK;
}

//StorageReadSketch

storage_status_t storage_read_sketch( storage_t *st, docid_t docid, irudiko_sketch_t *sketch) {
        assert(st!=NULL);
	assert(docid > 0);
	assert(sketch!=NULL);
	
	if(docid >= st->max_docid) { return STORAGE_NOT_FOUND; }

	if( lseek( st->sketch_file, (off64_t)(docid * sizeof(irudiko_sketch_t)), SEEK_SET) < (off64_t)0 ) {
		perror( "storage_read_sketch: couldn't seek on sketch" );
		die("seek on sketch");
	}
	if( read( st->sketch_file, sketch, sizeof(irudiko_sketch_t)) != sizeof(irudiko_sketch_t)) {
		perror( "storage_read_sketch: couldn't read from sketch" );
		die("read from sketch");
	}
	
	return STORAGE_OK;
}
