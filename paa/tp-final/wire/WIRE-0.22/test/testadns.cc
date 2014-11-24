
#include <adns.h>
#include <assert.h>
#include <errno.h>
#include <sys/poll.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "die.h"
#include "cleanup.h"

char CONF_HARVESTER_RESOLVCONF[MAX_STR_LEN];
int http_resolve_with_adns( char *hostname, uint timeout, struct in_addr *addr );

int main( int argc, char **argv ) {
	struct in_addr addr;

	if( argc != 3 ) {
		die( "Usage: program nameserver_ip_address host_name_to_resolv\n" );
	}
	sprintf( CONF_HARVESTER_RESOLVCONF, "nameserver %s", argv[1] );

	printf( "Using nameserver %s\n", argv[1] );
	printf( "Querying for %s\n", argv[2] );

	http_resolve_with_adns( argv[2], 10000, &(addr) );

	printf( "Resolved: %s\n", inet_ntoa(addr) );

}

int http_resolve_with_adns( char *hostname, uint timeout, struct in_addr *addr ) {
	adns_state adns;
	adns_answer *answer;
	adns_query query;
	int result = 0;
	assert( timeout > 0 );

	errno = adns_init_strcfg( &(adns), adns_if_noenv, NULL, CONF_HARVESTER_RESOLVCONF );
	if( errno != 0 ) {
		perror( "adns_init_strcfg" );
		fprintf( stderr, "Init string was '%s'\n", CONF_HARVESTER_RESOLVCONF );
		fprintf( stderr, "Error in adns_init_strcfg: possible cause is number of descriptors available" );
		exit(1);
	}

	// Submit the query to adns
	int rc = adns_submit(adns,
		hostname,
		 adns_r_a,
	     (adns_queryflags)(adns_qf_quoteok_cname|adns_qf_cname_loose),
		 NULL,
		 &query);
	assert( rc == 0 );

    // Check the query. It probably it's not ready, so this will
	// return EWOULDBLOCK
	rc = adns_check( adns, &(query), &(answer), NULL );
	bool timedout = false;

	if( rc == EWOULDBLOCK ) {
		struct pollfd *fds;
		int nfds;
		int adns_timeout;
		int err;

		// Copy 
		nfds = ADNS_POLLFDS_RECOMMENDED;
		fds = (struct pollfd *)malloc(sizeof(struct pollfd)*nfds);
		assert( fds != NULL );

		// Copy the timeout
		adns_timeout = timeout;

		// Check the number of fds required
		err = adns_beforepoll(adns, fds, &nfds, &adns_timeout, 0);
		fprintf( stderr, "Requires %d sockets\n", nfds );

		// Check if adns will require more fds
		if( err == ERANGE ) {
			fprintf( stderr, "*** Number of FDs changed: nfds=%d, recom=%d\n", nfds, ADNS_POLLFDS_RECOMMENDED );
			free( fds );
			fds = (struct pollfd *)malloc(sizeof(struct pollfd)*nfds);
			assert( fds != NULL);
		} else {
			assert( err == 0 );
		}

		// Check if the timeout was changed
		if( adns_timeout != (int)timeout ) {
			fprintf( stderr, "*** Timeout changed: from %d to %d\n", timeout, adns_timeout );
		}

		// Poll
		rc = poll(fds, nfds, adns_timeout);

		if( rc < 0 ) {
			// Error
			perror( "poll" );

			// Errors of poll can happend during debugging,
			// so we ignore the "Interrupted system call", just
			// cancel the request
			if( errno != EINTR ) {
				fprintf( stderr, "poll" );
				exit(1);
			} else {
				adns_cancel(query);
				memset( addr, 0, sizeof(struct in_addr) );
				timedout = true;
				result = ETIMEDOUT;
			}

		} else if ( rc == 0 ) {
			// Timeout
			adns_cancel(query);
			memset( addr, 0, sizeof(struct in_addr) );
			timedout = true;
			result = ETIMEDOUT;

		} else {
			// Ready
			adns_afterpoll(adns, fds, rc ? nfds : 0, 0);
			rc = adns_wait( adns, &(query), &(answer), NULL );
			assert( rc == 0 );
		}
		free( fds );
	}

	if( !timedout ) {
		if( answer->status == adns_s_ok ) {
			memcpy( addr, answer->rrs.inaddr, sizeof(struct in_addr) );
			free(answer);
			result = 0;
		} else {
			memset( addr, 0, sizeof(struct in_addr) );
			result = ENOENT;
		}
	}
	adns_finish( adns );

	return result;
}

void cleanup() { }
