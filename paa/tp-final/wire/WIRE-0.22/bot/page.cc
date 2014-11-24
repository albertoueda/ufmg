
#include "page.h"

//
// Name: page_failed
//
// Description:
//   Sets the vars in page->doc, when the
//   http request failed
//    
// Input:
//   p - the page
//   http_code - the code
//

void page_failed( page_t *page, int http_code ) {
	assert( page != NULL );
	doc_t *doc = page->doc;
	assert( doc != NULL );

	// Now
	time_t now 			= time(NULL);

	doc->http_status	= http_code;
	doc->last_visit		= now;
	if( doc->number_visits == 0 ) {
		doc->first_visit = now;
	}
	doc->number_visits	++;
	doc->raw_content_length	= 0;
}

//
// Name: page_create_request
//
// Description:
//   Writes the request to server->socket
//
// Input:
//   server - the server
//   page - this page
//   useragent - identification of this crawler
//   max_request_size - maximum size of the request
//
// Output:
//   request - must be of size max_request_size
//   request_size - the size of the created request
//

void page_create_request( server_t *server, page_t *page, char *useragent, uint max_request_size, char *request, uint *request_size ) {
	assert( server != NULL );
	assert( server->socket >= 0 );
	assert( page != NULL );
	assert( page->path != NULL );
	assert( strlen(page->path) == 0 || page->path[0] != '/' );
	doc_t *doc = page->doc;
	assert( doc != NULL );
	assert( doc->docid > (docid_t)0 );
	
	// Create request
	request[0] = '\0';

	// Notice that the PATH could be exactly MAX_STR_LEN - 1,
	// So the request line must be rather large
	char req_line[MAX_STR_LEN * 2];
	sprintf( req_line, "GET /%s HTTP/1.1\r\n",	page->path );
		strcat( request, req_line );
	sprintf( req_line, "Host: %s\r\n",		server->hostname );
		strcat( request, req_line );
	sprintf( req_line, "Range: 0-%lu\r\n",		CONF_MAX_FILE_SIZE );
		strcat( request, req_line );
	sprintf( req_line, "User-Agent: %s\r\n",	useragent );
		strcat( request, req_line );
	sprintf( req_line, "Connection: close\r\n" );
		strcat( request, req_line );
	sprintf( req_line, "Accept: %s\r\n",		CONF_HARVESTER_ACCEPTMIME );
		strcat( request, req_line );

	// Add Last-Visited if necessary
	if( doc->last_visit > 0 ) {
		char last_visited[MAX_STR_LEN];
		strftime( last_visited, MAX_STR_LEN, HTTP_TIME_FORMAT, gmtime( &(doc->last_visit) ) );

		sprintf(  req_line, "If-Modified-Since: %s\r\n", last_visited );
			strcat( request, req_line );
	}

	// Blank line at the end of the request
	strcat( request, "\r\n" );
	(*request_size)	= strlen( request );
	assert( (*request_size) < max_request_size );
}
