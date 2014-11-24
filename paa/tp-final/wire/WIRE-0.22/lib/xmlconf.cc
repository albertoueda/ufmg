
#include "xmlconf.h"

// 
// Name: xmlconf_read_string
//
// Description:
//   Reads a single string from the configuration file.
// 
// Input:
//   xpath - a xpath route, relative to the base path for the configuration
//
// Output:
//   dest - the string found at that node
//

void xmlconf_read_string( const char *xpath, char *dest ) {
	xmlXPathObjectPtr path; 
	char xpath_expression[MAX_STR_LEN];

	// Transform the expression
	sprintf( xpath_expression, "/%s/%s/text()", XMLCONF_BASE_XPATH, xpath );

	// Evaluate the xpath expression
	path = xmlXPathEvalExpression((const xmlChar *)xpath_expression
			, xmlconf_context); 

	// Check the result
	if( path->nodesetval == NULL
			|| path->nodesetval->nodeNr != 1 
			|| path->nodesetval->nodeTab[0]->type != XML_TEXT_NODE 
			|| path->nodesetval->nodeTab[0]->content == NULL ) {
		cerr << "Configuration variable '" << xpath_expression
			<< "' not found in config file" << endl;
		cerr << endl;
		cerr << "Please check that this variable exists in the XML configuration file." << endl;
		cerr << "If it doesn't, simply copy the default value from the sample" << endl;
		cerr << "configuration file (sample.conf) included in the distribution." << endl;
		cerr << "Please note that new variables are added in new versions." << endl;
		die( "reading config file" );
	}

	// Get from the document
	char *found = NULL;
	found = (char *)(path->nodesetval->nodeTab[0]->content);

	// Check string length
	assert( strlen(found) + 1 < MAX_STR_LEN );

	// copy
	strcpy( dest, found );

	// Free
	xmlXPathFreeObject(path); 
}

// 
// Name: xmlconf_try_read_string
//
// Description:
//   Reads a single string from the configuration file, if it's not
//   found, do not die
// 
// Input:
//   xpath - a xpath route, relative to the base path for the configuration
//
// Output:
//   dest - the string found at that node
//
// Return:
//   0 if error
//   1 if successfull

int xmlconf_try_read_string( const char *xpath, char *dest ) {
	xmlXPathObjectPtr path; 
	char xpath_expression[MAX_STR_LEN];

	// Transform the expression
	sprintf( xpath_expression, "/%s/%s/text()", XMLCONF_BASE_XPATH, xpath );

	// Evaluate the xpath expression
	path = xmlXPathEvalExpression((const xmlChar *)xpath_expression
			, xmlconf_context); 

	// Check the result
	if( path->nodesetval == NULL
			|| path->nodesetval->nodeNr != 1 
			|| path->nodesetval->nodeTab[0]->type != XML_TEXT_NODE 
			|| path->nodesetval->nodeTab[0]->content == NULL ) {
			return 0;
	}

	// Get from the document
	char *found = NULL;
	found = (char *)(path->nodesetval->nodeTab[0]->content);

	// Check string length
	assert( strlen(found) + 1 < MAX_STR_LEN );

	// copy
	strcpy( dest, found );

	// Free
	xmlXPathFreeObject(path); 

	return 1;
}

// 
// Name: xmlconf_read_multi_string
//
// Description:
//   Reads multiple string from the configuration file, concatenate
//   them and return as a list of space-separated strings. It is
//   different from xmlconf_read_string because the other function
//   fails if more than one node is found at the corresponding path.
// 
// Input:
//   xpath - a xpath route, relative to the base path for the configuration
//
// Output:
//   dest - the strings found at that node
//

void xmlconf_read_multi_string( const char *xpath, char *dest ) {
	xmlXPathObjectPtr path; 
	char xpath_expression[MAX_STR_LEN];

	// Transform the expression
	sprintf( xpath_expression, "/%s/%s/text()", XMLCONF_BASE_XPATH, xpath );

	// Evaluate the xpath expression
	path = xmlXPathEvalExpression((const xmlChar *)xpath_expression
			, xmlconf_context); 

	// Check the result
	if( path->nodesetval == NULL || path->nodesetval->nodeNr <= 0 ) {
		cerr << "Failed to read '" << xpath_expression
			<< "' in config file" << endl;
		die( "reading config file" );
	}

	// Get from the document
	dest[0] = '\0';
	for( int i=0; i<path->nodesetval->nodeNr; i++ ) {
		char *found	= (char *)(path->nodesetval->nodeTab[i]->content);

		// Check string length
		assert( strlen(dest) + strlen(found) + 1 < MAX_STR_LEN );

		// Add to the string
		strcat( dest, found );
		strcat( dest, " " );
	}
	assert( strlen(dest) > 0 );
	assert( strlen(dest) < MAX_STR_LEN );

	// Remove trailing space
	dest[strlen(dest)-1]	= '\0';

	// Free
	xmlXPathFreeObject(path); 
}

//
// Name: xmlconf_suffix_multiplier
//
// Description:
//   Returns the multiplier associated to a given suffix,
//   e.g.: returns 1,000 for "K", 1,000,000 for "M", etc.
//
// Input:
//   str - the string
//
// Returns:
//   1 if the string doesn't hold a valid suffix
//   the multiplier for that suffix otherwise
//

uint xmlconf_suffix_multiplier( char *suffix ) {
	if( suffix == NULL ) {
		return 1;					// No number
	} else if( strlen(suffix) == 0 ) {
		return 1;					// No suffix
	} else if( !strcmp( suffix, "m" ) ) {
		return 60;                  // Minutes
	} else if( !strcmp( suffix, "h" ) ) {
		return 60 * 60;             // Hours
	} else if( !strcmp( suffix, "d" ) ) {
		return 60 * 60 * 24;        // Days
	} else if( !strcmp( suffix, "w" ) ) {
		return 60 * 60 * 24 * 7;    // Weeks
	} else if( !strcmp( suffix, "K" ) ) {
		return 1000;                // Kilo
	} else if( !strcmp( suffix, "M" ) ) {
		return 1000000;             // Mega
	} else {
		fprintf( stderr, "Warning! Unknown suffix: '%s' (ignoring)\n", suffix );
		return 1;
	}
}

//
// Name: xmlconf_read_uint
//
// Description:
//   Reads an unsigned integer from the configuration file.
//
// Input:
//   xpath - an xpath expression, relative to the base of the configuration
//
// Output:
//   dest - the number read
//

void xmlconf_read_uint( const char *xpath, uint *dest ) {
	char tmp[MAX_STR_LEN];
	char *suffix;

	xmlconf_read_string( xpath, tmp );

	*dest = (uint)strtol(tmp, &(suffix), 10);	

	(*dest)	*= (uint)xmlconf_suffix_multiplier( suffix );
}

//
// Name: xmlconf_read_uint_default
//
// Description:
//   Reads an unsigned integer from the configuration file,
//   can use a default value
//
// Input:
//   xpath - an xpath expression, relative to the base of the configuration
//   default - the default value
//
// Output:
//   dest - the number read
//

void xmlconf_read_uint_default( const char *xpath, uint *dest, uint default_value ) {
	char tmp[MAX_STR_LEN];
	char *suffix;

	if( xmlconf_try_read_string( xpath, tmp ) ) {
		*dest = (uint)strtol(tmp, &(suffix), 10);	
		(*dest)	*= (uint)xmlconf_suffix_multiplier( suffix );
	} else {
		(*dest) = default_value;
		cerr << "Using default value: " << default_value << " for " << xpath << endl;
	}

}

//
// Name: xmlconf_read_ulong
//
// Description:
//   Reads an unsigned long integer from the configuration file.
//
// Input:
//   xpath - an xpath expression, relative to the base of the configuration
//
// Output:
//   dest - the number read
//

void xmlconf_read_ulong( const char *xpath, unsigned long *dest ) {
	char tmp[MAX_STR_LEN];
	char *suffix;
	xmlconf_read_string( xpath, tmp );

	*dest = strtoul(tmp, &(suffix), 10);	

	(*dest)	*=	(ulong)xmlconf_suffix_multiplier( suffix );
}

//
// Name: xmlconf_read_double
//
// Description:
//   Reads a double precision floating number from the configuration file.
//
// Input:
//   xpath - an xpath expression, relative to the base of the configuration
//
// Output:
//   dest - the number read
//

void xmlconf_read_double( const char *xpath, double *dest ) {
	char tmp[MAX_STR_LEN];
	xmlconf_read_string( xpath, tmp );
	*dest = (double)atof(tmp);
}

//
// Name: xmlconf_load
//
// Description:
//   Loads the contents of the configuration file, as
//   specified by xmlconf-vars.h
//

void xmlconf_load() {

	// Search the configuration file
	char search_path[MAX_STR_LEN];
	int found	= 0;

	// Check if WIRE_CONF is defined
	char *env_wire_conf = getenv( XMLCONF_ENVIRONMENT_VAR );
	if( env_wire_conf != NULL ) {
		strcpy( search_path, env_wire_conf );
	} else {
		strcpy( search_path, XMLCONF_FILE_PATH );
	}

	// Scan the path
	char *filename = strtok( search_path, ":" );
	while( filename != NULL ) {
		struct stat64 stat_buf;
		if( stat64(filename, &stat_buf) == 0 ) {
			found = 1;
			cerr << "[using " << filename << "] ";
			break;
		} else {
			filename = strtok( NULL, ":" );
		}
	}

	// Check if found
	if( !found ) {
		cerr << "Couldn't find configuration file" << endl;
		cerr << "Tried the default search path " << XMLCONF_FILE_PATH << endl;
		if( !env_wire_conf) {
			cerr << "You MUST set the environment variable " << XMLCONF_ENVIRONMENT_VAR << endl;
		} else {
			cerr << "The variable " << XMLCONF_ENVIRONMENT_VAR << " points to " << env_wire_conf << endl;

		}
		die( "Couldn't found configuration file: check environment." );
	}

	// Init the xml parser and the xpath engine
	xmlKeepBlanksDefault(1); 
	xmlXPathInit();

	// Parse the file
	xmlconf_document = xmlParseFile( filename );
	if( xmlconf_document == NULL ) {
		die( "Unable to parse configuration file" );
	}
	assert(xmlconf_document != NULL);

	// Get the context
	xmlconf_context = xmlXPathNewContext(xmlconf_document);
	assert( xmlconf_context != NULL );

	// Read the variables; this is on a separate file
	// because it is easier to maintain that way
 	_CONF_EVALUATE

	// Cleanup
	xmlFreeDoc( xmlconf_document );
	xmlXPathFreeContext(xmlconf_context); 

	CONF_OK = true;
}
