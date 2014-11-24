
#include "perfect_hash.h"

//
// Name: perfhash_dump
//
// Description:
//   Dumps the state of a keyword check
//
// Input:
//   perfhash - the keyword checker object 
//

void perfhash_dump( perfhash_t *perfhash ) {
	fprintf( stderr, "BEGIN perfhash dump\n" );
	fprintf( stderr, "length        : %d\n", perfhash->length );
	fprintf( stderr, "count         : %d\n", perfhash->count );
	fprintf( stderr, "A             : %d\n", perfhash->A );
	fprintf( stderr, "B             : %d\n", perfhash->B );
	fprintf( stderr, "check_matches : %d\n", perfhash->check_matches );
	fprintf( stderr, "END perfhash dump\n" );
}

/*
 Name: perfhash_create

 Description:
   Creates a perfhash object

 Input:
   perfhash - space for a perfhash object
   perfhash->check_matches - 0:probabilistic (w/false positives) 1:exact
   keywords - a string containing a list of keywords separated by spaces
*/

void perfhash_create( perfhash_t *perfhash, char *in_keywords ) {
	assert( PERFHASH_MAX_WORD_LEN < MAX_STR_LEN );
	assert( (PERFHASH_MAX_WORD_LEN+1 * PERFHASH_MAX_WORDS) < MAX_DOC_LEN );

	char *copy_keywords = NULL;

	// Copy the keywords
	copy_keywords = (char *)malloc(sizeof(char)*(PERFHASH_MAX_WORD_LEN+1)*PERFHASH_MAX_WORDS);

	perfhash->count = 0;
	assert( strlen(in_keywords) < ((PERFHASH_MAX_WORD_LEN+1)*PERFHASH_MAX_WORDS) );
	strcpy( copy_keywords, in_keywords );

	/* We will tokenize only once */
	perfhash->count = 0;
	char *word;
	word = strtok( copy_keywords, CONF_LIST_SEPARATOR );
	while( word != NULL ) {

		int word_length = strlen(word);
		if( word_length >= PERFHASH_MAX_WORD_LEN ) {
			cerr << "This word is too long: '" << word << "'" << endl;
			assert( 0 );
		}

		// Store the keyword
		perfhash->keywords[perfhash->count] = (char *)malloc(sizeof(char) * PERFHASH_MAX_WORD_LEN );
		assert( perfhash->keywords[perfhash->count] != NULL );
		assert( strlen(word) < PERFHASH_MAX_WORD_LEN );
		strcpy( perfhash->keywords[perfhash->count], word );

		// Next item
		perfhash->count ++;
		assert( perfhash->count < PERFHASH_MAX_WORDS );

		word = strtok( NULL, CONF_LIST_SEPARATOR );
	}

	// Create a hash table with enough space
	perfhash->length = (uint)((double)perfhash->count / (1.0 - PERFHASH_SECURITY_FACTOR));
	perfhash->length += (uint)(((double)rand() / (double)RAND_MAX) * perfhash->length);
	assert( perfhash->length > 0 );

	// Ask memory for the table
	perfhash->table = (int *)malloc(sizeof(int) * (perfhash->length+1));
	assert( perfhash->table != NULL );

	// Try many times with random factors
	int retries = 1;
	char last_collision[MAX_STR_LEN] = "";
	while( retries <= PERFHASH_MAX_RETRIES ) {

		// Clear the table
		_perfhash_clear_table( perfhash );

		// Try with some parameters
		if( _perfhash_try_parameters( perfhash, last_collision ) ) {

			break;
		} else {

			// If not sucesful, keep trying
			retries++;
		}
	}

	// Check the number of tries
	if( retries >= PERFHASH_MAX_RETRIES ) {
		cerr << "Failed to create hash table for this after " << PERFHASH_MAX_RETRIES << " tries." << endl;
		for( uint i=0; i<perfhash->count; i++ ) {
			cerr << " - " << perfhash->keywords[i] << endl;
		}
		cerr << "Last collision was: '" << last_collision << "'" << endl;
		perfhash_dump( perfhash );
		assert(0);
	}

	/* We don't need to keep the keywords unless we check them */	
	if( !perfhash->check_matches ) {
		for( uint i=0; i<perfhash->count; i++ ) {
			free( perfhash->keywords[i] );
		}
	}

	// Validate that the table was created ok, and
	// that at least all its elements are ok.
	strcpy( copy_keywords, in_keywords );
	word = strtok( copy_keywords, CONF_LIST_SEPARATOR );
	while( word != NULL ) {
		if( ! perfhash_check( perfhash, word ) ) {
			cerr << endl;
			cerr << "Error! keyword not found: " << word << endl;
			cerr << "Original keywords were: " << in_keywords << endl;
			assert(0);

		}
		word = strtok( NULL, CONF_LIST_SEPARATOR );
	}
	free( copy_keywords );

}

/*
 Name: _perfhash_clear_table

 Description:
   Clears the hash table of a perfhash

 Input:
   perfhash.length - length of the table
*/

void _perfhash_clear_table( perfhash_t *perfhash ) {
	for( uint i=0; i<perfhash->length; i++ ) {
		(perfhash->table)[i] = -1;
	}
}

/*
 Name: _perfhash_hash

 Description:
   Calculates the hash function for a given string

 Input:
   perfhash - the perfhash object
   word - the word to be hashed

 Output:
   hash1 - value of the hash function
   hash2 - other hash
*/

void _perfhash_hash( perfhash_t *perfhash, char *word, int *hash, int *hash2 ) {
    (*hash)		= 0;
    (*hash2)	= 0;

	for( ; (*word) != '\0'; word++ ) {
		(*hash) = (perfhash->A) * (*hash + perfhash->B) + (int)(*word);
		(*hash2) = 131 * (*hash2) + (int)(*word);
	}

	(*hash) = (*hash) % perfhash->length;

	if( (*hash2) == -1 ) {
		// -1 marks an empty slot, so we don't use it to verify
		(*hash2) = -2;
	}
}

//
// Name: _perfhash_try_parameters
//
// Description:
//   Generates parameters and try to create a perfect hashing function
//   with them.
//
// Input:
//   perfhash->count - number of keywords
//   perfhash->keywords - keywords
//
// Output:
//   perfhash->A, perfhash->B - hash function parameters
//   perfhash->table - hash table
//   perfhash->length - length of the hash table (or zero, if failed to create)
//

bool _perfhash_try_parameters( perfhash_t *perfhash, char *last_collision ) {
	/* Randomized parameters, notice that they are not even primes */
	perfhash->A = (uint)rand();
	perfhash->B = (uint)rand();

	/* We will check for collisions */
	bool has_collisions = false;
	for( uint i=0; i<perfhash->count; i++ ) {
		int position;
		int vrfy;

		_perfhash_hash( perfhash, perfhash->keywords[i], &(position), &(vrfy) );

		if( perfhash->table[position] != -1 ) {
			has_collisions = true;
			assert( strlen(perfhash->keywords[i]) < PERFHASH_MAX_WORD_LEN );
			strcpy( last_collision, perfhash->keywords[i] );
			strcat( last_collision, "/" );
			strcat( last_collision, perfhash->keywords[perfhash->table[position]] );
			break;

		} else {
			if( perfhash->check_matches ) {
				// Store the index of the keyword
				perfhash->table[position] = i;
			} else {
				// Store a hash for verification
				perfhash->table[position] = vrfy;
			}
		}
	}

	if( has_collisions ) {
		// Parameter generation failed
		return false;
	} else {
		// Parameters ok
		return true;
	}
}

/*
 Name: perfhash_destroy

 Description:
   Destroy the table.

 Input:
   perfhash - the perfhash object

 Output:
   perfhash->table - is freed
   perfhash->length - is set to 0
   
*/

void perfhash_destroy( perfhash_t *perfhash ) {
	free( perfhash->table );
	perfhash->length = 0;

	/* It is necessary to clean keywords if they were stored */
	if( perfhash->check_matches ) {
		for( uint i=0; i<perfhash->count; i++ ) {
			free( perfhash->keywords[i] );
		}
	}
	perfhash->count  = 0;
}

/*
 Name: perfhash_check

 Description:
   Checks if a keyword is present; this is probabilistic and gives
   false-positives (unless check_matches=true), but is very fast.

 Input:
   perfhash - the perfhash object
   word - the word to check
*/

bool perfhash_check( perfhash_t *perfhash, char *word ) {
	assert( perfhash->length > 0 );
	int position;
	int vrfy;
	int table_content;

	_perfhash_hash( perfhash, word, &(position), &(vrfy) );


	table_content 	= perfhash->table[position];

	if( table_content == -1 ) {
		return false;
	}

	if( perfhash->check_matches ) {
		assert( table_content >= 0 );

		// We will check the word
		char *word_to_check = perfhash->keywords[table_content];


		// Compare, INCLUDING the '\0' at the end
		for( uint i=0; i<=strlen(word); i++ ) {
			if( word_to_check[i] != word[i] ) {
				return false;
			}
		}
		return true;


	} else {
		// This is fast, but generates false positives, with low prob. 
		if( table_content == vrfy ) {
			return true;
		} else {
			return false;
		}
	}
}
