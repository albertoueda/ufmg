
#include "utils.h"

//
// Name: tokenizeToMap
//
// Description:
//   Split a string in tokens, save all tokens as keys in a map
//   The values will be boolean true
//
// Input:
//   str - String to tokenize
//
// Output:
//   theMap - Output map
//

void tokenizeToMap(char *str, map<string,bool> &theMap) {
    if( str==NULL ) {
        return;
    }

    uint element_count = 0;

    char *s = strtok(str,CONF_LIST_SEPARATOR);
    while(s != NULL) {
        theMap[s] = true;
	element_count++;
        cerr << s << ", ";
        s = strtok(NULL,CONF_LIST_SEPARATOR);
    }

    return;
}

// 
// Name: tokenizeToTable
//
// Description:
//   Split a string into tokens, save all tokens in a table
//
// Input:
//   str - String to tokenize
//
// Output:
//   nvars - the number of vars
//
// Return:
//   a pointer to the table
//

char **tokenizeToTable( char *str, int *ntokens_out ) {
	// Copy the string, to avoid changing it
	char *token_cpy;
	char str_cpy[MAX_STR_LEN];
	strcpy( str_cpy, str );

	// Table with tokens
	char **table	= (char **)malloc(sizeof(char *) * MAX_TOKENS);

	// Iterate through the copy

	int ntokens   = 0;
	char *token = strtok( str_cpy, CONF_LIST_SEPARATOR );
	while( token != NULL ) {
		token_cpy   = (char *)malloc(sizeof(char)*MAX_STR_LEN);
		assert( token_cpy != NULL );
		strncpy( token_cpy, token, MAX_STR_LEN );
		table[ntokens++]	= token_cpy;
		token			= strtok( NULL, CONF_LIST_SEPARATOR );
		assert( ntokens < MAX_TOKENS );
	}

	// Copy the number of variables
	(*ntokens_out)	= ntokens;

	// Return the created table
	return table;
}

//
// Name: tokenizeToRegex
//
// Description:
//   Split a string in tokens, save all tokens as strings
//   to match in a regex
//
// Input:
//   str - String to tokenize
//
// Output:
//   regex - The regular expression
//

void tokenizeToRegex(char *str, regex_t *regex ) {
    if( str==NULL ) {
        return;
    }

	int rc;
	char reg_expression[MAX_STR_LEN];
	reg_expression[0] = '\0';
	uint str_count = 0;

    char *s = strtok(str,CONF_LIST_SEPARATOR);
    while(s != NULL) {
		if( str_count > 0 ) {
			strcat( reg_expression, "|" );
		}
		strcat( reg_expression, s );
        s = strtok(NULL,CONF_LIST_SEPARATOR);
		str_count ++;
    }

	rc = regcomp( regex, reg_expression, REG_EXTENDED|REG_NOSUB );
	if( rc ) {
			char error[MAX_STR_LEN];
			sprintf( error, "Can't compile this regexp: '%s'", reg_expression );
			die( error );
	}

    return;
}
// 
// Name: unescape_url
//
// Description:
//   Reduce any %xx escape sequences to the characters they represent
//   Fixed from NCSA HTTPD's example programs
//
// Input:
//   url - with escapes (e.g.: 'www/%7ejsmith')
//
// Output
//   url - without escapes (e.g.: 'www/~jsmith' )
//

/** Convert a two-char hex string into the char it represents. **/
char unescape_url_hex_to_char(char *what) {
   register char digit;

   digit = (what[0] >= 'A' ? ((what[0] & 0xdf) - 'A')+10 : (what[0] - '0'));
   digit *= 16;
   digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A')+10 : (what[1] - '0'));
   return(digit);
}

void unescape_url(char *url) {
    register int i,j;

    for(i=0,j=0; url[j]; ++i,++j) {
        if((url[i] = url[j]) == '%') {
			if( url[j+1] != '\0' && url[j+2] != '\0' ) {
				url[i] = unescape_url_hex_to_char(&url[j+1]) ;
				j+= 2 ;
			}
        }
    }
    url[i] = '\0' ;
}

//
// Name: has_nonprintable_character
//
// Description:
//   Check for non printable characters, using the isprint() macro
//
// Input:
//   str - a string to check
//
// Return:
//   true iff str has at least 1 non-printable character
//

bool has_nonprintable_characters( char *str ) {
	assert( str != NULL );
	size_t len	= strlen(str);
	assert( len <= MAX_STR_LEN );

	for( size_t i=0; i<len; i++ ) {
		if( ! isprint(str[i]) ) {
			return true;
		}
	}

	return false;
}

//
// Name: timestamp
//
// Description:
//   Generates a timestamp string, useful for naming
//   directories. Its resolution is up to the hour
//
// Output:
//   str - the string containing the timestamp
//

void timestamp( char *str ) {
	assert( str != NULL );
	time_t timevalue    = time(NULL);
	struct tm *timestruct   = localtime(&(timevalue));

	strftime( str, MAX_STR_LEN, "%Y%m%d_%Hh", timestruct );
}

//
// Name: createdir
//
// Description:
//   Tries to create a directory, die if fails, skip if exists
//
// Input:
//   dirname - the directory name
//

void createdir( const char *dirname ) {
	int sysrc = mkdir( dirname, 0777 );
	if( sysrc != 0 && errno != EEXIST ) {
		perror( dirname );
		die( "Failed to create directory" );
	}
}

//
// Name: replace_all
//
// Description:
//   Replaces occurences of a string in another
//
// Input:
//   str - the original string
//   pat - the string to search
//   rep - the new string
//

void replace_all( char *str, const char *pat, const char *rep ) {
	assert( str != NULL );
	assert( pat != NULL );
	assert( rep != NULL );

	assert( strlen(pat) > 0 );

	char *start;
	char copy[MAX_STR_LEN];

	while( (start = strstr( str, pat )) ) {

		// Copy the first part
		if( start != str ) {
			strncpy( copy, str, start - str );
		}

		copy[start-str]		= '\0';

		// Copy the replacement
		strcat( copy, rep );

		// Copy the rest
		strcat( copy, start + strlen(pat) );

		// Copy back to the string
		strcpy( str, copy );
	}
}

//
// Name: str_tolower
//
// Description:
//   Converts a string to lower case
//

void str_tolower( char *str ) {
	assert( str != NULL );

	uint i = 0;
	while( str[i] != '\0' ) {
		str[i]	= tolower(str[i]);
		i++;
	}
}

//
// Name: load_file
//
// Description:
//   Loads a file into a char array, up
//   to MAX_DOC_LEN bytes
//
// Input:
//   filename - the file name
//
// Output:
//   buffer - the buffer, filled with the contents of the file
//
//

size_t read_file( char *filename, char *buffer ) {
	assert( filename != NULL );
	assert( strlen(filename) > 0 );
	assert( buffer != NULL );

	// Ensure a consistent buffer
	buffer[0]	= '\0';

	// Open file
	FILE *file = fopen64( filename, "r" );
	if( file == NULL ) {
		perror( filename );
		die( "Can't load file" );
	}
	assert( file != NULL );
	size_t bytes = 0;

	// Read the contents of the file, they can be more
	// than MAX_DOC_LEN bytes
	bytes = fread( buffer, 1, (MAX_DOC_LEN - 1), file );
	assert( bytes < MAX_DOC_LEN );
	fclose( file );

	// Close string
	buffer[bytes]	= '\0';

	return bytes;
}




//
// Name: wire_start
//  
// Description:
//   Starts the system, prints a message about this program
//   Reads the configuration file.
//   Changes to the base dir
//
// Input:
//   program_name - this program name
//   

void wire_start ( const char *program_name ) {
	// Print program banner
	cerr << PACKAGE << " " << VERSION << " " << program_name << endl;

	// Read configuration and enable cleanup handler
	cerr << "Load configuration ... ";
	xmlconf_load();
	
	// Change to the base directory
	cerr << "chdir ... ";
	int rc = chdir( CONF_COLLECTION_BASE );
	if( rc ) {
		perror( CONF_COLLECTION_BASE );
		die( "Couldn't change to the base dir of the collection, please create this directory or check the configuration file" );
	}

	// Notice that if you want a core file for examining it
	// you don't want to cleanup_enable()
	cleanup_enable();
	cerr << "done." << endl;

	// Write to syslog
	openlog( PACKAGE, LOG_PERROR|LOG_PID, LOG_USER );
	syslog( LOG_INFO, "%s start", program_name );
}

//
// Name: wire_stop
//
// Description:
//   Closes syslog, ends the program
//
// Input:
//   code - exit code

void wire_stop( int code ) {
	if( code == 0 ) {
		cerr << "Normal finish ... ";
	} else {
		cerr << "Exit, code=" << code << " ... ";
	}

	// Clean up
	cleanup_disable();
	cleanup();
	cerr << "done." << endl;

	// Write to syslog
	if( code == 0 ) {
		syslog( LOG_INFO, "normal stop" );
	} else {
		syslog( LOG_WARNING, "abnormal stop, exit(%d)", code );
	}
	closelog();

	// Exit
	exit(code);
}

//
// Name: timer_delta
// 
// Description:
//   Gets the difference in seconds between two timevals
//
// Input:
//   tv1, tv2 - two times
//
// Returns:
//   tv2 - tv1
//

float timer_delta( struct timeval *tv1, struct timeval *tv2 ) {
	return( ( (float)(tv1->tv_sec) - (float)(tv2->tv_sec) )
		  + ( ( (float)(tv1->tv_usec) - (float)(tv2->tv_usec) ) / (float)1000000.0 ) );
}
