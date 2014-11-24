
#include "string_list.h"

//
// Name: string_list_create
//
// Description:
//   Creates a string list
//
// Input:
//   words - a string containing a list of words
//
// Returns:
//   the created string list
//

string_list_t *string_list_create( char *words ) {
	assert( words != NULL );
	assert( strlen(words) > 0 );
	assert( strlen(words) < MAX_STR_LEN );

	// Allocate memory for the string list
	string_list_t *list	= (string_list_t *)malloc(sizeof(string_list_t));
	list->count			= 0;

	// Create a copy, as strtok destroys the string
	char words_cpy[MAX_STR_LEN];
	assert( strlen(words) < MAX_STR_LEN );
	strcpy( words_cpy, words );

	char *word	= strtok( words_cpy, CONF_LIST_SEPARATOR );
	while( word != NULL ) {
		// Test word
		list->len[list->count]	= strlen(word);
		assert( list->len[list->count] > 0 );

		// Get memory
		list->str[list->count]	= (char *)malloc(sizeof(char)*(strlen(word)+1));

		// Copy the word
		assert( strlen(word) < MAX_STR_LEN );
		strcpy( list->str[list->count], word );

		// Next word
		list->count		++;
		word	= strtok( NULL, CONF_LIST_SEPARATOR );
		assert( list->count < MAX_STRING_LIST_LENGTH );
	}
	assert( list->count > 0 );

	return list;
}

//
// Name: string_list_free
//
// Description:
//   Frees the memory used by a string list
//
// Input:
//   list - the list
//

void string_list_free( string_list_t *list ) {
	assert( list != NULL );

	for( uint i=0; i<list->count; i++ ) {
		free( list->str[i] );
	}
	free( list );
}

//
// Name: string_list_suffix
//
// Description:
//   Indicates if a given string has one of the
//   strings in the list as a suffix. This uses
//   brute force.
//
// Input:
//   list - the list
//   str - the string
//
// Returns:
//   true iff it has one of the strings as a suffix
//

bool string_list_suffix( string_list_t *list, char *str ) {
	assert( list != NULL );
	assert( str != NULL );
	assert( list->count > 0 );

	// If the string is empty, then we will consider it
	// does not match any of the suffixes
	if( strlen(str) == 0 ) {
		return false;
	}

	// Build a list of the strings that are still active,
	// in terms of still having possibilities of being a suffix
	uint len = (uint)(strlen(str));
	bool active[MAX_STRING_LIST_LENGTH];
	for( uint i=0; i<list->count; i++ ) {
		// If the string is long enough to be a suffix
		if( list->len[i] <= len ) {
			active[i]	= true;
		} else {
			active[i] 	= false;
		}
	}

	// Iterate
	for( uint pos=0; pos<len; pos++ ) {

		// Compare from the last character to the beginning
		char compared	= tolower(str[len-pos-1]);

		bool still_active = false;
		for( uint i=0; i<list->count; i++ ) {
			if( active[i] ) {
				// Check if the string still can be compared
				if( list->len[i] >= pos  ) {

					// Compare the string
					if( tolower((list->str[i])[(list->len[i])-pos-1])
							== compared ) {

						// If we are in the last position
						if( list->len[i] - pos - 1 == 0 ) {
							return true;
						} else {
							// We are still searching
							still_active	= true;
						}
					} else {
						// The still differs in this position
						active[i] = false;
					} 
				} else {
					// That string cannot be compared any more
					active[i] = false;
				}
			}
		}

		// No comparison was successfull
		if( ! still_active ) {
			return false;
		}
	}
	return true;
}
