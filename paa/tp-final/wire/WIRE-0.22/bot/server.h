
#ifndef _SERVER_H_INCLUDED_
#define _SERVER_H_INCLUDED_

#include <config.h>

// System libraries

#include <adns.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/poll.h>

// Local libraries

#include "xmlconf.h"
#include "metaidx.h"
#include "http_charset.h"

// Constants

#define REQUEST_MAXSIZE	4096
#define BUFFER_SIZE	32768
#define SERVER_MAX_NAMESERVERS	100

enum conn_status_t {
	CONN_NONE = 0,
	CONN_ACTIVATING,
	CONN_RESOLVING,
	CONN_CONNECTING,
	CONN_REQUESTING,
	CONN_RECEIVING,
	CONN_DEACTIVATING
};

enum poll_status_t {
	POLL_NOTHING = 0,
	POLL_READY,
	POLL_ERROR,
	POLL_DISCONNECT
};

// Structs

// Linked list of docs
typedef struct page_ptr {
	doc_t 		*doc;
	char		path[MAX_STR_LEN];
	char		relocation[MAX_STR_LEN];

	struct	timeval		timestamp_begin_connect;
	struct	timeval		timestamp_end_connect;
	struct	timeval		timestamp_first_read;
	struct	timeval		timestamp_last_read;
	off64_t		bytes_first_packet;
	off64_t		bytes_total;

	uint		speed_measures;

	page_ptr	*next;
} page_t;

// Server (site + pages + hostname + networkstate)
typedef struct {
	site_t	*site;
	char	*hostname;	// The ip address is in the 'site' structure

	int pool_slot;		// In the 'activepool_poll' list (can be -1)
	uint serverid;		// In the 'servers' list

	int	socket;			// -1 = not connected

	uint	ndocs_done;	// docs fetched
	uint	ndocs;		// docs in total
	page_t	*pages;		// pages (docs+)

	uint http_status;

	uint wait;			// time to wait
	uint errors_in_this_harvest; // errors of this server in this harvest

	conn_status_t	conn_status;

	struct	timeval		timestamp_resolver_start;
	struct	timeval		timestamp_resolver_end;
} server_t;

#include "page.h"

extern off64_t	bytes_in;
extern off64_t	bytes_out;

// Functions

// Query
bool server_has_ip( server_t *server );
bool server_has_more_documents( server_t *server );

// Init
void server_copy_site( server_t *server, site_t *site, const char *hostname, uint serverid );

// Resolve
void server_resolver_prepare();
void server_resolver_start( server_t *server, adns_state *adns_ptr, adns_query *adns_query_ptr, short *events, uint *timeout_seconds );
void server_resolver_afterpoll( server_t *server, adns_state *adns_ptr, adns_query *adns_query_ptr, int revents );
void server_resolver_done( server_t *server, adns_state *adns_ptr, adns_answer *answer );

// Connect
void server_connect_start( server_t *server, short *events, uint *timeout_seconds, char *useragent );

// Request
void server_request_start( server_t *server, short *events, uint *timeout_seconds, char *useragent );
void server_request_afterpoll( server_t *server, short revents );

// Receive
void server_receive( server_t *server, short *events, uint *timeout_seconds, FILE *content_file );

// Parse headers
ssize_t server_parse_headers( server_t *server, char *buffer, ssize_t bufsize );
uint server_parse_http_code( server_t *server, char *header, uint len );
void server_parse_http_header(server_t *server, char *header, uint len );

// Save documents
void server_skip_all( server_t *server, FILE *doc_file, uint http_code );
void server_save_site( server_t *server, FILE *site_file );
void server_save_current_page( server_t *server, FILE *doc_file );

// Close
void server_close( server_t *server );

#endif
