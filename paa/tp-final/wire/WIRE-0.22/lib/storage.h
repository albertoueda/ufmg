
#ifndef _STORAGE_H_INCLUDED_
#define _STORAGE_H_INCLUDED_

#include <config.h>

// System libraries

#include <features.h>
#include <assert.h>
#include <stdio.h>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <queue>
#include <string>

using namespace std;

// Local libraries

#include "const.h"
#include "die.h"
#include "xmlconf.h"
#include "irudiko.h"

// Filenames

#define STORAGE_FILENAME_MAIN		"storage.main"
#define STORAGE_FILENAME_INDEX		"storage.index"
#define STORAGE_FILENAME_CONTENT	"storage.raw"
#define STORAGE_FILENAME_FREE		"storage.free"
#define STORAGE_FILENAME_DUPLICATES "storage_duplicates.docid_t"
#define STORAGE_FILENAME_SKETCH         "storage.sketch"
// Globals

// Records to create when eof reached
#define STORAGE_GROW_FACTOR			2

// Hash table size for duplicates, as a factor of CONF_COLLECTION_MAXDOC
#define STORAGE_DUPLICATE_HASH_FACTOR	10

// Status

enum storage_status_t {
	STORAGE_OK			= 0,
	STORAGE_NOT_FOUND	= 1,
	STORAGE_DUPLICATE	= 2,
	STORAGE_UNCHANGED
};

// Types

typedef struct {
	off64_t offset;
	off64_t size;
} storage_record_t;

typedef struct _storage_free_list_t {
	storage_record_t rec;
	_storage_free_list_t *next;
} storage_free_list_t;
	
typedef struct {
	char basedir[MAX_STR_LEN-1];
	bool readonly;
	docid_t max_docid;
	int content_file;
	int index_file;
	storage_free_list_t *free_list;
	int duplicates_file;
        int sketch_file;
} storage_t;

// Functions

storage_t *storage_open( const char *basedir, bool readonly );
storage_t *storage_create( const char *basedir, bool check_duplicates );
void storage_remove( const char *basedir );
storage_status_t storage_write( storage_t *st, docid_t docid, char *buf, off64_t content_length, doc_hash_t *hash, docid_t *duplicate_of );
storage_status_t storage_read( storage_t *st, docid_t docid, char *buf, off64_t *len );
storage_status_t storage_delete( storage_t *st, docid_t docid );
storage_status_t storage_exists( storage_t *st, docid_t docid );

void storage_close( storage_t *st );
void storage_dump_status( storage_t *st );

storage_status_t storage_prepare_sequential_read( storage_t *st );

// Irudiko-related functions
storage_status_t storage_write_sketch( storage_t *st, docid_t docid, irudiko_sketch_t *sketch);
storage_status_t storage_read_sketch( storage_t *st, docid_t docid, irudiko_sketch_t *sketch);

// "Private" functions

void _storage_grow( storage_t *st, docid_t docid );
void _storage_read_index_record( storage_t *st, docid_t docid, storage_record_t *rec );
void _storage_write_index_record( storage_t *st, docid_t docid, storage_record_t *rec );
void _storage_seek_index_record( storage_t *st, docid_t docid );
docid_t _storage_hash( off64_t length, char *content );

// INLINED FUNCTIONS (used by the linkidx also)

//
// Name: storage_sequential_read
//
// Description:
//   Prepares the storage file for sequential reading
//   THIS WORKS only if there are no deletions
//
// Input:
//   st - the storage object
//
// Output:
//   buf - the readed object
//   len - the length
//   

inline storage_status_t storage_sequential_read( storage_t *st, storage_record_t *rec, char *buf ) {

	// Read the storage record
    ssize_t readed = read( st->index_file, rec, sizeof( storage_record_t ) );
	if( readed == 0 ) {
		// End of file, this is ok, this means that we've reached
		// the end of the storage file, and the next records are empty
		buf[0] = '\0';
		return STORAGE_NOT_FOUND;
	}

	if( readed < 0 ) {
		perror( "storage_sequential_read: couldn't read from index" );
		die( "couldn't read from index" );
	}

	// Check for empty documents
	if( rec->size == -1 ) {
		buf[0] = '\0';
		return STORAGE_NOT_FOUND;
	}

	// Go to the content file address for that document
	if( lseek( st->content_file, rec->offset, SEEK_SET ) != rec->offset ) {
		perror( "storage_sequential_read: couldn't seek in content file" );
		die( "seek in content file" );
	}

	// Read the document
	if( read( st->content_file, buf, rec->size ) != rec->size ) {
		perror( "storage_sequential_read: failed to read content file" );
		die( "read content file" );
	}
	buf[rec->size] = '\0';

	return STORAGE_OK;
}

//
// Name: storage_sequential_skip
//
// Description:
//   Skips a number of records in a sequential read process
//   this is useful if we already know that those records
//   are empty, etc.
//
// Input:
//   st - the storage object
//   skip - the number of records to skip
//

inline void storage_sequential_skip( storage_t *st, uint skip ) {
	// Read the storage record
	off64_t rc = lseek( st->index_file, sizeof( storage_record_t ) * skip, SEEK_CUR );
	if( rc < 0 ) {
		perror( "storage_sequential_skip lseek" );
		die( "while skipping records" );
	}
}


#endif
